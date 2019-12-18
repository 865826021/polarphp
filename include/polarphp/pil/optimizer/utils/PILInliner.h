//===--- PILInliner.h - Inlines PIL functions -------------------*- C++ -*-===//
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
// This file defines the PILInliner class, used for inlining PIL functions into
// function application sites
//
//===----------------------------------------------------------------------===//

#ifndef POLARPHP_PIL_OPTIMIZER_UTILS_PILINLINER_H
#define POLARPHP_PIL_OPTIMIZER_UTILS_PILINLINER_H

#include "polarphp/ast/SubstitutionMap.h"
#include "polarphp/pil/lang/ApplySite.h"
#include "polarphp/pil/lang/PILInstruction.h"
#include "polarphp/pil/lang/PILOpenedArchetypesTracker.h"
#include "llvm/ADT/DenseMap.h"
#include <functional>

namespace polar {

class PILOptFunctionBuilder;

// For now Free is 0 and Expensive is 1. This can be changed in the future by
// adding more categories.
enum class InlineCost : unsigned {
  Free = 0,
  Expensive = 1
};

/// Return the 'cost' of one instruction. Instructions that are expected to
/// disappear at the LLVM IR level are assigned a cost of 'Free'.
InlineCost instructionInlineCost(PILInstruction &I);

class PILInliner {
public:
  enum class InlineKind { MandatoryInline, PerformanceInline };

  using DeletionFuncTy = std::function<void(PILInstruction *)>;

private:
  PILOptFunctionBuilder &FuncBuilder;
  InlineKind IKind;
  SubstitutionMap ApplySubs;
  PILOpenedArchetypesTracker &OpenedArchetypesTracker;

  DeletionFuncTy DeletionCallback;

public:
  PILInliner(PILOptFunctionBuilder &FuncBuilder, InlineKind IKind,
             SubstitutionMap ApplySubs,
             PILOpenedArchetypesTracker &OpenedArchetypesTracker)
      : FuncBuilder(FuncBuilder), IKind(IKind), ApplySubs(ApplySubs),
        OpenedArchetypesTracker(OpenedArchetypesTracker) {}

  /// Returns true if we are able to inline \arg AI.
  ///
  /// *NOTE* This must be checked before attempting to inline \arg AI. If one
  /// attempts to inline \arg AI and this returns false, an assert will fire.
  static bool canInlineApplySite(FullApplySite apply);

  /// Returns true if inlining \arg apply can result in improperly nested stack
  /// allocations.
  ///
  /// In this case stack nesting must be corrected after inlining with the
  /// StackNesting utility.
  static bool needsUpdateStackNesting(FullApplySite apply) {
    // Inlining of coroutines can result in improperly nested stack
    // allocations.
    return isa<BeginApplyInst>(apply);
  }

  /// Allow the client to track instructions before they are deleted. The
  /// registered callback is called from
  /// recursivelyDeleteTriviallyDeadInstructions.
  ///
  /// (This is safer than the PILModule deletion callback because the
  /// instruction is still in a valid form and its operands can be inspected.)
  void setDeletionCallback(DeletionFuncTy f) { DeletionCallback = f; }

  /// Inline a callee function at the given apply site with the given
  /// arguments. Delete the apply and any dead arguments. Return a valid
  /// iterator to the first inlined instruction (or first instruction after the
  /// call for an empty function).
  ///
  /// This only performs one step of inlining: it does not recursively
  /// inline functions called by the callee.
  ///
  /// This may split basic blocks and delete instructions anywhere.
  ///
  /// All inlined instructions must be either inside the original call block or
  /// inside new basic blocks laid out after the original call block.
  ///
  /// Any instructions in the original call block after the inlined call must be
  /// in a new basic block laid out after all inlined blocks.
  ///
  /// The above guarantees ensure that inlining is liner in the number of
  /// instructions and that inlined instructions are revisited exactly once.
  ///
  /// *NOTE*: This attempts to perform inlining unconditionally and thus asserts
  /// if inlining will fail. All users /must/ check that a function is allowed
  /// to be inlined using PILInliner::canInlineApplySite before calling this
  /// function.
  ///
  /// *NOTE*: Inlining can result in improperly nested stack allocations, which
  /// must be corrected after inlining. See needsUpdateStackNesting().
  ///
  /// Returns an iterator to the first inlined instruction (or the end of the
  /// caller block for empty functions) and the last block in function order
  /// containing inlined instructions (the original caller block for
  /// single-block functions).
  std::pair<PILBasicBlock::iterator, PILBasicBlock *>
  inlineFunction(PILFunction *calleeFunction, FullApplySite apply,
                 ArrayRef<PILValue> appliedArgs);

  /// Inline the function called by the given full apply site. This creates
  /// an instance of PILInliner by constructing a substitution map and
  /// OpenedArchetypesTracker from the given apply site, and invokes
  /// `inlineFunction` method on the PILInliner instance to inline the callee.
  /// This requires the full apply site to be a direct call i.e., the apply
  /// instruction must have a function ref.
  ///
  /// *NOTE*:This attempts to perform inlining unconditionally and thus asserts
  /// if inlining will fail. All users /must/ check that a function is allowed
  /// to be inlined using PILInliner::canInlineApplySite before calling this
  /// function.
  ///
  /// *NOTE*: Inlining can result in improperly nested stack allocations, which
  /// must be corrected after inlining. See needsUpdateStackNesting().
  ///
  /// Returns an iterator to the first inlined instruction (or the end of the
  /// caller block for empty functions) and the last block in function order
  /// containing inlined instructions (the original caller block for
  /// single-block functions).
  static std::pair<PILBasicBlock::iterator, PILBasicBlock *>
  inlineFullApply(FullApplySite apply, PILInliner::InlineKind inlineKind,
                  PILOptFunctionBuilder &funcBuilder);
};

} // end namespace polar

#endif
