//===--- DiagnosticsCommon.def - Diagnostics Text ---------------*- C++ -*-===//
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
//  This file defines diagnostics that can be emitted across the whole compiler.
//  Each diagnostic is described using one of three kinds (error, warning, or
//  note) along with a unique identifier, category, options, and text, and is
//  followed by a signature describing the diagnostic argument kinds.
//
//===----------------------------------------------------------------------===//
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/04/25.

#if !(defined(DIAG) || (defined(ERROR) && defined(WARNING) && defined(NOTE) && defined(REMARK)))
#  error Must define either DIAG or the set {ERROR,WARNING,NOTE,REMARK}
#endif

#ifndef DIAG
#  define DIAG(ERROR,ID,Options,Text,Signature)
#endif

#ifndef ERROR
#  define ERROR(ID,Options,Text,Signature)   \
  DIAG(ERROR,ID,Options,Text,Signature)
#endif

#ifndef WARNING
#  define WARNING(ID,Options,Text,Signature) \
  DIAG(WARNING,ID,Options,Text,Signature)
#endif

#ifndef NOTE
#  define NOTE(ID,Options,Text,Signature) \
  DIAG(NOTE,ID,Options,Text,Signature)
#endif

#ifndef REMARK
#  define REMARK(ID,Options,Text,Signature) \
  DIAG(REMARK,ID,Options,Text,Signature)
#endif

ERROR(invalid_diagnostic, none,
      "INTERNAL ERROR: this diagnostic should not be produced", ())

ERROR(not_implemented, none,
      "INTERNAL ERROR: feature not implemented: %0", (StringRef))

ERROR(error_opening_output, none,
      "error opening '%0' for output: %1", (StringRef, StringRef))

ERROR(error_no_group_info, none,
      "no group info found for file: '%0'", (StringRef))

NOTE(brace_stmt_suggest_do, none,
     "did you mean to use a 'do' statement?", ())

// Generic disambiguation
NOTE(while_parsing_as_left_angle_bracket, none,
     "while parsing this '<' as a type parameter bracket", ())

// Generic determinism-forcing override.
REMARK(remark_max_determinism_overriding, none,
         "SWIFTC_MAXIMUM_DETERMINISM overriding %0", (StringRef))

// FIXME: This is used both as a parse error (a literal "super" outside a
// method) and a type-checker error ("super" in a method of a non-class type).
ERROR(super_not_in_class_method, none,
      "'super' cannot be used outside of class members", ())

ERROR(class_func_not_in_class, none,
      "class methods are only allowed within classes; "
      "use 'static' to declare a %select{static|requirement fulfilled by either a static or class}0 method", (bool))
ERROR(class_var_not_in_class, none,
      "class properties are only allowed within classes; "
      "use 'static' to declare a %select{static|requirement fulfilled by either a static or class}0 property", (bool))

// FIXME: Used by both the parser and the type-checker.
ERROR(func_decl_without_brace, PointsToFirstBadToken,
      "expected '{' in body of function declaration", ())

NOTE(convert_let_to_var, none,
     "change 'let' to 'var' to make it mutable", ())

NOTE(note_typo_candidate, none,
     "did you mean '%0'?", (StringRef))

NOTE(profile_read_error, none,
     "failed to load profile data '%0': '%1'", (StringRef, StringRef))

ERROR(generic_signature_not_minimal,none,
      "generic requirement '%0' is redundant in %1", (StringRef, StringRef))

WARNING(protocol_extension_redundant_requirement,none,
      "requirement of '%1' to '%2' is redundant in an extension of '%0'",
      (StringRef, StringRef, StringRef))

ERROR(attr_only_on_parameters, none,
      "'%0' may only be used on parameters", (StringRef))

ERROR(function_type_no_parens, none,
      "single argument function types require parentheses", ())

// FIXME: Used by swift-api-digester. Don't want to set up a separate diagnostics
// file just for a few errors.
ERROR(sdk_node_unrecognized_key, none,
      "unrecognized key '%0' in SDK node", (StringRef))
ERROR(sdk_node_unrecognized_node_kind, none,
      "unrecognized SDK node kind '%0'", (StringRef))
ERROR(sdk_node_unrecognized_type_attr_kind, none,
      "unrecognized type attribute '%0' in SDK node", (StringRef))
ERROR(sdk_node_unrecognized_decl_attr_kind, none,
      "unrecognized declaration attribute '%0' in SDK node", (StringRef))
ERROR(sdk_node_unrecognized_decl_kind, none,
      "unrecognized declaration kind '%0' in SDK node", (StringRef))

//------------------------------------------------------------------------------
// MARK: Circular reference diagnostics
//------------------------------------------------------------------------------
ERROR(circular_reference, none,
      "circular reference", ())

ERROR(redundant_type_alias_define, none,
      "redundant type alias declaration", ())

NOTE(circular_reference_through, none,
     "through reference here", ())

#ifndef DIAG_NO_UNDEF
# if defined(DIAG)
#  undef DIAG
# endif
# undef NOTE
# undef WARNING
# undef ERROR
#endif
