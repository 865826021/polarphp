//===--- Version.cpp - Swift Version Number -------------------------------===//
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
// This file defines several version-related utility functions for Swift.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/CharInfo.h"
#include "clang/Basic/Version.inc"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/llvm-config.h"
#include "polarphp/ast/DiagnosticsParse.h"
#include "polarphp/basic/LLVM.h"
#include "polarphp/basic/Version.h"
#include "polarphp/global/Config.h"

#include <vector>

#define TOSTR2(X) #X
#define TOSTR(X) TOSTR2(X)

#ifdef POLAR_VERSION_PATCHLEVEL
/// Helper macro for POLAR_VERSION_STRING.
#define POLAR_MAKE_VERSION_STRING(X, Y, Z) TOSTR(X) "." TOSTR(Y) "." TOSTR(Z)

/// A string that describes the Swift version number, e.g., "1.0".
#define POLAR_VERSION_STRING                                                   \
  POLAR_MAKE_VERSION_STRING(POLAR_VERSION_MAJOR, POLAR_VERSION_MINOR,          \
                            POLAR_VERSION_PATCHLEVEL)
#else
/// Helper macro for POLAR_VERSION_STRING.
#define POLAR_MAKE_VERSION_STRING(X, Y) TOSTR(X) "." TOSTR(Y)

/// A string that describes the polarphp version number, e.g., "1.0".
#define POLAR_VERSION_STRING                                                   \
  POLAR_MAKE_VERSION_STRING(POLAR_VERSION_MAJOR, POLAR_VERSION_MINOR)
#endif

namespace polar {
namespace version {

/// Print a string of the form "LLVM xxxxx, Clang yyyyy, polar zzzzz",
/// where each placeholder is the revision for the associated repository.
static void printFullRevisionString(raw_ostream &out) {
   // Arbitrarily truncate to 10 characters. This should be enough to unique
   // Git hashes for the time being, and certainly enough for SVN revisions,
   // while keeping the version string from being ridiculously long.
#if defined(LLVM_VERSION_STRING)
   out << "LLVM " << LLVM_VERSION_STRING;
# if defined(CLANG_REVISION) || defined(POLAR_REVISION)
  out << ", ";
# endif
#endif

#if defined(CLANG_VERSION_STRING)
   out << "Clang " << CLANG_VERSION_STRING;
# if defined(POLARPHP_VERSION)
  out << ", ";
# endif
#endif

#if defined(POLARPHP_VERSION)
   out << "polarphp " << POLARPHP_VERSION;
#endif
}

static void splitVersionComponents(
   SmallVectorImpl<std::pair<StringRef, SourceRange>> &SplitComponents,
   StringRef &VersionString, SourceLoc Loc,
   bool skipQuote = false) {
   SourceLoc Start = (Loc.isValid() && skipQuote) ? Loc.getAdvancedLoc(1) : Loc;
   SourceLoc End = Start;

   // Split the version string into tokens separated by the '.' character.
   while (!VersionString.empty()) {
      StringRef SplitComponent, Rest;
      std::tie(SplitComponent, Rest) = VersionString.split('.');

      if (Loc.isValid()) {
         End = End.getAdvancedLoc(SplitComponent.size());
      }
      auto Range = Loc.isValid() ? SourceRange(Start, End) : SourceRange();
      if (Loc.isValid())
         End = End.getAdvancedLoc(1);
      Start = End;
      SplitComponents.push_back({SplitComponent, Range});
      VersionString = Rest;
   }
}

Optional<Version> Version::parseCompilerVersionString(
   StringRef VersionString, SourceLoc Loc, DiagnosticEngine *Diags) {

   Version CV;
   SmallString<16> digits;
   llvm::raw_svector_ostream OS(digits);
   SmallVector<std::pair<StringRef, SourceRange>, 5> SplitComponents;

   splitVersionComponents(SplitComponents, VersionString, Loc,
      /*skipQuote=*/true);

   uint64_t ComponentNumber;
   bool isValidVersion = true;

   auto checkVersionComponent = [&](unsigned Component, SourceRange Range) {
      unsigned limit = CV.Components.empty() ? 9223371 : 999;

      if (Component > limit) {
         if (Diags)
            Diags->diagnose(Range.start,
                            diag::compiler_version_component_out_of_range, limit);
         isValidVersion = false;
      }
   };

   for (size_t i = 0; i < SplitComponents.size(); ++i) {
      StringRef SplitComponent;
      SourceRange Range;
      std::tie(SplitComponent, Range) = SplitComponents[i];

      // Version components can't be empty.
      if (SplitComponent.empty()) {
         if (Diags)
            Diags->diagnose(Range.start, diag::empty_version_component);
         isValidVersion = false;
         continue;
      }

      // The second version component isn't used for comparison.
      if (i == 1) {
         if (!SplitComponent.equals("*")) {
            if (Diags)
               Diags->diagnose(Range.start, diag::unused_compiler_version_component)
                  .fixItReplaceChars(Range.start, Range.end, "*");
         }

         CV.Components.push_back(0);
         continue;
      }

      // All other version components must be numbers.
      if (!SplitComponent.getAsInteger(10, ComponentNumber)) {
         checkVersionComponent(ComponentNumber, Range);
         CV.Components.push_back(ComponentNumber);
         continue;
      } else {
         if (Diags)
            Diags->diagnose(Range.start, diag::version_component_not_number);
         isValidVersion = false;
      }
   }

   if (CV.Components.size() > 5) {
      if (Diags)
         Diags->diagnose(Loc, diag::compiler_version_too_many_components);
      isValidVersion = false;
   }

   return isValidVersion ? Optional<Version>(CV) : None;
}

Optional<Version> Version::parseVersionString(StringRef VersionString,
                                              SourceLoc Loc,
                                              DiagnosticEngine *Diags) {
   Version TheVersion;
   SmallString<16> digits;
   llvm::raw_svector_ostream OS(digits);
   SmallVector<std::pair<StringRef, SourceRange>, 5> SplitComponents;
   // Skip over quote character in string literal.

   if (VersionString.empty()) {
      if (Diags)
         Diags->diagnose(Loc, diag::empty_version_string);
      return None;
   }

   splitVersionComponents(SplitComponents, VersionString, Loc, Diags);

   uint64_t ComponentNumber;
   bool isValidVersion = true;

   for (size_t i = 0; i < SplitComponents.size(); ++i) {
      StringRef SplitComponent;
      SourceRange Range;
      std::tie(SplitComponent, Range) = SplitComponents[i];

      // Version components can't be empty.
      if (SplitComponent.empty()) {
         if (Diags)
            Diags->diagnose(Range.start, diag::empty_version_component);

         isValidVersion = false;
         continue;
      }

      // All other version components must be numbers.
      if (!SplitComponent.getAsInteger(10, ComponentNumber)) {
         TheVersion.Components.push_back(ComponentNumber);
         continue;
      } else {
         if (Diags)
            Diags->diagnose(Range.start,
                            diag::version_component_not_number);
         isValidVersion = false;
      }
   }

   return isValidVersion ? Optional<Version>(TheVersion) : None;
}

Version::Version(StringRef VersionString,
                 SourceLoc Loc,
                 DiagnosticEngine *Diags)
   : Version(*parseVersionString(VersionString, Loc, Diags))
{}

Version Version::getCurrentCompilerVersion() {
#ifdef POLAR_COMPILER_VERSION
   auto currentVersion = Version::parseVersionString(
    POLAR_COMPILER_VERSION, SourceLoc(), nullptr);
  assert(currentVersion.hasValue() &&
         "Embedded Swift language version couldn't be parsed: '"
         POLAR_COMPILER_VERSION
         "'");
  return currentVersion.getValue();
#else
   return Version();
#endif
}

Version Version::getCurrentLanguageVersion() {
#if POLARPHP_VERSION_PATCH
   return {POLARPHP_VERSION_MAJOR, POLARPHP_VERSION_MINOR, POLARPHP_VERSION_PATCH};
#else
   return {POLARPHP_VERSION_MAJOR, POLARPHP_VERSION_MINOR};
#endif
}

raw_ostream &operator<<(raw_ostream &os, const Version &version) {
   if (version.empty())
      return os;
   os << version[0];
   for (size_t i = 1, e = version.size(); i != e; ++i)
      os << '.' << version[i];
   return os;
}

std::string
Version::preprocessorDefinition(StringRef macroName,
                                ArrayRef<uint64_t> componentWeights) const {
   uint64_t versionConstant = 0;

   for (size_t i = 0, e = std::min(componentWeights.size(), Components.size());
        i < e; ++i) {
      versionConstant += componentWeights[i] * Components[i];
   }

   std::string define("-D");
   llvm::raw_string_ostream(define) << macroName << '=' << versionConstant;
   // This isn't using stream.str() so that we get move semantics.
   return define;
}

Version::operator llvm::VersionTuple() const
{
   switch (Components.size()) {
      case 0:
         return llvm::VersionTuple();
      case 1:
         return llvm::VersionTuple((unsigned)Components[0]);
      case 2:
         return llvm::VersionTuple((unsigned)Components[0],
                                   (unsigned)Components[1]);
      case 3:
         return llvm::VersionTuple((unsigned)Components[0],
                                   (unsigned)Components[1],
                                   (unsigned)Components[2]);
      case 4:
      case 5:
         return llvm::VersionTuple((unsigned)Components[0],
                                   (unsigned)Components[1],
                                   (unsigned)Components[2],
                                   (unsigned)Components[3]);
      default:
         llvm_unreachable("swift::version::Version with 6 or more components");
   }
}

Optional<Version> Version::getEffectiveLanguageVersion() const {
   switch (size()) {
      case 0:
         return None;
      case 1:
         break;
      case 2:
         // The only valid explicit language version with a minor
         // component is 4.2.
         if (Components[0] == 4 && Components[1] == 2)
            break;
         return None;
      default:
         // We do not want to permit users requesting more precise effective language
         // versions since accepting such an argument promises more than we're able
         // to deliver.
         return None;
   }
   // @todo
   // FIXME: When we switch to polarphp 5 by default, the "4" case should return
   // a version newer than any released 4.x compiler, and the
   // "5" case should start returning getCurrentLanguageVersion. We should
   // also check for the presence of POLAR_VERSION_PATCHLEVEL, and if that's
   // set apply it to the "3" case, so that Swift 4.0.1 will automatically
   // have a compatibility mode of 3.2.1.
//   switch (Components[0]) {
//      case 4:
//         // Version '4' on its own implies '4.1.50'.
//         if (size() == 1)
//            return Version{4, 1, 50};
//         // This should be true because of the check up above.
//         assert(size() == 2 && Components[0] == 4 && Components[1] == 2);
//         return Version{4, 2};
//      case 5:
//         static_assert(POLARPHP_VERSION_MAJOR == 5,
//                       "getCurrentLanguageVersion is no longer correct here");
//         return Version::getCurrentLanguageVersion();
//      default:
//         return None;
//   }
}

Version Version::asMajorVersion() const {
   if (empty())
      return {};
   Version res;
   res.Components.push_back(Components[0]);
   return res;
}

std::string Version::asAPINotesVersionString() const {
   // Other than for "4.2.x", map the Swift major version into
   // the API notes version for Swift. This has the effect of allowing
   // API notes to effect changes only on Swift major versions,
   // not minor versions.
   if (size() >= 2 && Components[0] == 4 && Components[1] == 2)
      return "4.2";
   return llvm::itostr(Components[0]);
}

bool operator>=(const class Version &lhs,
                const class Version &rhs) {

   // The empty compiler version represents the latest possible version,
   // usually built from the source repository.
   if (lhs.empty())
      return true;

   auto n = std::max(lhs.size(), rhs.size());

   for (size_t i = 0; i < n; ++i) {
      auto lv = i < lhs.size() ? lhs[i] : 0;
      auto rv = i < rhs.size() ? rhs[i] : 0;
      if (lv < rv)
         return false;
      else if (lv > rv)
         return true;
   }
   // Equality
   return true;
}

bool operator<(const class Version &lhs, const class Version &rhs) {

   return !(lhs >= rhs);
}

bool operator==(const class Version &lhs,
                const class Version &rhs) {
   auto n = std::max(lhs.size(), rhs.size());
   for (size_t i = 0; i < n; ++i) {
      auto lv = i < lhs.size() ? lhs[i] : 0;
      auto rv = i < rhs.size() ? rhs[i] : 0;
      if (lv != rv)
         return false;
   }
   return true;
}

std::pair<unsigned, unsigned> retrieve_polarphp_numeric_version() {
   return { POLARPHP_VERSION_MAJOR, POLARPHP_VERSION_MINOR };
}

std::string retrieve_polarphp_full_version(Version effectiveVersion) {
   std::string buf;
   llvm::raw_string_ostream OS(buf);

#ifdef POLAR_VENDOR
   OS << POLAR_VENDOR " ";
#endif

   OS << "polarphp version " POLAR_VERSION_STRING;
#ifndef POLAR_COMPILER_VERSION
   OS << "-dev";
#endif

   if (!(effectiveVersion == Version::getCurrentLanguageVersion())) {
      OS << " effective-" << effectiveVersion;
   }

#if defined(POLAR_COMPILER_VERSION)
   OS << " (polarphp-" POLAR_COMPILER_VERSION;
#if defined(CLANG_COMPILER_VERSION)
  OS << " clang-" CLANG_COMPILER_VERSION;
#endif
  OS << ")";
#elif defined(LLVM_VERSION_STRING) || defined(CLANG_VERSION_STRING) || \
      defined(POLARPHP_VERSION)
   OS << " (";
  printFullRevisionString(OS);
  OS << ")";
#endif

   // Suppress unused function warning
   (void)&printFullRevisionString;

   return OS.str();
}

std::string retrieve_polarphp_revision() {
#ifdef POLARPHP_VERSION
   return POLARPHP_VERSION;
#else
   return "";
#endif
}

} // end namespace version
} // end namespace polar
