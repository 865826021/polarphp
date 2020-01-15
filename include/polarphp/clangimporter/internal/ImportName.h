//===--- ImportName.h - Imported Swift names for Clang decls ----*- C++ -*-===//
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
// This file provides class definitions for naming-related concerns in the
// ClangImporter.
//
//===----------------------------------------------------------------------===//
#ifndef POLARPHP_CLANG_INTERNAL_IMPORT_NAME_H
#define POLARPHP_CLANG_INTERNAL_IMPORT_NAME_H

#include "polarphp/clangimporter/internal/ImportEnumInfo.h"
#include "polarphp/clangimporter/internal/TypePHPLookupTable.h"
#include "polarphp/basic/StringExtras.h"
#include "polarphp/basic/Version.h"
#include "polarphp/ast/AstContext.h"
#include "polarphp/ast/Decl.h"
#include "polarphp/ast/ForeignErrorConvention.h"
#include "clang/Sema/Sema.h"

namespace polar {
namespace importer {
struct PlatformAvailability;

/// The kind of accessor that an entity will be imported as.
enum class ImportedAccessorKind : unsigned {
   None = 0,
   PropertyGetter,
   PropertySetter,
   SubscriptGetter,
   SubscriptSetter,
};
enum { NumImportedAccessorKindBits = 3 };

/// The name version
class ImportNameVersion : public RelationalOperationsBase<ImportNameVersion> {
   unsigned rawValue;
   friend llvm::DenseMapInfo<ImportNameVersion>;

   enum AsConstExpr_t { AsConstExpr };

   constexpr ImportNameVersion() : rawValue(0) {}
   constexpr ImportNameVersion(unsigned version, AsConstExpr_t)
      : rawValue(version) {}
   explicit ImportNameVersion(unsigned version) : rawValue(version) {
      assert(version >= 2 && "only Swift 2 and later are supported");
   }
public:
   /// Map a language version into an import name version.
   static ImportNameVersion fromOptions(const LangOptions &langOpts) {
      // We encode the 'rawValue' as just major version numbers with the
      // exception of '4.2', which is a special minor version that can impact
      // importing of names.  We treat that with a rawValue of 5, and treat
      // all major values of 5 or higher as being rawValue = majorversion + 1.
      const auto &version = langOpts.EffectiveLanguageVersion;
      // If the effective version is 4.x, where x >= 2, the import version
      // is 4.2.
      if (version.size() > 1 && version[0] == 4 && version[1] >= 2) {
         return ImportNameVersion::swift4_2();
      }
      unsigned major = version[0];
      return ImportNameVersion(major >= 5 ? major + 1 : major);
   }

   unsigned majorVersionNumber() const {
      assert(*this != ImportNameVersion::raw());
      if (*this == ImportNameVersion::swift4_2())
         return 4;
      return rawValue < 5 ? rawValue : rawValue - 1;
   }

   unsigned minorVersionNumber() const {
      assert(*this != ImportNameVersion::raw());
      if (*this == ImportNameVersion::swift4_2())
         return 2;
      return 0;
   }

   llvm::VersionTuple asClangVersionTuple() const {
      assert(*this != ImportNameVersion::raw());
      return llvm::VersionTuple(majorVersionNumber(), minorVersionNumber());
   }

   bool operator==(ImportNameVersion other) const {
      return rawValue == other.rawValue;
   }
   bool operator<(ImportNameVersion other) const {
      return rawValue < other.rawValue;
   }

   /// Calls \p action for each name version other than this one, first going
   /// backwards until ImportNameVersion::raw(), and then going forwards to
   /// ImportNameVersion::maxVersion().
   ///
   /// This is the most useful order for importing compatibility stubs.
   void forEachOtherImportNameVersion(
      llvm::function_ref<void(ImportNameVersion)> action) const {
      assert(*this >= ImportNameVersion::swift2());

      ImportNameVersion nameVersion = *this;
      while (nameVersion > ImportNameVersion::swift2()) {
         --nameVersion.rawValue;
         action(nameVersion);
      }

      action(ImportNameVersion::raw());

      nameVersion = *this;
      while (nameVersion < ImportNameVersion::maxVersion()) {
         ++nameVersion.rawValue;
         action(nameVersion);
      }
   }

   /// Names as they appear in C/ObjC.
   static constexpr inline ImportNameVersion raw() {
      return ImportNameVersion{};
   }

   /// Names as they appeared in Swift 2 family.
   static constexpr inline ImportNameVersion swift2() {
      return ImportNameVersion{2, AsConstExpr};
   }

   /// Names as they appeared in Swift 4.2 family.
   static constexpr inline ImportNameVersion swift4_2() {
      return ImportNameVersion{5, AsConstExpr};
   }

   /// The latest supported version.
   ///
   /// FIXME: All other version information is in Version.h. Can this go there
   /// instead?
   static constexpr inline ImportNameVersion maxVersion() {
      return ImportNameVersion{6, AsConstExpr};
   }

   /// The version which should be used for importing types, which need to have
   /// one canonical definition.
   ///
   /// FIXME: Is this supposed to be the /newest/ version, or a canonical
   /// version that lasts forever as part of the ABI?
   static constexpr inline ImportNameVersion forTypes() {
      return ImportNameVersion::maxVersion();
   }
};

/// Describes a name that was imported from Clang.
class ImportedName {
   friend class NameImporter;

   /// The imported name.
   DeclName declName;

   /// The context into which this declaration will be imported.
   ///
   /// When the context into which the declaration will be imported
   /// matches a Clang declaration context (the common case), the
   /// result will be expressed as a declaration context. Otherwise,
   /// if the Clang type is not itself a declaration context (for
   /// example, a typedef that comes into Swift as a strong type),
   /// the type declaration will be provided.
   EffectiveClangContext effectiveContext;

   struct Info {
      /// For names that map Objective-C error handling conventions into
      /// throwing Swift methods, describes how the mapping is performed.
      ForeignErrorConvention::Info errorInfo;

      /// For a declaration name that makes the declaration into an
      /// instance member, the index of the "Self" parameter.
      unsigned selfIndex;

      /// For an initializer, the kind of initializer to import.
      CtorInitializerKind initKind;

      /// What kind of accessor this name refers to, if any.
      ImportedAccessorKind accessorKind : NumImportedAccessorKindBits;

      /// Whether this name was explicitly specified via a Clang
      /// swift_name attribute.
      unsigned hasCustomName : 1;

      /// Whether this was one of a special class of Objective-C
      /// initializers for which we drop the variadic argument rather
      /// than refuse to import the initializer.
      unsigned droppedVariadic : 1;

      /// Whether this is a global being imported as a member
      unsigned importAsMember : 1;

      unsigned hasSelfIndex : 1;

      unsigned hasErrorInfo : 1;

      Info()
         : errorInfo(), selfIndex(), initKind(CtorInitializerKind::Designated),
           accessorKind(ImportedAccessorKind::None), hasCustomName(false),
           droppedVariadic(false), importAsMember(false), hasSelfIndex(false),
           hasErrorInfo(false) {}
   } info;

public:
   ImportedName() = default;

   /// Produce just the imported name, for clients that don't care
   /// about the details.
   DeclName getDeclName() const { return declName; }
   operator DeclName() const { return getDeclName(); }
   void setDeclName(DeclName name) { declName = name; }

   /// The context into which this declaration will be imported.
   EffectiveClangContext getEffectiveContext() const {
      return effectiveContext;
   }
   void setEffectiveContext(EffectiveClangContext ctx) {
      effectiveContext = ctx;
   }

   /// For an initializer, the kind of initializer to import.
   CtorInitializerKind getInitKind() const { return info.initKind; }

   /// What kind of accessor this name refers to, if any.
   ImportedAccessorKind getAccessorKind() const { return info.accessorKind; }

   /// For names that map Objective-C error handling conventions into
   /// throwing Swift methods, describes how the mapping is performed.
   Optional<ForeignErrorConvention::Info> getErrorInfo() const {
      if (info.hasErrorInfo)
         return info.errorInfo;
      return None;
   }

   /// For a declaration name that makes the declaration into an
   /// instance member, the index of the "Self" parameter.
   Optional<unsigned> getSelfIndex() const {
      if (info.hasSelfIndex)
         return info.selfIndex;
      return None;
   }

   /// Whether this name was explicitly specified via a Clang
   /// swift_name attribute.
   bool hasCustomName() const { return info.hasCustomName; }
   void setHasCustomName() { info.hasCustomName = true; }

   /// Whether this was one of a special class of Objective-C
   /// initializers for which we drop the variadic argument rather
   /// than refuse to import the initializer.
   bool droppedVariadic() const { return info.droppedVariadic; }

   /// Whether this is a global being imported as a member
   bool importAsMember() const { return info.importAsMember; }

   /// Whether any name was imported.
   explicit operator bool() const { return static_cast<bool>(declName); }

   /// Whether this declaration is a property accessor (getter or setter).
   bool isPropertyAccessor() const {
      switch (getAccessorKind()) {
         case ImportedAccessorKind::None:
         case ImportedAccessorKind::SubscriptGetter:
         case ImportedAccessorKind::SubscriptSetter:
            return false;

         case ImportedAccessorKind::PropertyGetter:
         case ImportedAccessorKind::PropertySetter:
            return true;
      }
   }

   /// Whether this declaration is a subscript accessor (getter or setter).
   bool isSubscriptAccessor() const {
      switch (getAccessorKind()) {
         case ImportedAccessorKind::None:
         case ImportedAccessorKind::PropertyGetter:
         case ImportedAccessorKind::PropertySetter:
            return false;

         case ImportedAccessorKind::SubscriptGetter:
         case ImportedAccessorKind::SubscriptSetter:
            return true;
      }

      llvm_unreachable("Invalid ImportedAccessorKind.");
   }
};

/// Strips a trailing "Notification", if present. Returns {} if name doesn't end
/// in "Notification", or it there would be nothing left.
StringRef stripNotification(StringRef name);

/// Class to determine the Swift name of foreign entities. Currently fairly
/// stateless and borrows from the ClangImporter::Implementation, but in the
/// future will be more self-contained and encapsulated.
class NameImporter {
   AstContext &swiftCtx;
   const PlatformAvailability &availability;

   clang::Sema &clangSema;
   EnumInfoCache enumInfos;
   StringScratchSpace scratch;

   const bool inferImportAsMember;

   // TODO: remove when we drop the options (i.e. import all names)
   using CacheKeyType =
   std::pair<const clang::NamedDecl *, ImportNameVersion>;

   /// Cache for repeated calls
   llvm::DenseMap<CacheKeyType, ImportedName> importNameCache;

   /// The set of property names that show up in the defining module of
   /// an Objective-C class.
   llvm::DenseMap<std::pair<const clang::ObjCInterfaceDecl *, char>,
      std::unique_ptr<InheritedNameSet>> allProperties;

public:
   NameImporter(AstContext &ctx, const PlatformAvailability &avail,
                clang::Sema &cSema, bool inferIAM)
      : swiftCtx(ctx), availability(avail), clangSema(cSema),
        enumInfos(clangSema.getPreprocessor()),
        inferImportAsMember(inferIAM) {}

   /// Determine the Swift name for a Clang decl
   ImportedName importName(const clang::NamedDecl *decl,
                           ImportNameVersion version,
                           clang::DeclarationName preferredName =
                           clang::DeclarationName());

   /// Attempts to import the name of \p decl with each possible
   /// ImportNameVersion. \p action will be called with each unique name.
   ///
   /// In this case, "unique" means either the full name is distinct or the
   /// effective context is distinct. This method does not attempt to handle
   /// "unresolved" contexts in any special way---if one name references a
   /// particular Clang declaration and the other has an unresolved context that
   /// will eventually reference that declaration, the contexts will still be
   /// considered distinct.
   ///
   /// If \p action returns false, the current name will \e not be added to the
   /// set of seen names.
   ///
   /// The active name for \p activeVerion is always first, followed by the
   /// other names in the order of
   /// ImportNameVersion::forEachOtherImportNameVersion.
   ///
   /// Returns \c true if it fails to import name for the active version.
   bool forEachDistinctImportName(
      const clang::NamedDecl *decl, ImportNameVersion activeVersion,
      llvm::function_ref<bool(ImportedName, ImportNameVersion)> action);

   /// Imports the name of the given Clang macro into Swift.
   Identifier importMacroName(const clang::IdentifierInfo *clangIdentifier,
                              const clang::MacroInfo *macro);

   AstContext &getContext() { return swiftCtx; }
   const LangOptions &getLangOpts() const { return swiftCtx.LangOpts; }

   Identifier getIdentifier(StringRef name) {
      return swiftCtx.getIdentifier(name);
   }

   StringScratchSpace &getScratch() { return scratch; }

   bool isInferImportAsMember() const { return inferImportAsMember; }

   EnumInfo getEnumInfo(const clang::EnumDecl *decl) {
      return enumInfos.getEnumInfo(decl);
   }
   EnumKind getEnumKind(const clang::EnumDecl *decl) {
      return enumInfos.getEnumKind(decl);
   }

   clang::Sema &getClangSema() { return clangSema; }
   clang::ASTContext &getClangContext() {
      return getClangSema().getASTContext();
   }
   clang::Preprocessor &getClangPreprocessor() {
      return getClangSema().getPreprocessor();
   }

   /// Retrieve the inherited name set for the given Objective-C class.
   const InheritedNameSet *getAllPropertyNames(
      clang::ObjCInterfaceDecl *classDecl,
      bool forInstance);

private:
   bool enableObjCInterop() const { return swiftCtx.LangOpts.EnableObjCInterop; }

   /// Look for a method that will import to have the same name as the
   /// given method after importing the Nth parameter as an elided error
   /// parameter.
   bool hasErrorMethodNameCollision(const clang::ObjCMethodDecl *method,
                                    unsigned paramIndex,
                                    StringRef suffixToStrip);

   /// Test to see if there is a value with the same name as 'proposedName' in
   /// the same module as the decl
   bool hasNamingConflict(const clang::NamedDecl *decl,
                          const clang::IdentifierInfo *proposedName,
                          const clang::TypedefNameDecl *cfTypedef);

   Optional<ForeignErrorConvention::Info>
   considerErrorImport(const clang::ObjCMethodDecl *clangDecl,
                       StringRef &baseName,
                       SmallVectorImpl<StringRef> &paramNames,
                       ArrayRef<const clang::ParmVarDecl *> params,
                       bool isInitializer, bool hasCustomName);

   EffectiveClangContext determineEffectiveContext(const clang::NamedDecl *,
                                                   const clang::DeclContext *,
                                                   ImportNameVersion version);

   ImportedName importNameImpl(const clang::NamedDecl *,
                               ImportNameVersion version,
                               clang::DeclarationName);
};

} // importer
} // polar

namespace llvm {
// Provide DenseMapInfo for ImportNameVersion.
template <> struct DenseMapInfo<polar::importer::ImportNameVersion> {
   using ImportNameVersion = polar::importer::ImportNameVersion;
   using DMIU = DenseMapInfo<unsigned>;
   static inline ImportNameVersion getEmptyKey() {
      return (ImportNameVersion)DMIU::getEmptyKey();
   }
   static inline ImportNameVersion getTombstoneKey() {
      return (ImportNameVersion)DMIU::getTombstoneKey();
   }
   static unsigned getHashValue(const ImportNameVersion &Val) {
      return DMIU::getHashValue(Val.rawValue);
   }
   static bool isEqual(const ImportNameVersion &LHS,
                       const ImportNameVersion &RHS) {
      return LHS == RHS;
   }
};
}

#endif // POLARPHP_CLANG_INTERNAL_IMPORT_NAME_H
