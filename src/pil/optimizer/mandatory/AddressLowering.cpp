//===--- AddressLowering.cpp - Lower PIL address-only types. --------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
// This pass lowers PILTypes. On completion, the PILType of every PILValue is
// its PIL storage type. A PIL storage type is always an address type for values
// that require indirect storage at the LLVM IR level. Consequently, this pass
// is required for IRGen. It is a mandatory IRGen preparation pass (not a
// diagnostic pass).
//
// In the following text, items marked "[REUSE]" only apply to the proposed
// storage reuse optimization, which is not currently implemented.
//
// ## State
//
// A `valueStorageMap` maps each opaque PIL value to its storage
// information containing:
//
// - An ordinal representing the position of this instruction.
//
// - [REUSE] The identifier of the storage object. An optimized storage object
//   may have multiple disjoint lifetimes. A storage object may also have
//   subobjects. Each subobject has its own live range. When considering
//   liveness of the subobject, one must also consider liveness of the
//   parent object.
//
// - If this is a subobject projection, refer back to the value whose
//  storage object will be the parent that this storage address is a
//  projection of.
//
// - The storage address for this subobject.
//
// ## Step #1: Map opaque values
//
// Populate `valueStorageMap` in forward order (RPO), giving each opaque value
// an ordinal position.
//
// [REUSE] Assign a storage identifier to each opaque value. Optionally optimize
// storage by assigning multiple values the same identifier.
//
// ## Step #2: Allocate storage
//
// In reverse order (PO), allocate the parent storage object for each opaque
// value.
//
// [REUSE] If storage has already been allocated for the current live range,
// then simply reuse it.
//
// If the value's use composes a parent object from this value, and use's
// storage can be projected from, then mark the value's storage as a projection
// from the use value. [REUSE] Also inherit the use's storage identifier, and
// add an interval to the live range with the current projection path.
//
// A use can be projected from if its allocation is available at (dominates)
// this value and using the same storage over the interval from this value to
// the use does not overlap with the existing live range.
//
// Checking interference requires checking all operands that have been marked as
// projections. In the case of block arguments, it means checking the terminator
// operands of all predecessor blocks.
//
// [REUSE] Rather than checking all value operands, each live range will contain
// a set of intervals. Each interval will be associated with a projection path.
//
// Opaque value's that are the root of all projection paths now have their
// `storageAddress` assigned to an `alloc_stack` or argument. Opaque value's
// that are projections do not yet have a `storageAddress`.
//
// ## Step #3. Rewrite opaque values
//
// In forward order (RPO), rewrite each opaque value definition, and all its
// uses. This generally involves creating a new `_addr` variant of the
// instruction and obtaining the storage address from the `valueStorageMap`.
//
// If this value's storage is a projection of the value defined by its composing
// use, then first generate instructions to materialize the projection. This is
// a recursive process starting with the root of the projection path.
//
// A projection path will be materialized once, for the leaf subobject. When
// this happens, the `storageAddress` will be assigned for any intermediate
// projection paths. When those values are rewritten, their `storageAddress`
// will already be available.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "address-lowering"

#include "polarphp/pil/lang/DebugUtils.h"
#include "polarphp/pil/lang/PILArgument.h"
#include "polarphp/pil/lang/PILBuilder.h"
#include "polarphp/pil/lang/PILVisitor.h"
#include "polarphp/pil/optimizer/analysis/PostOrderAnalysis.h"
#include "polarphp/pil/optimizer/passmgr/Transforms.h"
#include "polarphp/pil/optimizer/utils/InstOptUtils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace polar;
using llvm::SmallSetVector;
using llvm::PointerIntPair;

llvm::cl::opt<bool>
   OptimizeOpaqueAddressLowering("optimize-opaque-address-lowering",
                                 llvm::cl::init(false));

// Visit all call results.
// Stop when the visitor returns `false`.
static void visitCallResults(ApplySite apply,
                             llvm::function_ref<bool(PILValue)> visitor) {
   // FIXME: this entire implementation only really works for ApplyInst.
   auto applyInst = cast<ApplyInst>(apply);
   if (applyInst->getType().is<TupleType>()) {
      // TODO: MultiValueInstruction
      for (auto *operand : applyInst->getUses()) {
         if (auto extract = dyn_cast<TupleExtractInst>(operand->getUser()))
            if (!visitor(extract))
               break;
      }
   } else
      visitor(applyInst);
}

//===----------------------------------------------------------------------===//
// ValueStorageMap: Map Opaque/Resilient PILValues to abstract storage units.
//===----------------------------------------------------------------------===//

namespace {
struct ValueStorage {
   enum { IsProjectionMask = 0x1, IsRewrittenMask = 0x2 };
   PointerIntPair<Operand *, 2, unsigned> projectionAndFlags;

   /// The final address of this storage unit after rewriting the PIL.
   /// For values linked to their own storage, this is set during storage
   /// allocation. For projections, it is only set after instruction rewriting.
   PILValue storageAddress;

   bool isProjection() const {
      return projectionAndFlags.getInt() & IsProjectionMask;
   }
   /// Return the operand the composes an aggregate from this value.
   Operand *getComposedOperand() const {
      assert(isProjection());
      return projectionAndFlags.getPointer();
   }
   void setComposedOperand(Operand *oper) {
      projectionAndFlags.setPointer(oper);
      projectionAndFlags.setInt(projectionAndFlags.getInt() | IsProjectionMask);
   }

   bool isRewritten() const {
      if (projectionAndFlags.getInt() & IsRewrittenMask) {
         assert(storageAddress);
         return true;
      }
      return false;
   }
   void markRewritten() {
      projectionAndFlags.setInt(projectionAndFlags.getInt() | IsRewrittenMask);
   }
};

/// Map each opaque/resilient PILValue to its abstract storage.
/// O(1) membership test.
/// O(n) iteration in RPO order.
class ValueStorageMap {
   typedef std::vector<std::pair<PILValue, ValueStorage>> ValueVector;
   // Hash of values to ValueVector indices.
   typedef llvm::DenseMap<PILValue, unsigned> ValueHashMap;

   ValueVector valueVector;
   ValueHashMap valueHashMap;

public:
   bool empty() const { return valueVector.empty(); }

   void clear() {
      valueVector.clear();
      valueHashMap.clear();
   }

   ValueVector::iterator begin() { return valueVector.begin(); }

   ValueVector::iterator end() { return valueVector.end(); }

   ValueVector::reverse_iterator rbegin() { return valueVector.rbegin(); }

   ValueVector::reverse_iterator rend() { return valueVector.rend(); }

   bool contains(PILValue value) const {
      return valueHashMap.find(value) != valueHashMap.end();
   }

   unsigned getOrdinal(PILValue value) {
      auto hashIter = valueHashMap.find(value);
      assert(hashIter != valueHashMap.end() && "Missing PILValue");
      return hashIter->second;
   }

   ValueStorage &getStorage(PILValue value) {
      return valueVector[getOrdinal(value)].second;
   }

   // This must be called in RPO order.
   ValueStorage &insertValue(PILValue value) {
      auto hashResult =
         valueHashMap.insert(std::make_pair(value, valueVector.size()));
      (void)hashResult;
      assert(hashResult.second && "PILValue already mapped");

      valueVector.emplace_back(value, ValueStorage());

      return valueVector.back().second;
   }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// AddressLoweringState: shared state for the pass's analysis and transforms.
//===----------------------------------------------------------------------===//

namespace {
struct AddressLoweringState {
   PILFunction *F;
   PILFunctionConventions loweredFnConv;

   // Dominators remain valid throughout this pass.
   DominanceInfo *domInfo;

   // All opaque values and associated storage.
   ValueStorageMap valueStorageMap;
   // All call sites with formally indirect PILArgument or PILResult conventions.
   // Calls are removed from the set when rewritten.
   SmallSetVector<ApplySite, 16> indirectApplies;
   // All function-exiting terminators (return or throw instructions).
   SmallVector<TermInst *, 8> returnInsts;
   // Delete these instructions after performing transformations.
   // They must not have any remaining users.
   SmallSetVector<PILInstruction *, 16> instsToDelete;

   AddressLoweringState(PILFunction *F, DominanceInfo *domInfo)
      : F(F),
        loweredFnConv(F->getLoweredFunctionType(),
                      PILModuleConventions::getLoweredAddressConventions(F->getModule())),
        domInfo(domInfo) {}

   bool isDead(PILInstruction *inst) const { return instsToDelete.count(inst); }

   void markDead(PILInstruction *inst) {
#ifndef NDEBUG
      for (auto result : inst->getResults())
         for (Operand *use : result->getUses())
            assert(instsToDelete.count(use->getUser()));
#endif
      instsToDelete.insert(inst);
   }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// OpaqueValueVisitor: Map OpaqueValues to ValueStorage.
//===----------------------------------------------------------------------===//

namespace {
/// Collect all opaque/resilient values, inserting them in `valueStorageMap` in
/// RPO order.
///
/// Collect all call arguments with formally indirect PIL argument convention in
/// `indirectOperands` and formally indirect PIL results in `indirectResults`.
///
/// TODO: Perform linear-scan style in-place stack slot coloring by keeping
/// track of each value's last use.
class OpaqueValueVisitor {
   AddressLoweringState &pass;
   PostOrderFunctionInfo postorderInfo;

public:
   explicit OpaqueValueVisitor(AddressLoweringState &pass)
      : pass(pass), postorderInfo(pass.F) {}

   void mapValueStorage();

protected:
   void visitApply(ApplySite applySite);
   void visitValue(PILValue value);
};
} // end anonymous namespace

/// Top-level entry: Populate `valueStorageMap`, `indirectResults`, and
/// `indirectOperands`.
///
/// Find all Opaque/Resilient PILValues and add them
/// to valueStorageMap in RPO.
void OpaqueValueVisitor::mapValueStorage() {
   for (auto *BB : postorderInfo.getReversePostOrder()) {
      if (BB->getTerminator()->isFunctionExiting())
         pass.returnInsts.push_back(BB->getTerminator());

      // Opaque function arguments have already been replaced.
      if (BB != pass.F->getEntryBlock()) {
         for (auto argI = BB->args_begin(), argEnd = BB->args_end();
              argI != argEnd; ++argI) {
            visitValue(*argI);
         }
      }
      for (auto &II : *BB) {
         if (auto apply = ApplySite::isa(&II))
            visitApply(apply);

         for (auto result : II.getResults())
            visitValue(result);
      }
   }
}

/// Populate `indirectApplies` and insert this apply in `valueStorageMap` if
/// the call's non-tuple result is returned indirectly.
void OpaqueValueVisitor::visitApply(ApplySite applySite) {
   auto calleeConv = applySite.getSubstCalleeConv();
   unsigned calleeArgIdx = applySite.getCalleeArgIndexOfFirstAppliedArg();
   for (Operand &operand : applySite.getArgumentOperands()) {
      if (operand.get()->getType().isObject()) {
         auto argConv = calleeConv.getPILArgumentConvention(calleeArgIdx);
         if (argConv.isIndirectConvention()) {
            pass.indirectApplies.insert(applySite);
         }
      }
      ++calleeArgIdx;
   }

   if (applySite.getSubstCalleeType()->hasIndirectFormalResults()) {
      pass.indirectApplies.insert(applySite);
      if (!applySite.getType().is<TupleType>())
         pass.valueStorageMap.insertValue(cast<ApplyInst>(applySite));

      return;
   }
}

/// If `value` is address-only add it to the `valueStorageMap`.
void OpaqueValueVisitor::visitValue(PILValue value) {
   if (value->getType().isObject()
       && value->getType().isAddressOnly(*pass.F)) {
      if (pass.valueStorageMap.contains(value)) {
         assert(isa<PILFunctionArgument>(
            pass.valueStorageMap.getStorage(value).storageAddress));
         return;
      }
      pass.valueStorageMap.insertValue(value);
   }
}

//===----------------------------------------------------------------------===//
// OpaqueStorageAllocation: Generate alloc_stack and address projections for all
// abstract storage locations.
//===----------------------------------------------------------------------===//

namespace {
/// Allocate storage on the stack for every opaque value defined in this
/// function in RPO order. If the definition is an argument of this function,
/// simply replace the function argument with an address representing the
/// caller's storage.
///
/// TODO: shrink lifetimes by inserting alloc_stack at the dominance LCA and
/// finding the lifetime boundary with a simple backward walk from uses.
class OpaqueStorageAllocation {
   AddressLoweringState &pass;

public:
   explicit OpaqueStorageAllocation(AddressLoweringState &pass) : pass(pass) {}

   void allocateOpaqueStorage();

protected:
   void convertIndirectFunctionArgs();
   unsigned insertIndirectReturnArgs();
   bool canProjectFrom(SingleValueInstruction *innerVal,
                       PILInstruction *composingUse);
   void allocateForValue(PILValue value, ValueStorage &storage);
};
} // end anonymous namespace

/// Top-level entry point: allocate storage for all opaque/resilient values.
void OpaqueStorageAllocation::allocateOpaqueStorage() {
   // Fixup this function's argument types with temporary loads.
   convertIndirectFunctionArgs();

   // Create a new function argument for each indirect result.
   insertIndirectReturnArgs();

   // Populate valueStorageMap.
   OpaqueValueVisitor(pass).mapValueStorage();

   // Create an AllocStack for every opaque value defined in the function.  Visit
   // values in post-order to create storage for aggregates before subobjects.
   for (auto &valueStorageI : llvm::reverse(pass.valueStorageMap))
      allocateForValue(valueStorageI.first, valueStorageI.second);
}

/// Replace each value-typed argument to the current function with an
/// address-typed argument by inserting a temporary load instruction.
void OpaqueStorageAllocation::convertIndirectFunctionArgs() {
   // Insert temporary argument loads at the top of the function.
   PILBuilder argBuilder(pass.F->getEntryBlock()->begin());
   argBuilder.setPILConventions(
      PILModuleConventions::getLoweredAddressConventions(pass.F->getModule()));

   auto fnConv = pass.F->getConventions();
   unsigned argIdx = fnConv.getPILArgIndexOfFirstParam();
   for (PILParameterInfo param :
      pass.F->getLoweredFunctionType()->getParameters()) {

      if (param.isFormalIndirect() && !fnConv.isPILIndirect(param)) {
         PILArgument *arg = pass.F->getArgument(argIdx);
         PILType addrType = arg->getType().getAddressType();

         LoadInst *loadArg = argBuilder.createLoad(
            RegularLocation(const_cast<ValueDecl *>(arg->getDecl())),
            PILUndef::get(addrType, *pass.F),
            LoadOwnershipQualifier::Unqualified);

         arg->replaceAllUsesWith(loadArg);
         assert(!pass.valueStorageMap.contains(arg));

         arg = arg->getParent()->replaceFunctionArgument(
            arg->getIndex(), addrType, ValueOwnershipKind::None, arg->getDecl());

         loadArg->setOperand(arg);

         if (addrType.isAddressOnly(*pass.F))
            pass.valueStorageMap.insertValue(loadArg).storageAddress = arg;
      }
      ++argIdx;
   }
   assert(argIdx
          == fnConv.getPILArgIndexOfFirstParam() + fnConv.getNumPILArguments());
}

/// Insert function arguments for any @out result type. Return the number of
/// indirect result arguments added.
unsigned OpaqueStorageAllocation::insertIndirectReturnArgs() {
   auto &ctx = pass.F->getModule().getAstContext();
   unsigned argIdx = 0;
   for (auto resultTy : pass.loweredFnConv.getIndirectPILResultTypes()) {
      auto bodyResultTy = pass.F->mapTypeIntoContext(resultTy);
      auto var = new (ctx)
         ParamDecl(SourceLoc(), SourceLoc(),
                   ctx.getIdentifier("$return_value"), SourceLoc(),
                   ctx.getIdentifier("$return_value"),
                   pass.F->getDeclContext());
      var->setSpecifier(ParamSpecifier::InOut);

      pass.F->begin()->insertFunctionArgument(
         argIdx, bodyResultTy.getAddressType(), ValueOwnershipKind::None, var);
      ++argIdx;
   }
   assert(argIdx == pass.loweredFnConv.getNumIndirectPILResults());
   return argIdx;
}

/// Is this operand composing an aggregate from a subobject, or simply
/// forwarding the operand's value to storage defined elsewhere?
///
/// TODO: Handle struct.
/// TODO: Make this a visitor.
bool OpaqueStorageAllocation::canProjectFrom(SingleValueInstruction *innerVal,
                                             PILInstruction *composingUse) {
   if (!OptimizeOpaqueAddressLowering)
      return false;

   PILValue composingValue;
   switch (composingUse->getKind()) {
      default:
         return false;
      case PILInstructionKind::ApplyInst:
         // @in operands never need their own storage since they are non-mutating
         // uses. They simply reuse the storage allocated for their operand. So it
         // wouldn't make sense to "project" out of the apply argument.
         return false;
      case PILInstructionKind::EnumInst:
         composingValue = cast<EnumInst>(composingUse);
         break;
      case PILInstructionKind::InitExistentialValueInst: {
         // Ensure that all opened archetypes are available at the inner value's
         // definition.
         auto *initExistential = cast<InitExistentialValueInst>(composingUse);
         for (Operand &operand : initExistential->getTypeDependentOperands()) {
            if (!pass.domInfo->properlyDominates(operand.get(), innerVal))
               return false;
         }
         composingValue = initExistential;
         break;
      }
      case PILInstructionKind::ReturnInst:
         return true;
      case PILInstructionKind::StoreInst: {
         if (cast<StoreInst>(composingUse)->getSrc() == innerVal
             && isa<CopyValueInst>(innerVal)) {
            return true;
         }
         return false;
      }
      case PILInstructionKind::TupleInst:
         composingValue = cast<TupleInst>(composingUse);
         break;
   }
   ValueStorage &storage = pass.valueStorageMap.getStorage(composingValue);
   if (PILValue addr = storage.storageAddress) {
      if (auto *stackInst = dyn_cast<AllocStackInst>(addr)) {
         assert(pass.domInfo->properlyDominates(stackInst, innerVal));
         return true;
      }
      if (isa<PILFunctionArgument>(addr)) {
         return true;
      }
   } else if (storage.isProjection())
      return canProjectFrom(innerVal, storage.getComposedOperand()->getUser());

   return false;
}

/// Allocate storage for a single opaque/resilient value.
void OpaqueStorageAllocation::allocateForValue(PILValue value,
                                               ValueStorage &storage) {
   assert(!isa<PILFunctionArgument>(value));

   if (auto apply = ApplySite::isa(value)) {
      // Result tuples will be canonicalized during apply rewriting so the tuple
      // itself is unused.
      if (value->getType().is<TupleType>()) {
         assert(apply.getSubstCalleeType()->getNumResults() > 1);
         return;
      }
   }

   // Argument loads already have a storage address.
   if (storage.storageAddress) {
      assert(isa<PILFunctionArgument>(storage.storageAddress));
      return;
   }

   if (value->hasOneUse()) {
      // TODO: Handle block arguments.
      // TODO: Handle subobjects with a single composition, and other non-mutating
      // uses such as @in arguments.
      if (auto *def = dyn_cast<SingleValueInstruction>(value)) {
         Operand *useOper = *value->use_begin();
         if (canProjectFrom(def, useOper->getUser())) {
            storage.setComposedOperand(useOper);
            return;
         }
      }
   }

   PILBuilder allocBuilder(pass.F->begin()->begin());
   allocBuilder.setPILConventions(
      PILModuleConventions::getLoweredAddressConventions(pass.F->getModule()));
   AllocStackInst *allocInstr =
      allocBuilder.createAllocStack(value.getLoc(), value->getType());

   storage.storageAddress = allocInstr;

   // Insert stack deallocations.
   for (TermInst *termInst : pass.returnInsts) {
      PILBuilder deallocBuilder(termInst);
      deallocBuilder.setPILConventions(
         PILModuleConventions::getLoweredAddressConventions(pass.F->getModule()));
      deallocBuilder.createDeallocStack(allocInstr->getLoc(), allocInstr);
   }
}

//===----------------------------------------------------------------------===//
// AddressMaterialization - materialize storage addresses, generate projections.
//===----------------------------------------------------------------------===//

namespace {
/// Materialize the address of a value's storage. For values that are directly
/// mapped to a storage location, simply return the mapped `AllocStackInst`.
/// For subobjects emit any necessary `_addr` projections using the provided
/// `PILBuilder`.
///
/// This is a common utility for ApplyRewriter, AddressOnlyDefRewriter,
/// and AddressOnlyUseRewriter.
class AddressMaterialization {
   AddressLoweringState &pass;
   PILBuilder &B;

public:
   AddressMaterialization(AddressLoweringState &pass, PILBuilder &B)
      : pass(pass), B(B) {}

   PILValue initializeOperandMem(Operand *operand);

   PILValue materializeAddress(PILValue origValue);

protected:
   PILValue materializeProjection(Operand *operand);
};
} // anonymous namespace

// Materialize an address pointing to initialized memory for this operand,
// generating a projection and copy if needed.
PILValue AddressMaterialization::initializeOperandMem(Operand *operand) {
   PILValue def = operand->get();
   PILValue destAddr;
   if (operand->get()->getType().isAddressOnly(*pass.F)) {
      ValueStorage &storage = pass.valueStorageMap.getStorage(def);
      // Source value should already be rewritten.
      assert(storage.isRewritten());
      if (storage.isProjection())
         destAddr = storage.storageAddress;
      else {
         destAddr = materializeProjection(operand);
         B.createCopyAddr(operand->getUser()->getLoc(), storage.storageAddress,
                          destAddr, IsTake, IsInitialization);
      }
   } else {
      destAddr = materializeProjection(operand);
      B.createStore(operand->getUser()->getLoc(), operand->get(), destAddr,
                    StoreOwnershipQualifier::Unqualified);
   }
   return destAddr;
}

/// Return the address of the storage for `origValue`. This may involve
/// materializing projections.
PILValue AddressMaterialization::materializeAddress(PILValue origValue) {
   ValueStorage &storage = pass.valueStorageMap.getStorage(origValue);

   if (!storage.storageAddress)
      storage.storageAddress =
         materializeProjection(storage.getComposedOperand());

   return storage.storageAddress;
}

PILValue AddressMaterialization::materializeProjection(Operand *operand) {
   PILInstruction *user = operand->getUser();

   switch (user->getKind()) {
      default:
         LLVM_DEBUG(user->dump());
         llvm_unreachable("Unexpected subobject composition.");
      case PILInstructionKind::EnumInst: {
         auto *enumInst = cast<EnumInst>(user);
         PILValue enumAddr = materializeAddress(enumInst);
         return B.createInitEnumDataAddr(enumInst->getLoc(), enumAddr,
                                         enumInst->getElement(),
                                         operand->get()->getType().getAddressType());
      }
      case PILInstructionKind::InitExistentialValueInst: {
         auto *initExistentialValue = cast<InitExistentialValueInst>(user);
         PILValue containerAddr = materializeAddress(initExistentialValue);
         auto canTy = initExistentialValue->getFormalConcreteType();
         auto opaque = lowering::AbstractionPattern::getOpaque();
         auto &concreteTL = pass.F->getTypeLowering(opaque, canTy);
         return B.createInitExistentialAddr(
            initExistentialValue->getLoc(), containerAddr, canTy,
            concreteTL.getLoweredType(), initExistentialValue->getConformances());
      }
      case PILInstructionKind::ReturnInst: {
         assert(pass.loweredFnConv.hasIndirectPILResults());
         return pass.F->getArguments()[0];
      }
      case PILInstructionKind::TupleInst: {
         auto *tupleInst = cast<TupleInst>(user);
         // Function return values.
         if (tupleInst->hasOneUse()
             && isa<ReturnInst>(tupleInst->use_begin()->getUser())) {
            unsigned resultIdx = tupleInst->getElementIndex(operand);
            assert(resultIdx < pass.loweredFnConv.getNumIndirectPILResults());
            // Cannot call getIndirectPILResults here because that API uses the
            // original function type.
            return pass.F->getArguments()[resultIdx];
         }
         // TODO: emit tuple_element_addr
         llvm_unreachable("Unimplemented");
      }
   }
}

//===----------------------------------------------------------------------===//
// ApplyRewriter - rewrite call sites with indirect arguments.
//===----------------------------------------------------------------------===//

namespace {
/// Rewrite an Apply, lowering its indirect PIL arguments.
///
/// Replace indirect parameter arguments of this function with address-type
/// arguments.
///
/// Insert new indirect result arguments for this function to represent the
/// caller's storage.
class ApplyRewriter {
   AddressLoweringState &pass;
   ApplySite apply;
   PILBuilder argBuilder;

   /// For now, we assume that the apply site is a normal apply.
   ApplyInst *getApplyInst() const { return cast<ApplyInst>(apply); }

public:
   ApplyRewriter(ApplySite origCall, AddressLoweringState &pass)
      : pass(pass), apply(origCall), argBuilder(origCall.getInstruction()) {
      argBuilder.setPILConventions(
         PILModuleConventions::getLoweredAddressConventions(origCall.getModule()));
   }

   void rewriteParameters();
   void rewriteIndirectParameter(Operand *operand);

   void convertApplyWithIndirectResults();

protected:
   void
   canonicalizeResults(MutableArrayRef<SingleValueInstruction *> directResultValues,
                       ArrayRef<Operand *> nonCanonicalUses);
   PILValue materializeIndirectResultAddress(
      SingleValueInstruction *origDirectResultVal,
      PILType argTy);
};
} // end anonymous namespace

/// Rewrite any indirect parameter in place.
void ApplyRewriter::rewriteParameters() {
   // Rewrite all incoming indirect operands.
   unsigned calleeArgIdx = apply.getCalleeArgIndexOfFirstAppliedArg();
   for (Operand &operand : apply.getArgumentOperands()) {
      if (operand.get()->getType().isObject()) {
         auto argConv =
            apply.getSubstCalleeConv().getPILArgumentConvention(calleeArgIdx);
         if (argConv.isIndirectConvention())
            rewriteIndirectParameter(&operand);
      }
      ++calleeArgIdx;
   }
}

/// Deallocate temporary call-site stack storage.
///
/// `argLoad` is non-null for @out args that are loaded.
static void insertStackDeallocationAtCall(AllocStackInst *allocInst,
                                          PILInstruction *applyInst,
                                          PILInstruction *argLoad) {
   PILInstruction *lastUse = argLoad ? argLoad : applyInst;

   switch (applyInst->getKind()) {
      case PILInstructionKind::ApplyInst: {
         PILBuilder deallocBuilder(&*std::next(lastUse->getIterator()));
         deallocBuilder.setPILConventions(
            PILModuleConventions::getLoweredAddressConventions(applyInst->getModule()));
         deallocBuilder.createDeallocStack(allocInst->getLoc(), allocInst);
         break;
      }
      case PILInstructionKind::TryApplyInst:
         // TODO!!!: insert dealloc in the catch block.
         llvm_unreachable("not implemented for this instruction!");
      case PILInstructionKind::PartialApplyInst:
         llvm_unreachable("partial apply cannot have indirect results.");
      default:
         llvm_unreachable("not implemented for this instruction!");
   }
}

/// Rewrite a formally indirect parameter in place.
/// Update the operand to the incoming value's storage address.
/// After this, the PIL argument types no longer match PIL function conventions.
///
/// Temporary argument storage may be created for loadable values.
///
/// Note: Temporary argument storage does not own its value. If the argument
/// is owned, the stored value should already have been copied.
void ApplyRewriter::rewriteIndirectParameter(Operand *operand) {
   PILValue argValue = operand->get();

   if (argValue->getType().isAddressOnly(*pass.F)) {
      ValueStorage &storage = pass.valueStorageMap.getStorage(argValue);
      // Source value should already be rewritten.
      assert(storage.isRewritten());
      operand->set(storage.storageAddress);
      return;
   }
   // Allocate temporary storage for a loadable operand.
   AllocStackInst *allocInstr =
      argBuilder.createAllocStack(apply.getLoc(), argValue->getType());

   argBuilder.createStore(apply.getLoc(), argValue, allocInstr,
                          StoreOwnershipQualifier::Unqualified);

   operand->set(allocInstr);

   insertStackDeallocationAtCall(allocInstr, apply.getInstruction(),
      /*argLoad=*/nullptr);
}

// Canonicalize call result uses. Treat each result of a multi-result call as
// an independent value. Currently, PILGen may generate tuple_extract for each
// result but generate a single destroy_value for the entire tuple of
// results. This makes it impossible to reason about each call result as an
// independent value according to the callee's function type.
//
// directResultValues has an entry for each tuple extract corresponding to
// that result if one exists. This function will add an entry to
// directResultValues whenever it needs to materialize a TupleExtractInst.
void ApplyRewriter::canonicalizeResults(
   MutableArrayRef<SingleValueInstruction *> directResultValues,
   ArrayRef<Operand *> nonCanonicalUses) {

   auto *applyInst = getApplyInst();

   for (Operand *operand : nonCanonicalUses) {
      auto *destroyInst = dyn_cast<DestroyValueInst>(operand->getUser());
      if (!destroyInst)
         llvm::report_fatal_error("Simultaneous use of multiple call results.");

      for (unsigned resultIdx : indices(directResultValues)) {
         SingleValueInstruction *result = directResultValues[resultIdx];
         if (!result) {
            PILBuilder resultBuilder(std::next(PILBasicBlock::iterator(applyInst)));
            resultBuilder.setPILConventions(
               PILModuleConventions::getLoweredAddressConventions(applyInst->getModule()));
            result = resultBuilder.createTupleExtract(applyInst->getLoc(),
                                                      applyInst, resultIdx);
            directResultValues[resultIdx] = result;
         }
         PILBuilder B(destroyInst);
         B.setPILConventions(PILModuleConventions::getLoweredAddressConventions(applyInst->getModule()));
         auto &TL = pass.F->getTypeLowering(result->getType());
         TL.emitDestroyValue(B, destroyInst->getLoc(), result);
      }
      destroyInst->eraseFromParent();
   }
}

/// Return the storage address for the indirect result corresponding to the
/// given original result value. Allocate temporary argument storage for any
/// indirect results that are unmapped because they are loadable or unused.
///
/// origDirectResultVal may be nullptr for unused results.
PILValue ApplyRewriter::materializeIndirectResultAddress(
   SingleValueInstruction *origDirectResultVal, PILType argTy) {

   if (origDirectResultVal
       && origDirectResultVal->getType().isAddressOnly(*pass.F)) {
      auto &storage = pass.valueStorageMap.getStorage(origDirectResultVal);
      storage.markRewritten();
      // Pass the local storage address as the indirect result address.
      return storage.storageAddress;
   }
   // Allocate temporary call-site storage for an unused or loadable result.
   PILInstruction *origCallInst = apply.getInstruction();
   PILLocation loc = origCallInst->getLoc();
   auto *allocInst = argBuilder.createAllocStack(loc, argTy);
   LoadInst *loadInst = nullptr;
   if (origDirectResultVal) {
      // TODO: Find the try_apply's result block.
      // Build results outside-in to next stack allocations.
      PILBuilder resultBuilder(std::next(PILBasicBlock::iterator(origCallInst)));
      resultBuilder.setPILConventions(
         PILModuleConventions::getLoweredAddressConventions(origCallInst->getModule()));
      // This is a formally indirect argument, but is loadable.
      loadInst = resultBuilder.createLoad(loc, allocInst,
                                          LoadOwnershipQualifier::Unqualified);
      origDirectResultVal->replaceAllUsesWith(loadInst);
      pass.markDead(origDirectResultVal);
   }
   insertStackDeallocationAtCall(allocInst, origCallInst, loadInst);
   return PILValue(allocInst);
}

/// Allocate storage for formally indirect results at the given call site.
/// Create a new call instruction with indirect PIL arguments.
void ApplyRewriter::convertApplyWithIndirectResults() {
   assert(apply.getSubstCalleeType()->hasIndirectFormalResults());

   auto *origCallInst = getApplyInst();
   PILFunctionConventions origFnConv = apply.getSubstCalleeConv();

   // Gather the original direct return values.
   // Canonicalize results so no user uses more than one result.
   SmallVector<SingleValueInstruction *, 8> origDirectResultValues(
      origFnConv.getNumDirectPILResults());
   SmallVector<Operand *, 4> nonCanonicalUses;
   if (origCallInst->getType().is<TupleType>()) {
      for (Operand *operand : origCallInst->getUses()) {
         if (auto *extract = dyn_cast<TupleExtractInst>(operand->getUser()))
            origDirectResultValues[extract->getFieldNo()] = extract;
         else
            nonCanonicalUses.push_back(operand);
      }
      if (!nonCanonicalUses.empty())
         canonicalizeResults(origDirectResultValues, nonCanonicalUses);
   } else {
      // This call has a single, indirect result (convertApplyWithIndirectResults
      // only handles call with at least one indirect result).
      // An unused result can remain unmapped. Temporary storage will be allocated
      // later when fixing up the call's uses.
      assert(origDirectResultValues.size() == 1);
      if (!origCallInst->use_empty()) {
         assert(pass.valueStorageMap.contains(origCallInst));
         origDirectResultValues[0] = origCallInst;
      }
   }

   // Prepare to emit a new call instruction.
   PILLocation loc = origCallInst->getLoc();
   PILBuilder callBuilder(origCallInst);
   callBuilder.setPILConventions(
      PILModuleConventions::getLoweredAddressConventions(origCallInst->getModule()));

   // The new call instruction's PIL calling convention.
   PILFunctionConventions loweredCalleeConv(
      apply.getSubstCalleeType(),
      PILModuleConventions::getLoweredAddressConventions(origCallInst->getModule()));

   // The new call instruction's PIL argument list.
   SmallVector<PILValue, 8> newCallArgs(loweredCalleeConv.getNumPILArguments());

   // Map the original result indices to new result indices.
   SmallVector<unsigned, 8> newDirectResultIndices(
      origFnConv.getNumDirectPILResults());
   // Indices used to populate newDirectResultIndices.
   unsigned oldDirectResultIdx = 0, newDirectResultIdx = 0;

   // The index of the next indirect result argument.
   unsigned newResultArgIdx =
      loweredCalleeConv.getPILArgIndexOfFirstIndirectResult();

   // Visit each result. Redirect results that are now indirect by calling
   // materializeIndirectResultAddress.  Result that remain direct will be
   // redirected later.  Populate newCallArgs and newDirectResultIndices.
   for_each(
      apply.getSubstCalleeType()->getResults(),
      origDirectResultValues,
      [&](PILResultInfo resultInfo, SingleValueInstruction *origDirectResultVal) {
         // Assume that all original results are direct in PIL.
         assert(!origFnConv.isPILIndirect(resultInfo));

         if (loweredCalleeConv.isPILIndirect(resultInfo)) {
            PILValue indirectResultAddr = materializeIndirectResultAddress(
               origDirectResultVal, loweredCalleeConv.getPILType(resultInfo));
            // Record the new indirect call argument.
            newCallArgs[newResultArgIdx++] = indirectResultAddr;
            // Leave a placeholder for indirect results.
            newDirectResultIndices[oldDirectResultIdx++] = ~0;
         } else {
            // Record the new direct result, and advance the direct result indices.
            newDirectResultIndices[oldDirectResultIdx++] = newDirectResultIdx++;
         }
         // replaceAllUses will be called later to handle direct results that
         // remain direct results of the new call instruction.
      });

   // Append the existing call arguments to the PIL argument list. They were
   // already lowered to addresses by rewriteIncomingArgument.
   assert(newResultArgIdx == loweredCalleeConv.getPILArgIndexOfFirstParam());
   unsigned origArgIdx = apply.getSubstCalleeConv().getPILArgIndexOfFirstParam();
   for (unsigned endIdx = newCallArgs.size(); newResultArgIdx < endIdx;
        ++newResultArgIdx, ++origArgIdx) {
      newCallArgs[newResultArgIdx] = apply.getArgument(origArgIdx);
   }

   // Create a new apply with indirect result operands.
   ApplyInst *newCallInst;
   switch (origCallInst->getKind()) {
      case PILInstructionKind::ApplyInst:
         newCallInst = callBuilder.createApply(
            loc, apply.getCallee(), apply.getSubstitutionMap(), newCallArgs,
            cast<ApplyInst>(origCallInst)->isNonThrowing());
         break;
      case PILInstructionKind::TryApplyInst:
         // TODO: insert dealloc in the catch block.
         llvm_unreachable("not implemented for this instruction!");
      case PILInstructionKind::PartialApplyInst:
         // Partial apply does not have formally indirect results.
      default:
         llvm_unreachable("not implemented for this instruction!");
   }

   // Replace all unmapped uses of the original call with uses of the new call.
   //
   // TODO: handle bbargs from try_apply.
   PILBuilder resultBuilder(
      std::next(PILBasicBlock::iterator(origCallInst)));
   resultBuilder.setPILConventions(
      PILModuleConventions::getLoweredAddressConventions(apply.getModule()));

   SmallVector<Operand*, 8> origUses(origCallInst->getUses());
   for (Operand *operand : origUses) {
      auto *extractInst = dyn_cast<TupleExtractInst>(operand->getUser());
      if (!extractInst) {
         assert(origFnConv.getNumDirectPILResults() == 1);
         assert(pass.valueStorageMap.contains(origCallInst));
         continue;
      }
      unsigned origResultIdx = extractInst->getFieldNo();
      auto resultInfo = origFnConv.getResults()[origResultIdx];

      if (extractInst->getType().isAddressOnly(*pass.F)) {
         // Uses of indirect results will be rewritten by AddressOnlyUseRewriter.
         assert(loweredCalleeConv.isPILIndirect(resultInfo));
         assert(pass.valueStorageMap.contains(extractInst));
         if (extractInst->use_empty())
            pass.markDead(extractInst);
         continue;
      }
      if (loweredCalleeConv.isPILIndirect(resultInfo)) {
         // This loadable indirect use should already be redirected to a load from
         // the argument storage and marked dead.
         assert(extractInst->use_empty());
         continue;
      }
      // Either the new call instruction has only a single direct result, or we
      // map the original tuple field to the new tuple field.
      PILValue newValue = newCallInst;
      if (loweredCalleeConv.getNumDirectPILResults() > 1) {
         assert(newValue->getType().is<TupleType>());
         newValue = resultBuilder.createTupleExtract(
            extractInst->getLoc(), newValue,
            newDirectResultIndices[origResultIdx]);
      }
      extractInst->replaceAllUsesWith(newValue);
      extractInst->eraseFromParent();
   }
   if (!pass.valueStorageMap.contains(origCallInst))
      pass.markDead(origCallInst);
}

//===----------------------------------------------------------------------===//
// ReturnRewriter - rewrite return instructions for indirect results.
//===----------------------------------------------------------------------===//

class ReturnRewriter {
   AddressLoweringState &pass;

public:
   ReturnRewriter(AddressLoweringState &pass) : pass(pass) {}

   void rewriteReturns();

protected:
   void rewriteReturn(ReturnInst *returnInst);
};

void ReturnRewriter::rewriteReturns() {
   for (TermInst *termInst : pass.returnInsts) {
      // TODO: handle throws
      rewriteReturn(cast<ReturnInst>(termInst));
   }
}

void ReturnRewriter::rewriteReturn(ReturnInst *returnInst) {
   auto insertPt = PILBasicBlock::iterator(returnInst);
   auto bbStart = returnInst->getParent()->begin();
   while (insertPt != bbStart) {
      --insertPt;
      if (!isa<DeallocStackInst>(*insertPt))
         break;
   }
   PILBuilder B(insertPt);
   B.setPILConventions(
      PILModuleConventions::getLoweredAddressConventions(returnInst->getModule()));

   // Gather direct function results.
   unsigned numOrigDirectResults =
      pass.F->getConventions().getNumDirectPILResults();
   SmallVector<PILValue, 8> origDirectResultValues;
   if (numOrigDirectResults == 1)
      origDirectResultValues.push_back(returnInst->getOperand());
   else {
      auto *tupleInst = cast<TupleInst>(returnInst->getOperand());
      origDirectResultValues.append(tupleInst->getElements().begin(),
                                    tupleInst->getElements().end());
      assert(origDirectResultValues.size() == numOrigDirectResults);
   }

   PILFunctionConventions origFnConv(pass.F->getConventions());
   (void)origFnConv;

   // Convert each result.
   SmallVector<PILValue, 8> newDirectResults;
   unsigned newResultArgIdx =
      pass.loweredFnConv.getPILArgIndexOfFirstIndirectResult();

   for_each(
      pass.F->getLoweredFunctionType()->getResults(), origDirectResultValues,
      [&](PILResultInfo resultInfo, PILValue origDirectResultVal) {
         // Assume that all original results are direct in PIL.
         assert(!origFnConv.isPILIndirect(resultInfo));

         if (pass.loweredFnConv.isPILIndirect(resultInfo)) {
            assert(newResultArgIdx
                   < pass.loweredFnConv.getPILArgIndexOfFirstParam());

            PILArgument *resultArg = B.getFunction().getArgument(newResultArgIdx);
            PILType resultTy = origDirectResultVal->getType();
            if (resultTy.isAddressOnly(*pass.F)) {
               ValueStorage &storage =
                  pass.valueStorageMap.getStorage(origDirectResultVal);
               assert(storage.isRewritten());
               if (!storage.isProjection()) {
                  // Copy the result from local storage into the result argument.
                  PILValue resultAddr = storage.storageAddress;
                  B.createCopyAddr(returnInst->getLoc(), resultAddr, resultArg,
                                   IsTake, IsInitialization);
               }
            } else {
               // Store the result into the result argument.
               B.createStore(returnInst->getLoc(), origDirectResultVal, resultArg,
                             StoreOwnershipQualifier::Unqualified);
            }
            ++newResultArgIdx;
         } else {
            // Record the direct result for populating the result tuple.
            newDirectResults.push_back(origDirectResultVal);
         }
      });
   assert(newDirectResults.size()
          == pass.loweredFnConv.getNumDirectPILResults());
   PILValue newReturnVal;
   if (newDirectResults.empty()) {
      PILType emptyTy = PILType::getPrimitiveObjectType(
         B.getModule().getAstContext().TheEmptyTupleType);
      newReturnVal = B.createTuple(returnInst->getLoc(), emptyTy, {});
   } else if (newDirectResults.size() == 1) {
      newReturnVal = newDirectResults[0];
   } else {
      newReturnVal =
         B.createTuple(returnInst->getLoc(),
                       pass.loweredFnConv.getPILResultType(), newDirectResults);
   }
   PILValue origFullResult = returnInst->getOperand();
   returnInst->setOperand(newReturnVal);
   if (auto *fullResultInst = origFullResult->getDefiningInstruction()) {
      if (!fullResultInst->hasUsesOfAnyResult())
         pass.markDead(fullResultInst);
   }
}

//===----------------------------------------------------------------------===//
// AddressOnlyUseRewriter - rewrite opaque value uses.
//===----------------------------------------------------------------------===//

namespace {
class AddressOnlyUseRewriter
   : PILInstructionVisitor<AddressOnlyUseRewriter> {
   friend PILVisitorBase<AddressOnlyUseRewriter>;
   friend PILInstructionVisitor<AddressOnlyUseRewriter>;

   AddressLoweringState &pass;

   PILBuilder B;
   AddressMaterialization addrMat;

   Operand *currOper;

public:
   explicit AddressOnlyUseRewriter(AddressLoweringState &pass)
      : pass(pass), B(*pass.F), addrMat(pass, B) {
      B.setPILConventions(
         PILModuleConventions::getLoweredAddressConventions(pass.F->getModule()));
   }

   void visitOperand(Operand *operand) {
      currOper = operand;
      visit(operand->getUser());
   }

protected:
   void markRewritten(PILValue oldValue, PILValue addr) {
      auto &storage = pass.valueStorageMap.getStorage(oldValue);
      storage.storageAddress = addr;
      storage.markRewritten();
   }

   void beforeVisit(PILInstruction *I) {
      LLVM_DEBUG(llvm::dbgs() << "  REWRITE USE "; I->dump());

      B.setInsertionPoint(I);
      B.setCurrentDebugScope(I->getDebugScope());
   }

   void visitPILInstruction(PILInstruction *I) {
      LLVM_DEBUG(I->dump());
      llvm_unreachable("Unimplemented?!");
   }

   void visitApplyInst(ApplyInst *applyInst) {
      ApplyRewriter(applyInst, pass).rewriteIndirectParameter(currOper);
   }

   void visitCopyValueInst(CopyValueInst *copyInst) {
      ValueStorage &storage = pass.valueStorageMap.getStorage(copyInst);
      // Fold a copy into a store.
      if (storage.isProjection()
          && isa<StoreInst>(storage.getComposedOperand()->getUser())) {
         return;
      }
      PILValue srcVal = copyInst->getOperand();
      PILValue srcAddr = pass.valueStorageMap.getStorage(srcVal).storageAddress;
      PILValue destAddr = addrMat.materializeAddress(copyInst);
      B.createCopyAddr(copyInst->getLoc(), srcAddr, destAddr, IsNotTake,
                       IsInitialization);
      markRewritten(copyInst, destAddr);
   }

   void visitDebugValueInst(DebugValueInst *debugInst) {
      PILValue srcVal = debugInst->getOperand();
      PILValue srcAddr = pass.valueStorageMap.getStorage(srcVal).storageAddress;
      B.createDebugValueAddr(debugInst->getLoc(), srcAddr,
                             *debugInst->getVarInfo());
      pass.markDead(debugInst);
   }

   void visitDestroyValueInst(DestroyValueInst *destroyInst) {
      PILValue srcVal = destroyInst->getOperand();
      PILValue srcAddr = pass.valueStorageMap.getStorage(srcVal).storageAddress;
      B.createDestroyAddr(destroyInst->getLoc(), srcAddr);
      pass.markDead(destroyInst);
   }

   // Handle EnumInst on the def side to handle both opaque and
   // loadable operands.
   void visitEnumInst(EnumInst *enumInst) {}

   // Handle InitExistentialValue on the def side to handle both opaque and
   // loadable operands.
   void
   visitInitExistentialValueInst(InitExistentialValueInst *initExistential) {}

   void visitReturnInst(ReturnInst *returnInst) {
      // Returns are rewritten for any function with indirect results after opaque
      // value rewriting.
   }

   void visitStoreInst(StoreInst *storeInst) {
      PILValue srcVal = storeInst->getSrc();
      assert(currOper->get() == srcVal);

      ValueStorage &storage = pass.valueStorageMap.getStorage(srcVal);
      PILValue srcAddr = storage.storageAddress;

      IsTake_t isTakeFlag = IsTake;
      assert(storeInst->getOwnershipQualifier()
             == StoreOwnershipQualifier::Unqualified);

      if (storage.isProjection()) {
         assert(!srcAddr);
         auto *copyInst = cast<CopyValueInst>(srcVal);
         ValueStorage &srcStorage =
            pass.valueStorageMap.getStorage(copyInst->getOperand());
         assert(!srcStorage.isProjection());
         srcAddr = srcStorage.storageAddress;
         isTakeFlag = IsNotTake;
      }
      // Bitwise copy the value. Two locations now share ownership. This is
      // modeled as a take-init.
      B.createCopyAddr(storeInst->getLoc(), srcAddr, storeInst->getDest(),
                       isTakeFlag, IsInitialization);
      pass.markDead(storeInst);
   }

   void visitTupleInst(TupleInst *tupleInst) {
      // Tuples are rewritten on the def-side, where both direct and indirect
      // elements are composed.
   }

   void visitTupleExtractInst(TupleExtractInst *extractInst) {
      // Apply results are rewritten when the result definition is visited.
      if (ApplySite::isa(currOper->get()))
         return;

      // TODO: generate tuple_element_addr.
      // generate copy_addr if we can't project.
      llvm_unreachable("unimplemented.");
   }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// AddressOnlyDefRewriter - rewrite opaque value definitions.
//===----------------------------------------------------------------------===//

namespace {
class AddressOnlyDefRewriter
   : PILInstructionVisitor<AddressOnlyDefRewriter> {
   friend PILVisitorBase<AddressOnlyDefRewriter>;
   friend PILInstructionVisitor<AddressOnlyDefRewriter>;

   AddressLoweringState &pass;

   PILBuilder B;
   AddressMaterialization addrMat;

   ValueStorage *storage = nullptr;

public:
   explicit AddressOnlyDefRewriter(AddressLoweringState &pass)
      : pass(pass), B(*pass.F), addrMat(pass, B) {
      B.setPILConventions(
         PILModuleConventions::getLoweredAddressConventions(pass.F->getModule()));
   }

   void visitInst(PILInstruction *inst) { visit(inst); }

protected:
   void beforeVisit(PILInstruction *I) {
      // This cast succeeds beecause only specific instructions get added to
      // the value storage map.
      storage = &pass.valueStorageMap.getStorage(cast<SingleValueInstruction>(I));

      LLVM_DEBUG(llvm::dbgs() << "REWRITE DEF "; I->dump());
      if (storage->storageAddress)
         LLVM_DEBUG(llvm::dbgs() << "  STORAGE "; storage->storageAddress->dump());

      B.setInsertionPoint(I);
      B.setCurrentDebugScope(I->getDebugScope());
   }

   void visitPILInstruction(PILInstruction *I) {
      LLVM_DEBUG(I->dump());
      llvm_unreachable("Unimplemented?!");
   }

   void visitApplyInst(ApplyInst *applyInst) {
      assert(isa<SingleValueInstruction>(applyInst) &&
             "beforeVisit assumes that ApplyInst is an SVI");
      assert(!storage->isRewritten());
      // Completely rewrite the apply instruction, handling any remaining
      // (loadable) indirect parameters, allocating memory for indirect
      // results, and generating a new apply instruction.
      ApplyRewriter rewriter(applyInst, pass);
      rewriter.rewriteParameters();
      rewriter.convertApplyWithIndirectResults();
   }

   void visitCopyValueInst(CopyValueInst *copyInst) {
      // A folded copy is not rewritten.
      assert(storage->isProjection() || storage->isRewritten());
   }

   void visitEnumInst(EnumInst *enumInst) {
      PILValue enumAddr;
      if (enumInst->hasOperand()) {
         addrMat.initializeOperandMem(&enumInst->getOperandRef());

         assert(storage->storageAddress);
         enumAddr = storage->storageAddress;
      } else
         enumAddr = addrMat.materializeAddress(enumInst);

      B.createInjectEnumAddr(enumInst->getLoc(), enumAddr,
                             enumInst->getElement());

      storage->markRewritten();
   }

   void visitInitExistentialValueInst(
      InitExistentialValueInst *initExistentialValue) {

      // Initialize memory for the operand which may be opaque or loadable.
      addrMat.initializeOperandMem(&initExistentialValue->getOperandRef());

      assert(storage->storageAddress);
      storage->markRewritten();
   }

   void visitLoadInst(LoadInst *loadInst) {
      // Bitwise copy the value. Two locations now share ownership. This is
      // modeled as a take-init.
      PILValue addr = pass.valueStorageMap.getStorage(loadInst).storageAddress;
      if (addr != loadInst->getOperand()) {
         B.createCopyAddr(loadInst->getLoc(), loadInst->getOperand(), addr, IsTake,
                          IsInitialization);
      }
      storage->markRewritten();
   }

   void visitTupleInst(TupleInst *tupleInst) {
      ValueStorage &storage = pass.valueStorageMap.getStorage(tupleInst);
      if (storage.isProjection()
          && isa<ReturnInst>(storage.getComposedOperand()->getUser())) {
         // For indirectly returned values, each element has its own storage.
         return;
      }
      // For each element, initialize the operand's memory. Some tuple elements
      // may be loadable types.
      PILValue tupleAddr = addrMat.materializeAddress(tupleInst);
      unsigned eltIdx = 0;
      for (Operand &operand : tupleInst->getAllOperands()) {
         PILType eltTy = operand.get()->getType();
         if (eltTy.isAddressOnly(*pass.F))
            addrMat.initializeOperandMem(&operand);
         else {
            auto *elementAddr = B.createTupleElementAddr(
               tupleInst->getLoc(), tupleAddr, eltIdx, eltTy.getAddressType());
            B.createStore(tupleInst->getLoc(), operand.get(), elementAddr,
                          StoreOwnershipQualifier::Unqualified);
         }
         ++eltIdx;
      }
   }

   void visitTupleExtractInst(TupleExtractInst *extractInst) {
      // If the source is an opaque tuple, as opposed to a call result, then the
      // extract is rewritten on the use-side.
      if (storage->isRewritten())
         return;

      // This must be an indirect result for an apply that has not yet been
      // rewritten. Rewrite the apply.
      PILValue srcVal = extractInst->getOperand();
      ApplyRewriter(cast<ApplyInst>(srcVal), pass)
         .convertApplyWithIndirectResults();

      assert(storage->storageAddress);
   }
};
} // end anonymous namespace

static void rewriteFunction(AddressLoweringState &pass) {
   AddressOnlyDefRewriter defVisitor(pass);
   AddressOnlyUseRewriter useVisitor(pass);

   for (auto &valueStorageI : pass.valueStorageMap) {
      PILValue valueDef = valueStorageI.first;

      // TODO: MultiValueInstruction: ApplyInst
      if (auto *defInst = dyn_cast<SingleValueInstruction>(valueDef))
         defVisitor.visitInst(defInst);

      SmallVector<Operand *, 8> uses(valueDef->getUses());
      for (Operand *oper : uses)
         useVisitor.visitOperand(oper);
   }

   // Rewrite any remaining (loadable) indirect parameters.
   for (ApplySite apply : pass.indirectApplies) {
      // Calls with indirect formal results have already been rewritten.
      if (apply.getSubstCalleeType()->hasIndirectFormalResults()) {
         bool isRewritten = false;
         visitCallResults(apply, [&](PILValue result) {
            if (result->getType().isAddressOnly(*pass.F)) {
               assert(pass.valueStorageMap.getStorage(result).isRewritten());
               isRewritten = true;
               return false;
            }
            return true;
         });
         if (!isRewritten) {
            ApplyRewriter rewriter(apply, pass);
            rewriter.rewriteParameters();
            rewriter.convertApplyWithIndirectResults();
            continue;
         }
      }
      ApplyRewriter(apply, pass).rewriteParameters();
   }
   if (pass.F->getLoweredFunctionType()->hasIndirectFormalResults())
      ReturnRewriter(pass).rewriteReturns();
}

//===----------------------------------------------------------------------===//
// AddressLowering: Top-Level Function Transform.
//===----------------------------------------------------------------------===//

namespace {
class AddressLowering : public PILModuleTransform {
   /// The entry point to this function transformation.
   void run() override;

   void runOnFunction(PILFunction *F);
};
} // end anonymous namespace

void AddressLowering::runOnFunction(PILFunction *F) {
   auto *DA = PM->getAnalysis<DominanceAnalysis>();

   AddressLoweringState pass(F, DA->get(F));

   // Rewrite function args and insert alloc_stack/dealloc_stack.
   OpaqueStorageAllocation allocator(pass);
   allocator.allocateOpaqueStorage();

   LLVM_DEBUG(llvm::dbgs() << "\nREWRITING: " << F->getName(); F->dump());

   // Rewrite instructions with address-only operands or results.
   rewriteFunction(pass);

   invalidateAnalysis(F, PILAnalysis::InvalidationKind::Instructions);

   // Instructions that were explicitly marked dead should already have no
   // users.
   //
   // Add the rest of the instructions to the dead list in post order.
   // FIXME: make sure we cleaned up address-only BB arguments.
   for (auto &valueStorageI : llvm::reverse(pass.valueStorageMap)) {
      // TODO: MultiValueInstruction: ApplyInst
      auto *deadInst = dyn_cast<SingleValueInstruction>(valueStorageI.first);
      if (!deadInst)
         continue;

      LLVM_DEBUG(llvm::dbgs() << "DEAD "; deadInst->dump());
#ifndef NDEBUG
      for (auto result : deadInst->getResults())
         for (Operand *operand : result->getUses())
            assert(pass.instsToDelete.count(operand->getUser()));
#endif
      pass.instsToDelete.insert(deadInst);
   }
   pass.valueStorageMap.clear();

   // Delete instructions in postorder
   recursivelyDeleteTriviallyDeadInstructions(pass.instsToDelete.takeVector(),
                                              true);
}

/// The entry point to this function transformation.
void AddressLowering::run() {
   if (getModule()->getAstContext().LangOpts.EnablePILOpaqueValues) {
      for (auto &F : *getModule())
         runOnFunction(&F);
   }
   // Set the PIL state before the PassManager has a chance to run
   // verification.
   getModule()->setStage(PILStage::Lowered);
}

PILTransform *polar::createAddressLowering() { return new AddressLowering(); }
