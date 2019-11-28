//===--- ClangModuleLoader.h - Clang Module Loader Interface ----*- C++ -*-===//
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
// Created by polarboy on 2019/11/27.

#ifndef POLARPHP_AST_CLANG_MODULE_LOADER_H
#define POLARPHP_AST_CLANG_MODULE_LOADER_H

#include "polarphp/ast/ModuleLoader.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"

namespace polar::ast {

class DeclContext;

using llvm::StringRef;

class ClangModuleLoader : public ModuleLoader
{
private:
   virtual void anchor();
protected:
   using ModuleLoader::ModuleLoader;
public:
   virtual clang::ASTContext &getClangASTContext() const = 0;
   virtual clang::Preprocessor &getClangPreprocessor() const = 0;
   virtual clang::Sema &getClangSema() const = 0;
   virtual const clang::CompilerInstance &getClangInstance() const = 0;
   virtual void printStatistics() const = 0;

   /// Returns the module that contains imports and declarations from all loaded
   /// Objective-C header files.
   virtual ModuleDecl *getImportedHeaderModule() const = 0;

   /// Adds a new search path to the Clang CompilerInstance, as if specified with
   /// -I or -F.
   ///
   /// \returns true if there was an error adding the search path.
   virtual bool addSearchPath(StringRef newSearchPath, bool isFramework,
                              bool isSystem) = 0;

   /// Determine whether \c overlayDC is within an overlay module for the
   /// imported context enclosing \c importedDC.
   ///
   /// This routine is used for various hacks that are only permitted within
   /// overlays of imported modules, e.g., Objective-C bridging conformances.
   virtual bool isInOverlayModuleForImportedModule(
         const DeclContext *overlayDC,
         const DeclContext *importedDC) = 0;

};

} // polar::ast

#endif // POLARPHP_AST_CLANG_MODULE_LOADER_H
