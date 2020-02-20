//===--- GenProto.h - Swift IR generation for prototypes --------*- C++ -*-===//
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
//  This file provides the private interface to the protocol-emission code.
//
//===----------------------------------------------------------------------===//

#ifndef POLARPHP_IRGEN_INTERNAL_GENP_INTERFACE_H
#define POLARPHP_IRGEN_INTERNAL_GENP_INTERFACE_H

#include "polarphp/pil/lang/PILFunction.h"
#include "polarphp/irgen/internal/Fulfillment.h"
#include "polarphp/irgen/internal/GenericRequirement.h"

namespace llvm {
class Type;
}

namespace polar {
class AssociatedConformance;
class AssociatedType;
class CanType;
class FuncDecl;
enum class MetadataState : size_t;
class InterfaceConformanceRef;
struct PILDeclRef;
class PILType;
class PILFunction;

namespace irgen {

class Address;
class DynamicMetadataRequest;
class Explosion;
class FunctionPointer;
class IRGenFunction;
class IRGenModule;
class MetadataPath;
class MetadataResponse;
class InterfaceInfo;
class TypeInfo;

/// Set an LLVM value name for the given type metadata.
void setTypeMetadataName(IRGenModule &IGM, llvm::Value *value, CanType type);

/// Set an LLVM value name for the given protocol witness table.
void setInterfaceWitnessTableName(IRGenModule &IGM, llvm::Value *value,
                                  CanType type, InterfaceDecl *protocol);

/// Extract the method pointer from the given witness table
/// as a function value.
FunctionPointer emitWitnessMethodValue(IRGenFunction &IGF,
                                       llvm::Value *wtable,
                                       PILDeclRef member);

/// Extract the method pointer from an archetype's witness table
/// as a function value.
FunctionPointer emitWitnessMethodValue(IRGenFunction &IGF, CanType baseTy,
                                       llvm::Value **baseMetadataCache,
                                       PILDeclRef member,
                                       InterfaceConformanceRef conformance);

/// Compute the index into a witness table for a resilient protocol given
/// a reference to a descriptor of one of the requirements in that witness
/// table.
llvm::Value *computeResilientWitnessTableIndex(
   IRGenFunction &IGF,
   InterfaceDecl *proto,
   llvm::Constant *reqtDescriptor);

/// Given a type T and an associated type X of some protocol P to
/// which T conforms, return the type metadata for T.X.
///
/// \param parentMetadata - the type metadata for T
/// \param wtable - the witness table witnessing the conformance of T to P
/// \param associatedType - the declaration of X; a member of P
MetadataResponse emitAssociatedTypeMetadataRef(IRGenFunction &IGF,
                                               llvm::Value *parentMetadata,
                                               llvm::Value *wtable,
                                               AssociatedType associatedType,
                                               DynamicMetadataRequest request);

// Return the offset one should do on a witness table pointer to retrieve the
// `index`th piece of private data.
inline int privateWitnessTableIndexToTableOffset(unsigned index) {
   return -1 - (int)index;
}

/// Add the witness parameters necessary for calling a function with
/// the given generics clause.
void expandPolymorphicSignature(IRGenModule &IGM,
                                CanPILFunctionType type,
                                SmallVectorImpl<llvm::Type*> &types);

/// Return the number of trailing arguments necessary for calling a
/// witness method.
inline unsigned getTrailingWitnessSignatureLength(IRGenModule &IGM,
                                                  CanPILFunctionType type) {
   return 2;
}

/// Add the trailing arguments necessary for calling a witness method.
void expandTrailingWitnessSignature(IRGenModule &IGM,
                                    CanPILFunctionType type,
                                    SmallVectorImpl<llvm::Type*> &types);

struct WitnessMetadata {
   llvm::Value *SelfMetadata = nullptr;
   llvm::Value *SelfWitnessTable = nullptr;
};

/// Collect any required metadata for a witness method from the end
/// of the given parameter list.
void collectTrailingWitnessMetadata(IRGenFunction &IGF, PILFunction &fn,
                                    Explosion &params,
                                    WitnessMetadata &metadata);

using GetParameterFn = llvm::function_ref<llvm::Value*(unsigned)>;

/// In the prelude of a generic function, perform the bindings for a
/// generics clause.
///
/// \param witnessMetadata - can be omitted if the function is
///   definitely not a witness method
void emitPolymorphicParameters(IRGenFunction &IGF,
                               PILFunction &Fn,
                               Explosion &args,
                               WitnessMetadata *witnessMetadata,
                               const GetParameterFn &getParameter);

void emitPolymorphicParametersFromArray(IRGenFunction &IGF,
                                        NominalTypeDecl *typeDecl,
                                        Address array,
                                        MetadataState metadataState);

/// When calling a polymorphic call, pass the arguments for the
/// generics clause.
void emitPolymorphicArguments(IRGenFunction &IGF,
                              CanPILFunctionType origType,
                              SubstitutionMap subs,
                              WitnessMetadata *witnessMetadata,
                              Explosion &args);

/// Bind the polymorphic parameter inside of a partial apply forwarding thunk.
void bindPolymorphicParameter(IRGenFunction &IGF,
                              CanPILFunctionType &OrigFnType,
                              CanPILFunctionType &SubstFnType,
                              Explosion &nativeParam, unsigned paramIndex);

/// Load a reference to the protocol descriptor for the given protocol.
///
/// For Swift protocols, this is a constant reference to the protocol
/// descriptor symbol.
/// For ObjC protocols, descriptors are uniqued at runtime by the ObjC
/// runtime. We need to load the unique reference from a global variable fixed up at
/// startup.
llvm::Value *emitInterfaceDescriptorRef(IRGenFunction &IGF,
                                        InterfaceDecl *protocol);

/// Emit a witness table reference.
llvm::Value *emitWitnessTableRef(IRGenFunction &IGF,
                                 CanType srcType,
                                 llvm::Value **srcMetadataCache,
                                 InterfaceConformanceRef conformance);

llvm::Value *emitWitnessTableRef(IRGenFunction &IGF,
                                 CanType srcType,
                                 InterfaceConformanceRef conformance);

class MetadataSource {
public:
   enum class Kind {
      /// Metadata is derived from a source class pointer.
         ClassPointer,

      /// Metadata is derived from a type metadata pointer.
         Metadata,

      /// Metadata is derived from the origin type parameter.
         GenericLValueMetadata,

      /// Metadata is obtained directly from the from a Self metadata
      /// parameter passed via the WitnessMethod convention.
         SelfMetadata,

      /// Metadata is derived from the Self witness table parameter
      /// passed via the WitnessMethod convention.
         SelfWitnessTable,
   };

   static bool requiresSourceIndex(Kind kind) {
      return (kind == Kind::ClassPointer ||
              kind == Kind::Metadata ||
              kind == Kind::GenericLValueMetadata);
   }

   enum : unsigned { InvalidSourceIndex = ~0U };

private:
   /// The kind of source this is.
   Kind TheKind;

   /// The parameter index, for ClassPointer and Metadata sources.
   unsigned Index;

public:
   CanType Type;

   MetadataSource(Kind kind, unsigned index, CanType type)
      : TheKind(kind), Index(index), Type(type) {
      assert(index != InvalidSourceIndex || !requiresSourceIndex(kind));
   }

   Kind getKind() const { return TheKind; }
   unsigned getParamIndex() const {
      assert(requiresSourceIndex(getKind()));
      return Index;
   }
};

using GenericParamFulfillmentCallback =
llvm::function_ref<void(CanType genericParamType,
                        const MetadataSource &source,
                        const MetadataPath &path)>;

void enumerateGenericParamFulfillments(IRGenModule &IGM,
                                       CanPILFunctionType fnType,
                                       GenericParamFulfillmentCallback callback);
} // end namespace irgen
} // end namespace polar

#endif // POLARPHP_IRGEN_INTERNAL_GENP_INTERFACE_H