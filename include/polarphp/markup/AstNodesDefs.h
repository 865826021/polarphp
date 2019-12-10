//===--- ASTNodes.def - Swift Markup ----------------------------*- C++ -*-===//
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
// This file defines Swift Markup AST nodes.
//
//===----------------------------------------------------------------------===//

/// MARKUP_AST_NODE(Id, Parent)
///
/// ABSTRACT_REST_AST_NODE(Id, Parent)
///
/// MARKUP_AST_NODE_RANGE(Id, FirstId, LastId)

MARKUP_AST_NODE(Document, MarkupASTNode)
MARKUP_AST_NODE(BlockQuote, MarkupASTNode)
MARKUP_AST_NODE(List, MarkupASTNode)
MARKUP_AST_NODE(Item, MarkupASTNode)
MARKUP_AST_NODE(CodeBlock, MarkupASTNode)
MARKUP_AST_NODE(HTML, MarkupASTNode)
MARKUP_AST_NODE(Paragraph, MarkupASTNode)
MARKUP_AST_NODE(Header, MarkupASTNode)
MARKUP_AST_NODE(HRule, MarkupASTNode)
MARKUP_AST_NODE_RANGE(Block, Document, HRule)
ABSTRACT_MARKUP_AST_NODE(InlineContent, MarkupASTNode)
  MARKUP_AST_NODE(Text, InlineContent)
  MARKUP_AST_NODE(SoftBreak, InlineContent)
  MARKUP_AST_NODE(LineBreak, InlineContent)
  MARKUP_AST_NODE(Code, InlineContent)
  MARKUP_AST_NODE(InlineHTML, InlineContent)
  MARKUP_AST_NODE(Emphasis, InlineContent)
  MARKUP_AST_NODE(Strong, InlineContent)
  MARKUP_AST_NODE(Link, InlineContent)
  MARKUP_AST_NODE(Image, InlineContent)
MARKUP_AST_NODE_RANGE(Inline, Text, Image)

/// Private Markdown Extensions - these should not be implemented in the
/// underlying cmark parser.

MARKUP_AST_NODE(PrivateExtension, MarkupASTNode)
  MARKUP_AST_NODE(ParamField, PrivateExtension)

  // Simple fields
  // There must be a corresponding definition in
  // include/swift/Markup/SimpleFields.def.
  MARKUP_AST_NODE(AttentionField, PrivateExtension)
  MARKUP_AST_NODE(AuthorField, PrivateExtension)
  MARKUP_AST_NODE(AuthorsField, PrivateExtension)
  MARKUP_AST_NODE(BugField, PrivateExtension)
  MARKUP_AST_NODE(Complexity, PrivateExtension)
  MARKUP_AST_NODE(CopyrightField, PrivateExtension)
  MARKUP_AST_NODE(DateField, PrivateExtension)
  MARKUP_AST_NODE(ExperimentField, PrivateExtension)
  MARKUP_AST_NODE(ImportantField, PrivateExtension)
  MARKUP_AST_NODE(InvariantField, PrivateExtension)
  MARKUP_AST_NODE(LocalizationKeyField, PrivateExtension)
  MARKUP_AST_NODE(MutatingvariantField, PrivateExtension)
  MARKUP_AST_NODE(NonmutatingvariantField, PrivateExtension)
  MARKUP_AST_NODE(NoteField, PrivateExtension)
  MARKUP_AST_NODE(PostconditionField, PrivateExtension)
  MARKUP_AST_NODE(PreconditionField, PrivateExtension)
  MARKUP_AST_NODE(RemarkField, PrivateExtension)
  MARKUP_AST_NODE(RemarksField, PrivateExtension)
  MARKUP_AST_NODE(ReturnsField, PrivateExtension)
  MARKUP_AST_NODE(RequiresField, PrivateExtension)
  MARKUP_AST_NODE(SeeField, PrivateExtension)
  MARKUP_AST_NODE(SinceField, PrivateExtension)
  MARKUP_AST_NODE(TagField, PrivateExtension)
  MARKUP_AST_NODE(TODOField, PrivateExtension)
  MARKUP_AST_NODE(ThrowsField, PrivateExtension)
  MARKUP_AST_NODE(VersionField, PrivateExtension)
  MARKUP_AST_NODE(WarningField, PrivateExtension)
  MARKUP_AST_NODE(KeywordField, PrivateExtension)
  MARKUP_AST_NODE(RecommendedField, PrivateExtension)
  MARKUP_AST_NODE(RecommendedoverField, PrivateExtension)

MARKUP_AST_NODE_RANGE(Private, ParamField, WarningField)
#undef MARKUP_AST_NODE
#undef ABSTRACT_MARKUP_AST_NODE
#undef MARKUP_AST_NODE_RANGE
