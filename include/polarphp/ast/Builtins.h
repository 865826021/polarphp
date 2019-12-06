//===--- Builtins.h - Swift Builtin Functions -------------------*- C++ -*-===//
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
//
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/04/26.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interface to builtin functions.
//
//===----------------------------------------------------------------------===//

#ifndef POLARPHP_AST_BUILTINS_H
#define POLARPHP_AST_BUILTINS_H

#include "polarphp/basic/LLVM.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Attributes.h"
#include "polarphp/ast/Type.h"
#include "polarphp/ast/Types.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {
enum class AtomicOrdering;
} // llvm

namespace polar::ast {

class AstContext;
class Identifier;
class ValueDecl;
class Type;

class AstContext;
class Identifier;
class ValueDecl;

enum class BuiltinTypeKind : std::underlying_type<TypeKind>::type {
#define TYPE(id, parent)
#define BUILTIN_TYPE(id, parent)                                               \
  id = std::underlying_type<TypeKind>::type(TypeKind::id),
#include "polarphp/ast/TypeNodesDef.h"
};

/// Get the builtin type for the given name.
///
/// Returns a null type if the name is not a known builtin type name.
Type getBuiltinType(AstContext &Context, StringRef Name);

/// OverloadedBuiltinKind - Whether and how a builtin is overloaded.
enum class OverloadedBuiltinKind : uint8_t {
  /// The builtin is not overloaded.
  None,

  /// The builtin is overloaded over all integer types.
  Integer,

  /// The builtin is overloaded over all integer types and vectors of integers.
  IntegerOrVector,

  /// The builtin is overloaded over all integer types and the raw pointer type.
  IntegerOrRawPointer,

  /// The builtin is overloaded over all integer types, the raw pointer type,
  /// and vectors of integers.
  IntegerOrRawPointerOrVector,

  /// The builtin is overloaded over all floating-point types.
  Float,

  /// The builtin is overloaded over all floating-point types and vectors of
  /// floating-point types.
  FloatOrVector,

  /// The builtin has custom processing.
  Special
};

/// BuiltinValueKind - The set of (possibly overloaded) builtin functions.
enum class BuiltinValueKind {
  None = 0,
#define BUILTIN(Id, Name, Attrs) Id,
#include "polarphp/ast/BuiltinsDef.h"
};

/// Returns true if this is a polymorphic builtin that is only valid
/// in raw sil and thus must be resolved to have concrete types by the
/// time we are in canonical SIL.
bool isPolymorphicBuiltin(BuiltinValueKind Id);

/// Decode the type list of a builtin (e.g. mul_Int32) and return the base
/// name (e.g. "mul").
StringRef getBuiltinBaseName(AstContext &C, StringRef Name,
                             SmallVectorImpl<Type> &Types);

/// Given an LLVM IR intrinsic name with argument types remove (e.g. like
/// "bswap") return the LLVM IR IntrinsicID for the intrinsic or not_intrinsic
/// (0) if the intrinsic name doesn't match anything.
llvm::Intrinsic::ID getLLVMIntrinsicID(StringRef Name);

/// Get the LLVM intrinsic ID that corresponds to the given builtin with
/// overflow.
llvm::Intrinsic::ID
getLLVMIntrinsicIDForBuiltinWithOverflow(BuiltinValueKind ID);

/// Create a ValueDecl for the builtin with the given name.
///
/// Returns null if the name does not identifier a known builtin value.
ValueDecl *getBuiltinValueDecl(AstContext &Context, Identifier Name);

/// Returns the name of a builtin declaration given a builtin ID.
StringRef getBuiltinName(BuiltinValueKind ID);

/// The information identifying the builtin - its kind and types.
class BuiltinInfo {
public:
  BuiltinValueKind ID;
  SmallVector<Type, 4> Types;
  bool isReadNone() const;
};

/// The information identifying the llvm intrinsic - its id and types.
class IntrinsicInfo {
  mutable llvm::AttributeList Attrs =
      llvm::DenseMapInfo<llvm::AttributeList>::getEmptyKey();
public:
  llvm::Intrinsic::ID ID;
  SmallVector<Type, 4> Types;
  bool hasAttribute(llvm::Attribute::AttrKind Kind) const;
};

/// Turn a string like "release" into the LLVM enum.
llvm::AtomicOrdering decodeLLVMAtomicOrdering(StringRef O);

/// Returns true if the builtin with ID \p ID has a defined static overload for
/// the type \p Ty.
bool canBuiltinBeOverloadedForType(BuiltinValueKind ID, Type Ty);

} // polar::ast

#endif // POLARPHP_AST_BUILTINS_H