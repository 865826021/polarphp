//===--- CaptureInfo.h - Data Structure for Capture Lists -------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef POLARPHP_AST_CAPTURE_INFO_H
#define POLARPHP_AST_CAPTURE_INFO_H

#include "polarphp/basic/Debug.h"
#include "polarphp/basic/LLVM.h"
#include "polarphp/basic/OptionSet.h"
#include "polarphp/basic/SourceLoc.h"
#include "polarphp/ast/TypeAlignments.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/Support/TrailingObjects.h"
#include <vector>

namespace polar::ast {
class CapturedValue;
}

namespace llvm {
class raw_ostream;
template <> struct DenseMapInfo<polar::ast::CapturedValue>;
}

namespace polar::ast {
class ValueDecl;
class FuncDecl;
class OpaqueValueExpr;

using polar::basic::SourceLoc;
using polar::basic::OptionSet;

/// CapturedValue includes both the declaration being captured, along with flags
/// that indicate how it is captured.
class CapturedValue {
public:
   using Storage =
   llvm::PointerIntPair<llvm::PointerUnion<ValueDecl*, OpaqueValueExpr*>, 2,
   unsigned>;

private:
   Storage Value;
   SourceLoc Loc;

   explicit CapturedValue(Storage V, SourceLoc Loc) : Value(V), Loc(Loc) {}

public:
   friend struct llvm::DenseMapInfo<CapturedValue>;

   enum {
      /// IsDirect is set when a VarDecl with storage *and* accessors is captured
      /// by its storage address.  This happens in the accessors for the VarDecl.
      IsDirect = 1 << 0,

      /// IsNoEscape is set when a vardecl is captured by a noescape closure, and
      /// thus has its lifetime guaranteed.  It can be closed over by a fixed
      /// address if it has storage.
      IsNoEscape = 1 << 1
   };

   CapturedValue(ValueDecl *Val, unsigned Flags, SourceLoc Loc)
      : Value(Val, Flags), Loc(Loc) {}

   CapturedValue(OpaqueValueExpr *Val, unsigned Flags)
      : Value(Val, Flags), Loc(SourceLoc()) {}

   static CapturedValue getDynamicSelfMetadata()
   {
      return CapturedValue((ValueDecl *)nullptr, 0, SourceLoc());
   }

   bool isDirect() const { return Value.getInt() & IsDirect; }
   bool isNoEscape() const { return Value.getInt() & IsNoEscape; }

   bool isDynamicSelfMetadata() const { return !Value.getPointer(); }
   bool isOpaqueValue() const {
      return Value.getPointer().is<OpaqueValueExpr *>();
   }

   CapturedValue mergeFlags(CapturedValue cv) {
      assert(Value.getPointer() == cv.Value.getPointer() &&
             "merging flags on two different value decls");
      return CapturedValue(
               Storage(Value.getPointer(), getFlags() & cv.getFlags()),
               Loc);
   }

   ValueDecl *getDecl() const {
      assert(Value.getPointer() && "dynamic Self metadata capture does not "
                                   "have a value");
      return Value.getPointer().dyn_cast<ValueDecl *>();
   }

   OpaqueValueExpr *getOpaqueValue() const {
      assert(Value.getPointer() && "dynamic Self metadata capture does not "
                                   "have a value");
      return Value.getPointer().dyn_cast<OpaqueValueExpr *>();
   }

   SourceLoc getLoc() const { return Loc; }

   unsigned getFlags() const { return Value.getInt(); }
};

class DynamicSelfType;

/// Stores information about captured variables.
class CaptureInfo {
   class CaptureInfoStorage final
         : public llvm::TrailingObjects<CaptureInfoStorage, CapturedValue> {

      DynamicSelfType *DynamicSelf;
      OpaqueValueExpr *OpaqueValue;
      unsigned Count;
   public:
      explicit CaptureInfoStorage(unsigned count, DynamicSelfType *dynamicSelf,
                                  OpaqueValueExpr *opaqueValue)
         : DynamicSelf(dynamicSelf), OpaqueValue(opaqueValue), Count(count) { }

      ArrayRef<CapturedValue> getCaptures() const {
         return llvm::makeArrayRef(this->getTrailingObjects<CapturedValue>(),
                                   Count);
      }

      DynamicSelfType *getDynamicSelfType() const {
         return DynamicSelf;
      }

      OpaqueValueExpr *getOpaqueValue() const {
         return OpaqueValue;
      }
   };

   enum class Flags : unsigned {
      HasGenericParamCaptures = 1 << 0
   };

   llvm::PointerIntPair<const CaptureInfoStorage *, 2, OptionSet<Flags>>
   StorageAndFlags;

public:
   /// The default-constructed CaptureInfo is "not yet computed".
   CaptureInfo() = default;
   CaptureInfo(AstContext &ctx, ArrayRef<CapturedValue> captures,
               DynamicSelfType *dynamicSelf, OpaqueValueExpr *opaqueValue,
               bool genericParamCaptures);

   /// A CaptureInfo representing no captures at all.
   static CaptureInfo empty();

   bool hasBeenComputed() const {
      return StorageAndFlags.getPointer();
   }

   bool isTrivial() const {
      return getCaptures().empty() && !hasGenericParamCaptures() &&
            !hasDynamicSelfCapture() && !hasOpaqueValueCapture();
   }

   ArrayRef<CapturedValue> getCaptures() const {
      // FIXME: Ideally, everywhere that synthesizes a function should include
      // its capture info.
      if (!hasBeenComputed())
         return None;
      return StorageAndFlags.getPointer()->getCaptures();
   }

   /// Return a filtered list of the captures for this function,
   /// filtering out global variables.  This function returns the list that
   /// actually needs to be closed over.
   ///
   void getLocalCaptures(SmallVectorImpl<CapturedValue> &Result) const;

   /// \returns true if getLocalCaptures() will return a non-empty list.
   bool hasLocalCaptures() const;

   /// \returns true if the function captures any generic type parameters.
   bool hasGenericParamCaptures() const {
      // FIXME: Ideally, everywhere that synthesizes a function should include
      // its capture info.
      if (!hasBeenComputed())
         return false;
      return StorageAndFlags.getInt().contains(Flags::HasGenericParamCaptures);
   }

   /// \returns true if the function captures the dynamic Self type.
   bool hasDynamicSelfCapture() const {
      return getDynamicSelfType() != nullptr;
   }

   /// \returns the captured dynamic Self type, if any.
   DynamicSelfType *getDynamicSelfType() const {
      // FIXME: Ideally, everywhere that synthesizes a function should include
      // its capture info.
      if (!hasBeenComputed())
         return nullptr;
      return StorageAndFlags.getPointer()->getDynamicSelfType();
   }

   bool hasOpaqueValueCapture() const {
      return getOpaqueValue() != nullptr;
   }

   OpaqueValueExpr *getOpaqueValue() const {
      // FIXME: Ideally, everywhere that synthesizes a function should include
      // its capture info.
      if (!hasBeenComputed())
         return nullptr;
      return StorageAndFlags.getPointer()->getOpaqueValue();
   }

   POLAR_DEBUG_DUMP;
   void print(raw_ostream &OS) const;
};

} // namespace polar::ast

#endif // POLARPHP_AST_CAPTURE_INFO_H
