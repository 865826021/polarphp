// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/05/08.

#ifndef POLARPHP_SYNTAX_SYNTAX_NODE_COMMON_NODES_H
#define POLARPHP_SYNTAX_SYNTAX_NODE_COMMON_NODES_H

#include "polarphp/syntax/Syntax.h"
#include "polarphp/syntax/SyntaxCollection.h"
#include "polarphp/syntax/TokenSyntax.h"
#include "polarphp/syntax/UnknownSyntax.h"

namespace polar::syntax {

/// common syntex nodes
/// normal node
class DeclSyntax;
class ExprSyntax;
class StmtSyntax;
class UnknownDeclSyntax;
class UnknownExprSyntax;
class UnknownStmtSyntax;
class CodeBlockItemSyntax;
class CodeBlockSyntax;

/// collection node
using CodeBlockItemListSyntax = SyntaxCollection<SyntaxKind::CodeBlockItemList, CodeBlockItemSyntax>;

/// attribute syntax nodes
/// collection node
using TokenListSyntax = SyntaxCollection<SyntaxKind::TokenList, TokenSyntax>;
using NonEmptyTokenListSyntax = SyntaxCollection<SyntaxKind::NonEmptyTokenList, TokenSyntax>;

class DeclSyntax : public Syntax
{
public:
   DeclSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : Syntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return is_decl_kind(kind);
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

class StmtSyntax : public Syntax
{
public:
   StmtSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : Syntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return is_stmt_kind(kind);
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

class ExprSyntax : public Syntax
{
public:
   ExprSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : Syntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return is_expr_kind(kind);
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

class UnknownDeclSyntax final : public DeclSyntax
{
public:
   UnknownDeclSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : DeclSyntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return SyntaxKind::UnknownDecl == kind;
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

class UnknownExprSyntax final : public ExprSyntax
{
public:
   UnknownExprSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : ExprSyntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return SyntaxKind::UnknownExpr == kind;
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

class UnknownStmtSyntax final : public StmtSyntax
{
public:
   UnknownStmtSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : StmtSyntax(root, data)
   {}

   static bool kindOf(SyntaxKind kind)
   {
      return SyntaxKind::UnknownStmt == kind;
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
};

/// A CodeBlockItem is any Syntax node that appears on its own line inside
/// a CodeBlock.
class CodeBlockItemSyntax final : public Syntax
{
public:
   enum Cursor : uint32_t
   {
      /// type: Syntax
      /// ------------
      /// node choices
      /// name: Decl kind: Decl
      /// name: Stmt kind: Stmt
      /// name: TokenList kind: TokenList
      /// name: NonEmptyTokenList kind: NonEmptyTokenList
      Item,
      /// type: TokenSyntax
      Semicolon,
      /// type: Syntax
      ErrorTokens
   };

public:
   CodeBlockItemSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : Syntax(root, data)
   {
      this->validate();
   }

   /// The underlying node inside the code block.
   Syntax getItem();
   TokenSyntax getSemicolon();
   std::optional<Syntax> getErrorTokens();

   /// Returns a copy of the receiver with its `${child.name}` replaced.
   /// - param newChild: The new `Item` to replace the node's
   ///                   current `Item`, if present.
   CodeBlockItemSyntax withItem(std::optional<Syntax> item);

   /// the trailing semicolon at the end of the item.
   CodeBlockItemSyntax withSemicolon(std::optional<TokenSyntax> semicolon);
   CodeBlockItemSyntax withErrorTokens(std::optional<Syntax> errorTokens);

   static bool kindOf(SyntaxKind kind)
   {
      return SyntaxKind::CodeBlockItem == kind;
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }
private:
   friend class CodeBlockItemSyntaxBuilder;
   void validate();
};

class CodeBlockSyntax final : public Syntax
{
public:
   enum Cursor : uint32_t
   {
      /// type: TokenSyntax
      LeftBrace,
      /// type: CodeBlockItemListSyntax
      Statements,
      /// type: TokenSyntax
      RightBrace
   };

public:
   CodeBlockSyntax(const RefCountPtr<SyntaxData> root, const SyntaxData *data)
      : Syntax(root, data)
   {
      this->validate();
   }

   TokenSyntax getLeftBrace();
   TokenSyntax getRightBrace();
   CodeBlockItemListSyntax getStatements();

   /// Adds the provided `CodeBlockItem` to the node's `Statements`
    /// collection.
    /// - param element: The new `CodeBlockItem` to add to the node's
    ///                  `Statements` collection.
    /// - returns: A copy of the receiver with the provided `CodeBlockItem
    ///            appended to its `Statements` collection.
   CodeBlockSyntax addCodeBlockItem(CodeBlockItemSyntax codeBlockItem);

   CodeBlockSyntax withLeftBrace(std::optional<TokenSyntax> leftBrace);
   CodeBlockSyntax withRightBrace(std::optional<TokenSyntax> rightBrace);
   CodeBlockSyntax withStatements(std::optional<CodeBlockItemListSyntax> statements);

   static bool kindOf(SyntaxKind kind)
   {
      return SyntaxKind::CodeBlock == kind;
   }

   static bool classOf(const Syntax *syntax)
   {
      return kindOf(syntax->getKind());
   }

private:
   friend class CodeBlockSyntaxBuilder;
   void validate();
};

} // polar::syntax

#endif // POLARPHP_SYNTAX_SYNTAX_NODE_COMMON_NODES_H
