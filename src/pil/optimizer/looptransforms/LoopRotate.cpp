//===--- LoopRotate.cpp - Loop structure simplify -------------------------===//
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

#define DEBUG_TYPE "pil-looprotate"

#include "polarphp/pil/lang/Dominance.h"
#include "polarphp/pil/lang/PILArgument.h"
#include "polarphp/pil/lang/PILInstruction.h"
#include "polarphp/pil/optimizer/analysis/Analysis.h"
#include "polarphp/pil/optimizer/analysis/DominanceAnalysis.h"
#include "polarphp/pil/optimizer/analysis/LoopAnalysis.h"
#include "polarphp/pil/optimizer/passmgr/Transforms.h"
#include "polarphp/pil/optimizer/utils/BasicBlockOptUtils.h"
#include "polarphp/pil/optimizer/utils/CFGOptUtils.h"
#include "polarphp/pil/optimizer/utils/LoopUtils.h"
#include "polarphp/pil/optimizer/utils/PILSSAUpdater.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

using namespace polar;

static llvm::cl::opt<bool> ShouldRotate("pil-looprotate",
                                        llvm::cl::init(true));

/// Check whether all operands are loop invariant.
static bool
hasLoopInvariantOperands(PILInstruction *inst, PILLoop *loop,
                         llvm::DenseSet<PILInstruction *> &invariant) {
   auto operands = inst->getAllOperands();

   return llvm::all_of(operands, [=](Operand &operand) {
      ValueBase *def = operand.get();
      // Operand is outside the loop or marked invariant.
      if (auto *inst = def->getDefiningInstruction())
         return !loop->contains(inst->getParent()) || invariant.count(inst);
      if (auto *arg = dyn_cast<PILArgument>(def))
         return !loop->contains(arg->getParent());

      return false;
   });
}

/// We cannot duplicate blocks with AllocStack instructions (they need to be
/// FIFO). Other instructions can be moved to the preheader.
static bool
canDuplicateOrMoveToPreheader(PILLoop *loop, PILBasicBlock *preheader,
                              PILBasicBlock *bb,
                              SmallVectorImpl<PILInstruction *> &moves) {
   llvm::DenseSet<PILInstruction *> invariants;
   for (auto &instRef : *bb) {
      auto *inst = &instRef;
      if (auto *MI = dyn_cast<MethodInst>(inst)) {
         if (MI->getMember().isForeign)
            return false;
         if (!hasLoopInvariantOperands(inst, loop, invariants))
            continue;
         moves.push_back(inst);
         invariants.insert(inst);
      } else if (!inst->isTriviallyDuplicatable())
         return false;
         // It wouldn't make sense to rotate dealloc_stack without also rotating the
         // alloc_stack, which is covered by isTriviallyDuplicatable.
      else if (isa<DeallocStackInst>(inst))
         return false;
      else if (isa<FunctionRefInst>(inst)) {
         moves.push_back(inst);
         invariants.insert(inst);
      } else if (isa<DynamicFunctionRefInst>(inst)) {
         moves.push_back(inst);
         invariants.insert(inst);
      } else if (isa<PreviousDynamicFunctionRefInst>(inst)) {
         moves.push_back(inst);
         invariants.insert(inst);
      } else if (isa<IntegerLiteralInst>(inst)) {
         moves.push_back(inst);
         invariants.insert(inst);
      } else if (!inst->mayHaveSideEffects() && !inst->mayReadFromMemory()
                 && !isa<TermInst>(inst) && !isa<AllocationInst>(inst)
                 && /* not marked mayhavesideffects */
                 hasLoopInvariantOperands(inst, loop, invariants)) {
         moves.push_back(inst);
         invariants.insert(inst);
      }
   }

   return true;
}

static void mapOperands(PILInstruction *inst,
                        const llvm::DenseMap<ValueBase *, PILValue> &valueMap) {
   for (auto &operand : inst->getAllOperands()) {
      PILValue origVal = operand.get();
      ValueBase *origDef = origVal;
      auto found = valueMap.find(origDef);
      if (found != valueMap.end()) {
         PILValue mappedVal = found->second;
         operand.set(mappedVal);
      }
   }
}

static void updateSSAForUseOfValue(
   PILSSAUpdater &updater, SmallVectorImpl<PILPhiArgument *> &insertedPhis,
   const llvm::DenseMap<ValueBase *, PILValue> &valueMap,
   PILBasicBlock *Header, PILBasicBlock *EntryCheckBlock, PILValue Res) {
   // Find the mapped instruction.
   assert(valueMap.count(Res) && "Expected to find value in map!");
   PILValue MappedValue = valueMap.find(Res)->second;
   assert(MappedValue);
   assert(Res->getType() == MappedValue->getType() && "The types must match");

   insertedPhis.clear();
   updater.Initialize(Res->getType());
   updater.AddAvailableValue(Header, Res);
   updater.AddAvailableValue(EntryCheckBlock, MappedValue);

   // Because of the way that phi nodes are represented we have to collect all
   // uses before we update SSA. Modifying one phi node can invalidate another
   // unrelated phi nodes operands through the common branch instruction (that
   // has to be modified). This would invalidate a plain ValueUseIterator.
   // Instead we collect uses wrapping uses in branches specially so that we
   // can reconstruct the use even after the branch has been modified.
   SmallVector<UseWrapper, 8> storedUses;
   for (auto *use : Res->getUses())
      storedUses.push_back(UseWrapper(use));
   for (auto useWrapper : storedUses) {
      Operand *use = useWrapper;
      PILInstruction *user = use->getUser();
      assert(user && "Missing user");

      // Ignore uses in the same basic block.
      if (user->getParent() == Header)
         continue;

      assert(user->getParent() != EntryCheckBlock
             && "The entry check block should dominate the header");
      updater.RewriteUse(*use);
   }
   // Canonicalize inserted phis to avoid extra BB Args.
   for (PILPhiArgument *arg : insertedPhis) {
      if (PILValue inst = replaceBBArgWithCast(arg)) {
         arg->replaceAllUsesWith(inst);
         // DCE+SimplifyCFG runs as a post-pass cleanup.
         // DCE replaces dead arg values with undef.
         // SimplifyCFG deletes the dead BB arg.
      }
   }
}

static void
updateSSAForUseOfInst(PILSSAUpdater &updater,
                      SmallVectorImpl<PILPhiArgument *> &insertedPhis,
                      const llvm::DenseMap<ValueBase *, PILValue> &valueMap,
                      PILBasicBlock *header, PILBasicBlock *entryCheckBlock,
                      PILInstruction *inst) {
   for (auto result : inst->getResults())
      updateSSAForUseOfValue(updater, insertedPhis, valueMap, header,
                             entryCheckBlock, result);
}

/// Rewrite the code we just created in the preheader and update SSA form.
static void rewriteNewLoopEntryCheckBlock(
   PILBasicBlock *header, PILBasicBlock *entryCheckBlock,
   const llvm::DenseMap<ValueBase *, PILValue> &valueMap) {
   SmallVector<PILPhiArgument *, 4> insertedPhis;
   PILSSAUpdater updater(&insertedPhis);

   // Fix PHIs (incoming arguments).
   for (auto *arg : header->getArguments())
      updateSSAForUseOfValue(updater, insertedPhis, valueMap, header,
                             entryCheckBlock, arg);

   auto instIter = header->begin();

   // The terminator might change from under us.
   while (instIter != header->end()) {
      auto &inst = *instIter;
      updateSSAForUseOfInst(updater, insertedPhis, valueMap, header,
                            entryCheckBlock, &inst);
      instIter++;
   }
}

/// Update the dominator tree after rotating the loop.
/// The former preheader now dominates all of the former headers children. The
/// former latch now dominates the former header.
static void updateDomTree(DominanceInfo *domInfo, PILBasicBlock *preheader,
                          PILBasicBlock *latch, PILBasicBlock *header) {
   auto *headerN = domInfo->getNode(header);
   SmallVector<DominanceInfoNode *, 4> Children(headerN->begin(),
                                                headerN->end());
   auto *preheaderN = domInfo->getNode(preheader);
   for (auto *Child : Children)
      domInfo->changeImmediateDominator(Child, preheaderN);

   if (header != latch)
      domInfo->changeImmediateDominator(headerN, domInfo->getNode(latch));
}

static bool rotateLoopAtMostUpToLatch(PILLoop *loop, DominanceInfo *domInfo,
                                      PILLoopInfo *loopInfo,
                                      bool ShouldVerify) {
   auto *latch = loop->getLoopLatch();
   if (!latch) {
      LLVM_DEBUG(llvm::dbgs()
                    << *loop << " does not have a single latch block\n");
      return false;
   }

   bool didRotate =
      rotateLoop(loop, domInfo, loopInfo, false /* rotateSingleBlockLoops */,
                 latch, ShouldVerify);

   // Keep rotating at most until we hit the original latch.
   if (didRotate)
      while (rotateLoop(loop, domInfo, loopInfo, false, latch, ShouldVerify)) {
      }

   return didRotate;
}

/// Check whether this a single basic block loop - ignoring split back edges.
static bool isSingleBlockLoop(PILLoop *L) {
   auto Blocks = L->getBlocks();
   auto NumBlocks = Blocks.size();
   if (NumBlocks > 2)
      return false;

   if (NumBlocks == 1)
      return true;

   auto *header = L->getHeader();
   auto *BackEdge = Blocks[1];
   if (BackEdge == header)
      BackEdge = Blocks[0];

   if (!BackEdge->getSingleSuccessorBlock())
      return false;

   assert(BackEdge->getSingleSuccessorBlock() == header
          && "Loop not well formed");

   // Check whether the back-edge block is just a split-edge.
   return ++BackEdge->begin() == BackEdge->end();
}

/// We rotated a loop if it has the following properties.
///
/// * It has an exiting header with a conditional branch.
/// * It has a preheader (the function will try to create one for critical edges
///   from cond_br).
///
/// We will rotate at most up to the basic block passed as an argument.
/// We will not rotate a loop where the header is equal to the latch except is
/// rotateSingleBlockLoops is true.
///
/// Note: The code relies on the 'UpTo' basic block to stay within the rotate
/// loop for termination.
bool polar::rotateLoop(PILLoop *loop, DominanceInfo *domInfo,
                       PILLoopInfo *loopInfo, bool rotateSingleBlockLoops,
                       PILBasicBlock *upToBB, bool shouldVerify) {
   assert(loop != nullptr && domInfo != nullptr && loopInfo != nullptr
          && "Missing loop information");

   auto *header = loop->getHeader();
   if (!header)
      return false;

   // We need a preheader - this is also a canonicalization for follow-up
   // passes.
   auto *preheader = loop->getLoopPreheader();
   if (!preheader) {
      LLVM_DEBUG(llvm::dbgs() << *loop << " no preheader\n");
      LLVM_DEBUG(loop->getHeader()->getParent()->dump());
      return false;
   }

   if (!rotateSingleBlockLoops && (header == upToBB || isSingleBlockLoop(loop)))
      return false;

   assert(rotateSingleBlockLoops || loop->getBlocks().size() != 1);

   // Need a conditional branch that guards the entry into the loop.
   auto *loopEntryBranch = dyn_cast<CondBranchInst>(header->getTerminator());
   if (!loopEntryBranch)
      return false;

   // The header needs to exit the loop.
   if (!loop->isLoopExiting(header)) {
      LLVM_DEBUG(llvm::dbgs() << *loop << " not an exiting header\n");
      LLVM_DEBUG(loop->getHeader()->getParent()->dump());
      return false;
   }

   // We need a single backedge and the latch must not exit the loop if it is
   // also the header.
   auto *latch = loop->getLoopLatch();
   if (!latch) {
      LLVM_DEBUG(llvm::dbgs() << *loop << " no single latch\n");
      return false;
   }

   // Make sure we can duplicate the header.
   SmallVector<PILInstruction *, 8> moveToPreheader;
   if (!canDuplicateOrMoveToPreheader(loop, preheader, header,
                                      moveToPreheader)) {
      LLVM_DEBUG(llvm::dbgs()
                    << *loop << " instructions in header preventing rotating\n");
      return false;
   }

   auto *newHeader = loopEntryBranch->getTrueBB();
   auto *exit = loopEntryBranch->getFalseBB();
   if (loop->contains(exit))
      std::swap(newHeader, exit);
   assert(loop->contains(newHeader) && !loop->contains(exit)
          && "Could not find loop header and exit block");

   // We don't want to rotate such that we merge two headers of separate loops
   // into one. This can be turned into an assert again once we have guaranteed
   // preheader insertions.
   if (!newHeader->getSinglePredecessorBlock() && header != latch)
      return false;

   // Now that we know we can perform the rotation - move the instructions that
   // need moving.
   for (auto *inst : moveToPreheader)
      inst->moveBefore(preheader->getTerminator());

   LLVM_DEBUG(llvm::dbgs() << " Rotating " << *loop);

   // Map the values for the duplicated header block. We are duplicating the
   // header instructions into the end of the preheader.
   llvm::DenseMap<ValueBase *, PILValue> valueMap;

   // The original 'phi' argument values are just the values coming from the
   // preheader edge.
   ArrayRef<PILArgument *> phis = header->getArguments();
   OperandValueArrayRef preheaderArgs =
      cast<BranchInst>(preheader->getTerminator())->getArgs();
   assert(phis.size() == preheaderArgs.size()
          && "Basic block arguments and incoming edge mismatch");

   // Here we also store the value index to use into the value map (versus
   // non-argument values where the operand use decides which value index to
   // use).
   for (unsigned Idx = 0, E = phis.size(); Idx != E; ++Idx)
      valueMap[phis[Idx]] = preheaderArgs[Idx];

   // The other instructions are just cloned to the preheader.
   TermInst *preheaderBranch = preheader->getTerminator();
   for (auto &inst : *header) {
      if (PILInstruction *cloned = inst.clone(preheaderBranch)) {
         mapOperands(cloned, valueMap);

         // The actual operand will sort out which result idx to use.
         auto instResults = inst.getResults();
         auto clonedResults = cloned->getResults();
         assert(instResults.size() == clonedResults.size());
         for (auto i : indices(instResults))
            valueMap[instResults[i]] = clonedResults[i];
      }
   }

   preheaderBranch->dropAllReferences();
   preheaderBranch->eraseFromParent();

   // If there were any uses of instructions in the duplicated loop entry check
   // block rewrite them using the ssa updater.
   rewriteNewLoopEntryCheckBlock(header, preheader, valueMap);

   loop->moveToHeader(newHeader);

   // Now the original preheader dominates all of headers children and the
   // original latch dominates the header.
   updateDomTree(domInfo, preheader, latch, header);

   assert(domInfo->getNode(newHeader)->getIDom() == domInfo->getNode(preheader));
   assert(!domInfo->dominates(header, exit)
          || domInfo->getNode(exit)->getIDom() == domInfo->getNode(preheader));
   assert(domInfo->getNode(header)->getIDom() == domInfo->getNode(latch)
          || ((header == latch)
              && domInfo->getNode(header)->getIDom()
                 == domInfo->getNode(preheader)));

   // Beautify the IR. Move the old header to after the old latch as it is now
   // the latch.
   header->moveAfter(latch);

   // Merge the old latch with the old header if possible.
   mergeBasicBlockWithSuccessor(latch, domInfo, loopInfo);

   // Create a new preheader.
   splitIfCriticalEdge(preheader, newHeader, domInfo, loopInfo);

   if (shouldVerify) {
      domInfo->verify();
      loopInfo->verify();
      latch->getParent()->verify();
   }

   LLVM_DEBUG(llvm::dbgs() << "  to " << *loop);
   LLVM_DEBUG(loop->getHeader()->getParent()->dump());
   return true;
}

namespace {

class LoopRotation : public PILFunctionTransform {

   void run() override {
      PILLoopAnalysis *loopAnalysis = PM->getAnalysis<PILLoopAnalysis>();
      assert(loopAnalysis);
      DominanceAnalysis *domAnalysis = PM->getAnalysis<DominanceAnalysis>();
      assert(domAnalysis);

      PILFunction *f = getFunction();
      assert(f);
      // FIXME: Add ownership support.
      if (f->hasOwnership())
         return;

      PILLoopInfo *loopInfo = loopAnalysis->get(f);
      assert(loopInfo);
      DominanceInfo *domInfo = domAnalysis->get(f);

      if (loopInfo->empty()) {
         LLVM_DEBUG(llvm::dbgs() << "No loops in " << f->getName() << "\n");
         return;
      }
      if (!ShouldRotate) {
         LLVM_DEBUG(llvm::dbgs()
                       << "Skipping loop rotation in " << f->getName() << "\n");
         return;
      }
      LLVM_DEBUG(llvm::dbgs() << "Rotating loops in " << f->getName() << "\n");
      bool shouldVerify = getOptions().VerifyAll;

      bool changed = false;
      for (auto *LoopIt : *loopInfo) {
         // Rotate loops recursively bottom-up in the loop tree.
         SmallVector<PILLoop *, 8> worklist;
         worklist.push_back(LoopIt);
         for (unsigned i = 0; i < worklist.size(); ++i) {
            auto *L = worklist[i];
            for (auto *SubLoop : *L)
               worklist.push_back(SubLoop);
         }

         while (!worklist.empty()) {
            PILLoop *loop = worklist.pop_back_val();
            changed |= canonicalizeLoop(loop, domInfo, loopInfo);
            changed |=
               rotateLoopAtMostUpToLatch(loop, domInfo, loopInfo, shouldVerify);
         }
      }

      if (changed) {
         // We preserve loop info and the dominator tree.
         domAnalysis->lockInvalidation();
         loopAnalysis->lockInvalidation();
         PM->invalidateAnalysis(f, PILAnalysis::InvalidationKind::FunctionBody);
         domAnalysis->unlockInvalidation();
         loopAnalysis->unlockInvalidation();
      }
   }
};

} // end anonymous namespace

PILTransform *polar::createLoopRotate() {
   return new LoopRotation();
}
