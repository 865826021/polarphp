//===--- DiagnosticsSema.def - Diagnostics Text -----------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file defines diagnostics emitted during semantic analysis and type
//  checking.
//  Each diagnostic is described using one of three kinds (error, warning, or
//  note) along with a unique identifier, category, options, and text, and is
//  followed by a signature describing the diagnostic argument kinds.
//
//===----------------------------------------------------------------------===//

#if !(defined(DIAG) || (defined(ERROR) && defined(WARNING) && defined(NOTE)))
#  error Must define either DIAG or the set {ERROR,WARNING,NOTE}
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

#ifndef FIXIT
#  define FIXIT(ID, Text, Signature)
#endif

NOTE(decl_declared_here,none,
     "%0 declared here", (DeclName))
NOTE(kind_declared_here,none,
     "%0 declared here", (DescriptiveDeclKind))
NOTE(implicit_member_declared_here,none,
     "%1 '%0' is implicitly declared", (StringRef, StringRef))
NOTE(extended_type_declared_here,none,
     "extended type declared here", ())
NOTE(opaque_return_type_declared_here,none,
     "opaque return type declared here", ())

//------------------------------------------------------------------------------
// MARK: Constraint solver diagnostics
//------------------------------------------------------------------------------

ERROR(ambiguous_member_overload_set,none,
      "ambiguous reference to member %0", (DeclName))
ERROR(ambiguous_reference_to_decl,none,
      "ambiguous reference to %0 %1", (DescriptiveDeclKind, DeclName))
ERROR(no_overloads_match_exactly_in_call,none,
      "no exact matches in call to %0 %1",
      (DescriptiveDeclKind, DeclName))
ERROR(no_overloads_match_exactly_in_call_no_labels,none,
      "no exact matches in call to %0 %1",
      (DescriptiveDeclKind, DeclBaseName))
ERROR(no_overloads_match_exactly_in_call_special,none,
      "no exact matches in call to %0",
      (DescriptiveDeclKind))
ERROR(no_overloads_match_exactly_in_assignment,none,
      "no exact matches in assignment to %0",
      (DeclBaseName))

ERROR(ambiguous_subscript,none,
      "ambiguous subscript with base type %0 and index type %1",
      (Type, Type))
ERROR(could_not_find_value_subscript,none,
      "value of type %0 has no subscripts",
      (Type))

ERROR(could_not_find_tuple_member,none,
      "value of tuple type %0 has no member %1", (Type, DeclName))

ERROR(could_not_find_value_member,none,
      "value of type %0 has no member %1", (Type, DeclName))
ERROR(could_not_find_value_member_corrected,none,
      "value of type %0 has no member %1; did you mean %2?",
      (Type, DeclName, DeclName))
ERROR(could_not_find_value_dynamic_member_corrected,none,
      "value of type %0 has no dynamic member %2 using key path from root type %1; did you mean %3?",
      (Type, Type, DeclName, DeclName))
ERROR(could_not_find_value_dynamic_member,none,
      "value of type %0 has no dynamic member %2 using key path from root type %1",
      (Type, Type, DeclName))

ERROR(could_not_find_type_member,none,
      "type %0 has no member %1", (Type, DeclName))
ERROR(could_not_find_type_member_corrected,none,
      "type %0 has no member %1; did you mean %2?",
      (Type, DeclName, DeclName))

ERROR(could_not_find_subscript_member_did_you_mean,none,
      "value of type %0 has no property or method named 'subscript'; "
      "did you mean to use the subscript operator?",
      (Type))

ERROR(could_not_find_enum_case,none,
      "enum type %0 has no case %1; did you mean %2", (Type, DeclName, DeclName))

NOTE(did_you_mean_raw_type,none,
     "did you mean to specify a raw type on the enum declaration?", ())

NOTE(any_as_anyobject_fixit, none,
     "cast 'Any' to 'AnyObject' or use 'as!' to force downcast to a more specific type to access members", ())

ERROR(expected_argument_in_contextual_member,none,
      "member %0 expects argument of type %1", (DeclName, Type))
ERROR(expected_parens_in_contextual_member,none,
      "member %0 is a function; did you mean to call it?", (DeclName))

ERROR(expected_result_in_contextual_member,none,
      "member %0 in %2 produces result of type %1, but context expects %2",
      (DeclName, Type, Type))

ERROR(unexpected_arguments_in_enum_case,none,
      "enum case %0 has no associated values", (DeclName))

ERROR(could_not_use_value_member,none,
      "member %1 cannot be used on value of type %0", (Type, DeclName))
ERROR(could_not_use_type_member,none,
      "member %1 cannot be used on type %0", (Type, DeclName))

ERROR(could_not_use_type_member_on_instance,none,
      "static member %1 cannot be used on instance of type %0",
      (Type, DeclName))
ERROR(could_not_use_enum_element_on_instance,none,
      "enum case %0 cannot be used as an instance member",
      (DeclName))
ERROR(could_not_use_type_member_on_protocol_metatype,none,
      "static member %1 cannot be used on protocol metatype %0",
      (Type, DeclName))
ERROR(could_not_use_instance_member_on_type,none,
      "instance member %1"
      "%select{| of type %2}3 cannot be used on"
      "%select{| instance of nested}3 type %0",
      (Type, DeclName, Type, bool))
ERROR(could_not_use_member_on_existential,none,
      "member %1 cannot be used on value of protocol type %0; use a generic"
      " constraint instead",
      (Type, DeclName))
FIXIT(replace_with_type,"%0",(Type))
FIXIT(insert_type_qualification,"%0.",(Type))

ERROR(candidate_inaccessible,none,
      "%0 is inaccessible due to "
      "'%select{private|fileprivate|internal|%error|%error}1' protection level",
      (DeclName, AccessLevel))

NOTE(note_candidate_inaccessible,none,
     "%0 is inaccessible due to "
     "'%select{private|fileprivate|internal|%error|%error}1' protection level",
     (DeclName, AccessLevel))

ERROR(init_candidate_inaccessible,none,
      "%0 initializer is inaccessible due to "
      "'%select{private|fileprivate|internal|%error|%error}1' protection level",
      (Type, AccessLevel))

ERROR(cannot_pass_rvalue_mutating_subelement,none,
      "cannot use mutating member on immutable value: %0",
      (StringRef))
ERROR(cannot_pass_rvalue_mutating,none,
      "cannot use mutating member on immutable value of type %0",
      (Type))
ERROR(cannot_pass_rvalue_mutating_getter_subelement,none,
      "cannot use mutating getter on immutable value: %0",
      (StringRef))
ERROR(cannot_pass_rvalue_mutating_getter,none,
      "cannot use mutating getter on immutable value of type %0",
      (Type))

ERROR(expression_too_complex,none,
      "the compiler is unable to type-check this expression in reasonable time; "
      "try breaking up the expression into distinct sub-expressions", ())

ERROR(value_type_comparison_with_nil_illegal_did_you_mean,none,
      "value of type %0 cannot be compared by reference; "
      "did you mean to compare by value?",
      (Type))
ERROR(value_type_comparison_with_nil_illegal,none,
      "type %0 is not optional, value can never be nil",
      (Type))

ERROR(cannot_match_expr_pattern_with_value,none,
      "expression pattern of type %0 cannot match values of type %1",
      (Type, Type))
ERROR(cannot_match_unresolved_expr_pattern_with_value,none,
      "pattern cannot match values of type %0",
      (Type))

ERROR(cannot_reference_compare_types,none,
      "cannot check reference equality of functions; operands here have types "
      "%1 and %2",
      (StringRef, Type, Type))

ERROR(cannot_apply_binop_to_args,none,
      "binary operator '%0' cannot be applied to operands of type "
      "%1 and %2",
      (StringRef, Type, Type))

ERROR(cannot_apply_binop_to_same_args,none,
      "binary operator '%0' cannot be applied to two %1 operands",
      (StringRef, Type))

ERROR(cannot_apply_unop_to_arg,none,
      "unary operator '%0' cannot be applied to an operand of type %1",
      (StringRef, Type))

ERROR(cannot_apply_lvalue_unop_to_subelement,none,
      "cannot pass immutable value to mutating operator: %0",
      (StringRef))
ERROR(cannot_apply_lvalue_unop_to_rvalue,none,
      "cannot pass immutable value of type %0 to mutating operator",
      (Type))


ERROR(cannot_apply_lvalue_binop_to_subelement,none,
      "left side of mutating operator isn't mutable: %0", (StringRef))
ERROR(cannot_apply_lvalue_binop_to_rvalue,none,
      "left side of mutating operator has immutable type %0", (Type))

ERROR(cannot_subscript_with_index,none,
      "cannot subscript a value of type %0 with an argument of type %1",
      (Type, Type))

ERROR(cannot_subscript_base,none,
      "cannot subscript a value of type %0",
      (Type))

ERROR(cannot_subscript_ambiguous_base,none,
      "cannot subscript a value of incorrect or ambiguous type", ())

ERROR(cannot_subscript_nil_literal,none,
      "cannot subscript a nil literal value", ())
ERROR(conditional_cast_from_nil,none,
      "nil literal cannot be the source of a conditional cast", ())

ERROR(cannot_pass_rvalue_inout_subelement,none,
      "cannot pass immutable value as inout argument: %0",
      (StringRef))
ERROR(cannot_pass_rvalue_inout_converted,none,
      "inout argument could be set to a value with a type other than %0; "
      "use a value declared as type %1 instead", (Type, Type))
NOTE(inout_change_var_type_if_possible,none,
      "change variable type to %1 if it doesn't need to be declared as %0",
      (Type, Type))
ERROR(cannot_pass_rvalue_inout,none,
      "cannot pass immutable value of type %0 as inout argument",
      (Type))
ERROR(cannot_provide_default_value_inout,none,
      "cannot provide default value to inout parameter %0", (Identifier))

ERROR(cannot_call_with_no_params,none,
      "cannot invoke %select{|initializer for type }1'%0' with no arguments",
      (StringRef, bool))

ERROR(cannot_call_with_params, none,
      "cannot invoke %select{|initializer for type }2'%0' with an argument list"
      " of type '%1'", (StringRef, StringRef, bool))

ERROR(cannot_call_non_function_value,none,
      "cannot call value of non-function type %0", (Type))

ERROR(wrong_argument_labels_overload,none,
      "argument labels '%0' do not match any available overloads", (StringRef))

ERROR(no_candidates_match_result_type,none,
      "no '%0' candidates produce the expected contextual result type %1",
      (StringRef, Type))

ERROR(candidates_no_match_result_type,none,
      "'%0' produces %1, not the expected contextual result type %2",
      (StringRef, Type, Type))



ERROR(invalid_callee_result_type,none,
      "cannot convert call result type %0 to expected type %1",
      (Type, Type))


ERROR(cannot_invoke_closure,none,
      "cannot invoke closure expression with an argument list of type '%0'",
      (StringRef))
ERROR(cannot_invoke_closure_type,none,
      "cannot invoke closure of type %0 with an argument list of type '%1'",
      (Type, StringRef))

ERROR(cannot_infer_closure_type,none,
      "unable to infer closure type in the current context", ())
ERROR(cannot_infer_closure_result_type,none,
      "unable to infer complex closure return type; "
      "add explicit type to disambiguate", ())
FIXIT(insert_closure_return_type,
      "%select{| () }1-> %0 %select{|in }1",
      (Type, bool))

ERROR(incorrect_explicit_closure_result,none,
      "declared closure result %0 is incompatible with contextual type %1",
      (Type, Type))

ERROR(cannot_call_function_value,none,
      "cannot invoke value of function type with argument list '%0'",
      (StringRef))
ERROR(cannot_call_value_of_function_type,none,
      "cannot invoke value of type %0 with argument list '%1'",
      (Type, StringRef))

NOTE(suggest_expected_match,none,
     "%select{expected an argument list|produces result}0 of type '%1'",
     (bool, StringRef))

NOTE(suggest_partial_overloads,none,
      "overloads for '%1' exist with these %select{"
      "partially matching parameter lists|result types}0: %2",
      (bool, StringRef, StringRef))

NOTE(no_binary_op_overload_for_enum_with_payload,none,
      "binary operator '%0' cannot be synthesized for enums "
      "with associated values",
      (StringRef))

ERROR(cannot_convert_initializer_value,none,
      "cannot convert value of type %0 to specified type %1", (Type,Type))
ERROR(cannot_convert_initializer_value_protocol,none,
      "value of type %0 does not conform to specified type %1", (Type,Type))
ERROR(cannot_convert_initializer_value_nil,none,
      "'nil' cannot initialize specified type %0", (Type))

ERROR(cannot_convert_to_return_type,none,
      "cannot convert return expression of type %0 to return type %1",
      (Type,Type))
ERROR(cannot_convert_to_return_type_protocol,none,
      "return expression of type %0 does not conform to %1", (Type,Type))
ERROR(cannot_convert_to_return_type_nil,none,
      "'nil' is incompatible with return type %0", (Type))

ERROR(cannot_convert_thrown_type,none,
      "thrown expression type %0 does not conform to 'Error'", (Type))
ERROR(cannot_throw_error_code,none,
      "thrown error code type %0 does not conform to 'Error'; construct an %1 "
      "instance", (Type, Type))

FIXIT(insert_type_coercion,
      " %select{as!|as}0 %1",(bool, Type))

ERROR(bad_yield_count,none,
      "expected %0 yield value(s)", (unsigned))

ERROR(cannot_throw_nil,none,
      "cannot infer concrete Error for thrown 'nil' value", ())


ERROR(cannot_convert_raw_initializer_value,none,
      "cannot convert value of type %0 to raw type %1", (Type,Type))
ERROR(cannot_convert_raw_initializer_value_nil,none,
      "cannot convert 'nil' to raw type %0", (Type))

ERROR(cannot_convert_default_arg_value,none,
      "default argument value of type %0 cannot be converted to type %1",
      (Type,Type))
ERROR(cannot_convert_default_arg_value_protocol,none,
      "default argument value of type %0 does not conform to %1", (Type,Type))
ERROR(cannot_convert_default_arg_value_nil,none,
      "nil default argument value cannot be converted to type %0", (Type))

ERROR(cannot_convert_argument_value,none,
      "cannot convert value of type %0 to expected argument type %1",
      (Type,Type))

NOTE(candidate_has_invalid_argument_at_position,none,
     "candidate expects %select{|in-out }2value of type %0 for parameter #%1",
     (Type, unsigned, bool))

ERROR(cannot_convert_array_to_variadic,none,
      "cannot pass array of type %0 as variadic arguments of type %1",
      (Type,Type))
NOTE(candidate_would_match_array_to_variadic,none,
     "candidate would match if array elements were passed as"
     " variadic arguments of type %0", (Type))
NOTE(suggest_pass_elements_directly,none,
     "remove brackets to pass array elements directly", ())

ERROR(cannot_convert_argument_value_generic,none,
      "cannot convert value of type %0 (%1) to expected argument type %2 (%3)",
      (Type, StringRef, Type, StringRef))

// @_nonephemeral conversion diagnostics
ERROR(cannot_pass_type_to_non_ephemeral,none,
      "cannot pass %0 to parameter; argument %1 must be a pointer that "
      "outlives the call%select{| to %3}2", (Type, StringRef, bool, DeclName))
WARNING(cannot_pass_type_to_non_ephemeral_warning,none,
        "passing %0 to parameter, but argument %1 should be a pointer that "
        "outlives the call%select{| to %3}2", (Type, StringRef, bool, DeclName))
ERROR(cannot_use_inout_non_ephemeral,none,
      "cannot use inout expression here; argument %0 must be a pointer that "
      "outlives the call%select{| to %2}1", (StringRef, bool, DeclName))
WARNING(cannot_use_inout_non_ephemeral_warning,none,
        "inout expression creates a temporary pointer, but argument %0 should "
        "be a pointer that outlives the call%select{| to %2}1",
        (StringRef, bool, DeclName))
ERROR(cannot_construct_dangling_pointer,none,
      "initialization of %0 results in a dangling %select{|buffer }1pointer",
      (Type, unsigned))
WARNING(cannot_construct_dangling_pointer_warning,none,
        "initialization of %0 results in a dangling %select{|buffer }1pointer",
        (Type, unsigned))
NOTE(ephemeral_pointer_argument_conversion_note,none,
     "implicit argument conversion from %0 to %1 produces a pointer valid only "
     "for the duration of the call%select{| to %3}2",
     (Type, Type, bool, DeclName))
NOTE(ephemeral_use_with_unsafe_pointer,none,
     "use 'withUnsafe%select{Bytes|MutableBytes|Pointer|MutablePointer}0' in "
     "order to explicitly convert argument to %select{buffer |buffer ||}0"
     "pointer valid for a defined scope", (unsigned))
NOTE(ephemeral_use_string_with_c_string,none,
     "use the 'withCString' method on String in order to explicitly "
     "convert argument to pointer valid for a defined scope", ())
NOTE(ephemeral_use_array_with_unsafe_buffer,none,
     "use the 'withUnsafe%select{Bytes|MutableBytes|BufferPointer|"
     "MutableBufferPointer}0' method on Array in order to explicitly convert "
     "argument to buffer pointer valid for a defined scope", (unsigned))
NOTE(candidate_performs_illegal_ephemeral_conv,none,
     "candidate expects pointer that outlives the call for parameter #%0",
     (unsigned))

ERROR(cannot_convert_argument_value_protocol,none,
      "argument type %0 does not conform to expected type %1", (Type, Type))

ERROR(cannot_convert_argument_value_nil,none,
      "'nil' is not compatible with expected argument type %0", (Type))

ERROR(cannot_convert_condition_value,none,
      "cannot convert value of type %0 to expected condition type %1",
      (Type, Type))
ERROR(cannot_convert_condition_value_nil,none,
      "'nil' is not compatible with expected condition type %0", (Type))

ERROR(cannot_yield_rvalue_by_reference_same_type,none,
      "cannot yield immutable value of type %0 as an inout yield", (Type))
ERROR(cannot_yield_rvalue_by_reference,none,
      "cannot yield immutable value of type %0 as an inout yield of type %1",
      (Type,Type))
ERROR(cannot_yield_wrong_type_by_reference,none,
      "cannot yield reference to storage of type %0 as an inout yield of type %1",
      (Type,Type))
ERROR(cannot_convert_yield_value,none,
      "cannot convert value of type %0 to expected yield type %1",
      (Type,Type))
ERROR(cannot_convert_yield_value_protocol,none,
      "yielded type %0 does not conform to expected type %1",
      (Type,Type))
ERROR(cannot_convert_yield_value_nil,none,
      "nil is not compatible with expected yield type %0", (Type))

ERROR(cannot_convert_closure_result,none,
      "cannot convert value of type %0 to closure result type %1",
      (Type,Type))
ERROR(cannot_convert_closure_result_protocol,none,
      "result value of type %0 does not conform to closure result type %1",
      (Type, Type))
ERROR(cannot_convert_closure_result_nil,none,
      "'nil' is not compatible with closure result type %0", (Type))
ERROR(cannot_convert_parent_type,none,
      "cannot convert parent type %0 to expected type %1",
      (Type, Type))

NOTE(generic_argument_mismatch,none,
     "arguments to generic parameter %0 (%1 and %2) are expected to be equal",
     (Identifier, Type, Type))

ERROR(destructor_not_accessible,none,
      "deinitializers cannot be accessed", ())

// Array Element
ERROR(cannot_convert_array_element,none,
      "cannot convert value of type %0 to expected element type %1",
      (Type,Type))
ERROR(cannot_convert_array_element_protocol,none,
      "value of type %0 does not conform to expected element type %1",
      (Type, Type))
ERROR(cannot_convert_array_element_nil,none,
      "'nil' is not compatible with expected element type %0", (Type))

// Dictionary Key
ERROR(cannot_convert_dict_key,none,
      "cannot convert value of type %0 to expected dictionary key type %1",
      (Type,Type))
ERROR(cannot_convert_dict_key_protocol,none,
      "value of type %0 does not conform to expected dictionary key type %1",
      (Type, Type))
ERROR(cannot_convert_dict_key_nil,none,
      "'nil' is not compatible with expected dictionary key type %0", (Type))

// Dictionary Value
ERROR(cannot_convert_dict_value,none,
      "cannot convert value of type %0 to expected dictionary value type %1",
      (Type,Type))
ERROR(cannot_convert_dict_value_protocol,none,
      "value of type %0 does not conform to expected dictionary value type %1",
      (Type, Type))
ERROR(cannot_convert_dict_value_nil,none,
      "'nil' is not compatible with expected dictionary value type %0", (Type))

// Coerce Expr
ERROR(cannot_convert_coerce,none,
      "cannot convert value of type %0 to type %1 in coercion",
      (Type,Type))
ERROR(cannot_convert_coerce_protocol,none,
      "value of type %0 does not conform to %1 in coercion",
      (Type, Type))
ERROR(cannot_convert_coerce_nil,none,
      "'nil' is not compatible with type %0 in coercion", (Type))

// Assign Expr
ERROR(cannot_convert_assign,none,
      "cannot assign value of type %0 to type %1",
      (Type,Type))
NOTE(assign_protocol_conformance_fix_it,none,
     "add missing conformance to %0 to %1 %2",
     (Type, DescriptiveDeclKind, Type))
ERROR(cannot_convert_assign_protocol,none,
      "value of type %0 does not conform to %1 in assignment",
      (Type, Type))
ERROR(cannot_convert_assign_nil,none,
      "'nil' cannot be assigned to type %0", (Type))

// Subscript Assign Expr
ERROR(cannot_convert_subscript_assign,none,
      "cannot assign value of type %0 to subscript of type %1",
      (Type,Type))
ERROR(cannot_convert_subscript_assign_protocol,none,
      "value of type %0 does not conform to %1 in subscript assignment",
      (Type, Type))
ERROR(cannot_convert_subscript_assign_nil,none,
      "'nil' cannot be assigned to subscript of type %0", (Type))

NOTE(cannot_convert_candidate_result_to_contextual_type,none,
     "%0 produces %1, not the expected contextual result type %2",
     (DeclName, Type, Type))

// for ... in expression
ERROR(cannot_convert_sequence_element_value,none,
      "cannot convert sequence element type %0 to expected type %1",
      (Type, Type))
ERROR(cannot_convert_sequence_element_protocol,none,
      "sequence element type %0 does not conform to expected protocol %1",
      (Type, Type))

ERROR(throws_functiontype_mismatch,none,
      "invalid conversion from throwing function of type %0 to "
      "non-throwing function type %1", (Type, Type))

// Key-path expressions.
ERROR(expr_keypath_no_objc_runtime,none,
      "'#keyPath' can only be used with the Objective-C runtime", ())
ERROR(expression_unused_keypath_result,none,
      "result of key path is unused", ())
ERROR(expr_keypath_non_objc_property,none,
      "argument of '#keyPath' refers to non-'@objc' property %0",
      (DeclName))
WARNING(expr_keypath_swift3_objc_inference,none,
        "argument of '#keyPath' refers to property %0 in %1 that depends on "
        "'@objc' inference deprecated in Swift 4",
        (DeclName, Identifier))
ERROR(expr_keypath_type_of_property,none,
      "cannot refer to type member %0 within instance of type %1",
      (DeclName, Type))
ERROR(expr_keypath_generic_type,none,
      "key path cannot refer to generic type %0", (DeclName))
ERROR(expr_keypath_not_property,none,
      "%select{key path|dynamic key path member lookup}2 cannot refer to %0 %1",
      (DescriptiveDeclKind, DeclName, bool))
ERROR(expr_keypath_mutating_getter,none,
      "%select{key path|dynamic key path member lookup}1 cannot refer to %0, "
      "which has a mutating getter",
      (DeclName, bool))
ERROR(expr_keypath_static_member,none,
      "%select{key path|dynamic key path member lookup}1 cannot refer to static member %0",
      (DeclName, bool))
ERROR(expr_keypath_empty,none,
      "empty key path does not refer to a property", ())
ERROR(expr_unsupported_objc_key_path_component,none,
      "an Objective-C key path cannot contain "
      "%select{BAD|subscript|BAD|BAD|optional-forcing|optional-chaining|BAD} "
      "components",
      (unsigned))
ERROR(expr_unsupported_objc_key_path_compound_name,none,
      "an Objective-C key path cannot reference a declaration with a "
      "compound name", ())
ERROR(expr_keypath_no_keypath_type,none,
      "broken standard library: no 'KeyPath' type found", ())
ERROR(expr_swift_keypath_invalid_component,none,
      "invalid component of Swift key path", ())
ERROR(expr_swift_keypath_not_starting_with_type,none,
      "a Swift key path must begin with a type", ())
ERROR(expr_smart_keypath_value_covert_to_contextual_type,none,
      "key path value type %0 cannot be converted to contextual type %1",
      (Type, Type))
ERROR(expr_swift_keypath_empty, none,
      "key path must have at least one component", ())
ERROR(expr_string_interpolation_outside_string,none,
      "string interpolation can only appear inside a string literal", ())
ERROR(expr_keypath_subscript_index_not_hashable, none,
      "subscript index of type %0 in a key path must be Hashable", (Type))
ERROR(expr_smart_keypath_application_type_mismatch,none,
      "key path of type %0 cannot be applied to a base of type %1",
      (Type, Type))
ERROR(expr_swift_keypath_anyobject_root,none,
      "the root type of a Swift key path cannot be 'AnyObject'", ())
WARNING(expr_deprecated_writable_keypath,none,
        "forming a writable keypath to property %0 that is read-only in this context "
        "is deprecated and will be removed in a future release",(DeclName))

ERROR(cannot_return_value_from_void_func,none,
      "unexpected non-void return value in void function", ())

//------------------------------------------------------------------------------
// MARK: Name Binding
//------------------------------------------------------------------------------

ERROR(sema_no_import,Fatal,
      "no such module '%0'", (StringRef))
ERROR(sema_no_import_target,Fatal,
      "could not find module '%0' for target '%1'; "
      "found: %2", (StringRef, StringRef, StringRef))
ERROR(sema_no_import_repl,none,
      "no such module '%0'", (StringRef))
NOTE(sema_no_import_no_sdk,none,
     "did you forget to set an SDK using -sdk or SDKROOT?", ())
NOTE(sema_no_import_no_sdk_xcrun,none,
     "use \"xcrun swiftc\" to select the default macOS SDK "
     "installed with Xcode", ())
WARNING(sema_import_current_module,none,
        "this file is part of module %0; ignoring import", (Identifier))
WARNING(sema_import_current_module_with_file,none,
        "file '%0' is part of module %1; ignoring import",
        (StringRef, Identifier))
ERROR(sema_opening_import,Fatal,
      "opening import file for module %0: %1", (Identifier, StringRef))

ERROR(serialization_load_failed,Fatal,
      "failed to load module '%0'", (StringRef))
ERROR(serialization_malformed_module,Fatal,
      "malformed compiled module: %0", (StringRef))
ERROR(serialization_module_too_new,Fatal,
      "compiled module was created by a newer version of the compiler: %0",
      (StringRef))
ERROR(serialization_module_language_version_mismatch,Fatal,
      "module compiled with Swift %0 cannot be imported by the Swift %1 "
      "compiler: %2",
      (StringRef, StringRef, StringRef))
ERROR(serialization_module_too_old,Fatal,
      "compiled module was created by an older version of the compiler; "
      "rebuild %0 and try again: %1",
      (Identifier, StringRef))
ERROR(serialization_missing_single_dependency,Fatal,
      "missing required module '%0'", (StringRef))
ERROR(serialization_missing_dependencies,Fatal,
      "missing required modules: %0", (StringRef))
ERROR(serialization_circular_dependency,Fatal,
      "circular dependency between modules '%0' and %1",
      (StringRef, Identifier))
ERROR(serialization_missing_underlying_module,Fatal,
      "cannot load underlying module for %0", (Identifier))
ERROR(serialization_name_mismatch,Fatal,
      "cannot load module '%0' as '%1'", (StringRef, StringRef))
ERROR(serialization_name_mismatch_repl,none,
      "cannot load module '%0' as '%1'", (StringRef, StringRef))
ERROR(serialization_target_incompatible,Fatal,
      "module %0 was created for incompatible target %1: %2",
      (Identifier, StringRef, StringRef))
ERROR(serialization_target_incompatible_repl,none,
      "module %0 was created for incompatible target %1: %2",
      (Identifier, StringRef, StringRef))
ERROR(serialization_target_too_new,Fatal,
      "compiling for %0 %1, but module %2 has a minimum "
      "deployment target of %0 %3: %4",
      (StringRef, llvm::VersionTuple, Identifier, llvm::VersionTuple,
       StringRef))
ERROR(serialization_target_too_new_repl,none,
      "compiling for %0 %1, but module %2 has a minimum "
      "deployment target of %0 %3: %4",
      (StringRef, llvm::VersionTuple, Identifier, llvm::VersionTuple,
       StringRef))

ERROR(serialization_fatal,Fatal,
      "fatal error encountered while reading from module '%0'; "
      "please file a bug report with your project and the crash log",
      (StringRef))
NOTE(serialization_compatibility_version_mismatch,none,
     "compiling as Swift %0, with '%1' built as Swift %2 "
     "(this is supported but may expose additional compiler issues)",
     (StringRef, StringRef, StringRef))

ERROR(reserved_member_name,none,
      "type member must not be named %0, since it would conflict with the"
      " 'foo.%1' expression", (DeclName, StringRef))

ERROR(invalid_redecl,none,"invalid redeclaration of %0", (DeclName))
ERROR(invalid_redecl_init,none,
      "invalid redeclaration of synthesized %select{|memberwise }1%0",
      (DeclName, bool))
WARNING(invalid_redecl_swift5_warning,none,
        "redeclaration of %0 is deprecated and will be an error in Swift 5",
        (DeclName))

NOTE(invalid_redecl_prev,none,
     "%0 previously declared here", (DeclName))

ERROR(ambiguous_type_base,none,
      "%0 is ambiguous for type lookup in this context", (Identifier))
ERROR(invalid_member_type,none,
      "%0 is not a member type of %1", (Identifier, Type))
ERROR(invalid_member_type_suggest,none,
      "%0 does not have a member type named %1; did you mean %2?",
      (Type, Identifier, Identifier))
ERROR(invalid_member_reference,none,
      "%0 %1 is not a member type of %2",
      (DescriptiveDeclKind, Identifier, Type))
ERROR(ambiguous_member_type,none,
      "ambiguous type name %0 in %1", (Identifier, Type))
ERROR(no_module_type,none,
      "no type named %0 in module %1", (Identifier, Identifier))
ERROR(ambiguous_module_type,none,
      "ambiguous type name %0 in module %1", (Identifier, Identifier))
ERROR(use_nonmatching_operator,none,
      "%0 is not a %select{binary|prefix unary|postfix unary}1 operator",
      (DeclName, unsigned))
ERROR(unsupported_recursion_in_associated_type_reference,none,
      "unsupported recursion for reference to %select{associated type|type alias}0 %1 of type %2",
      (bool, DeclName, Type))
ERROR(broken_associated_type_witness,none,
      "reference to invalid %select{associated type|type alias}0 %1 of type %2",
      (bool, DeclName, Type))

ERROR(unspaced_binary_operator_fixit,none,
      "missing whitespace between %0 and %1 operators",
      (Identifier, Identifier, bool))
ERROR(unspaced_binary_operator,none,
      "ambiguous missing whitespace between unary and binary operators", ())
NOTE(unspaced_binary_operators_candidate,none,
     "could be %select{binary|postfix}2 %0 and %select{prefix|binary}2 %1",
     (Identifier, Identifier, bool))
ERROR(unspaced_unary_operator,none,
      "unary operators must not be juxtaposed; parenthesize inner expression",
      ())

ERROR(use_unresolved_identifier,none,
      "use of unresolved %select{identifier|operator}1 %0", (DeclName, bool))
ERROR(use_unresolved_identifier_corrected,none,
      "use of unresolved %select{identifier|operator}1 %0; did you mean '%2'?",
      (DeclName, bool, StringRef))
NOTE(confusable_character,none,
      "%select{identifier|operator}0 '%1' contains possibly confused characters; "
      "did you mean to use '%2'?",
      (bool, StringRef, StringRef))
ERROR(use_undeclared_type,none,
      "use of undeclared type %0", (Identifier))
ERROR(use_undeclared_type_did_you_mean,none,
      "use of undeclared type %0; did you mean to use '%1'?", (Identifier, StringRef))
NOTE(note_typo_candidate_implicit_member,none,
     "did you mean the implicitly-synthesized %1 '%0'?", (StringRef, StringRef))
NOTE(note_remapped_type,none,
     "did you mean to use '%0'?", (StringRef))
NOTE(note_module_as_type,none,
     "cannot use module %0 as a type", (Identifier))

ERROR(use_unknown_object_literal_protocol,none,
     "cannot deduce protocol for %0 literal", (StringRef))
ERROR(object_literal_default_type_missing,none,
     "could not infer type of %0 literal", (StringRef))
NOTE(object_literal_resolve_import,none,
     "import %0 to use '%1' as the default %2 literal type",
     (StringRef, StringRef, StringRef))

ERROR(use_local_before_declaration,none,
      "use of local variable %0 before its declaration", (DeclName))
ERROR(unsupported_existential_type,none,
      "protocol %0 can only be used as a generic constraint because it has "
      "Self or associated type requirements", (Identifier))

ERROR(decl_does_not_exist_in_module,none,
      "%select{%error|type|struct|class|enum|protocol|variable|function}0 "
      "%1 does not exist in module %2",
      (/*ImportKind*/ unsigned, Identifier, Identifier))
ERROR(imported_decl_is_wrong_kind,none,
      "%0 was imported as '%1', but is "
      "%select{%error|a type|a struct|a class|an enum|a protocol|a variable|"
      "a function}2",
      (Identifier, StringRef, /*ImportKind*/ unsigned))
ERROR(imported_decl_is_wrong_kind_typealias,none,
      "%0 %1 cannot be imported as '%2'",
      (DescriptiveDeclKind, Type, StringRef))
ERROR(ambiguous_decl_in_module,none,
      "ambiguous name %0 in module %1", (Identifier, Identifier))

ERROR(module_not_testable,Fatal,
      "module %0 was not compiled for testing", (Identifier))

ERROR(module_not_compiled_for_private_import,none,
      "module %0 was not compiled for private import", (Identifier))

ERROR(import_implementation_cannot_be_exported,none,
      "module %0 cannot be both exported and implementation-only", (Identifier))

WARNING(module_not_compiled_with_library_evolution,none,
        "module %0 was not compiled with library evolution support; "
        "using it means binary compatibility for %1 can't be guaranteed",
        (Identifier, Identifier))


// Operator decls
ERROR(ambiguous_operator_decls,none,
      "ambiguous operator declarations found for operator", ())
NOTE(found_this_operator_decl,none,
     "found this matching operator declaration", ())
ERROR(operator_redeclared,none,
      "operator redeclared", ())
NOTE(previous_operator_decl,none,
     "previous operator declaration here", ())
ERROR(declared_operator_without_operator_decl,none,
      "operator implementation without matching operator declaration", ())
ERROR(declared_unary_op_without_attribute,none,
      "unary operator implementation must have a 'prefix' or 'postfix' modifier", ())
ERROR(unary_op_missing_prepos_attribute,none,
      "%select{prefix|postfix}0 unary operator missing "
      "'%select{prefix|postfix}0' modifier", (bool))
NOTE(unary_operator_declaration_here,none,
   "%select{prefix|postfix}0 operator found here", (bool))
ERROR(invalid_arg_count_for_operator,none,
      "operators must have one or two arguments", ())
ERROR(operator_in_local_scope,none,
      "operator functions can only be declared at global or in type scope", ())
ERROR(nonstatic_operator_in_nominal,none,
      "operator %0 declared in type %1 must be 'static'",
      (Identifier, DeclName))
ERROR(nonstatic_operator_in_extension,none,
      "operator %0 declared in extension of %1 must be 'static'",
      (Identifier, TypeRepr*))
ERROR(nonfinal_operator_in_class,none,
      "operator %0 declared in non-final class %1 must be 'final'",
      (Identifier, Type))
ERROR(operator_in_unrelated_type,none,
      "member operator %2%select{| of protocol %0}1 must have at least one "
      "argument of type %select{%0|'Self'}1", (Type, bool, DeclName))

// Precedence groups
ERROR(ambiguous_precedence_groups,none,
      "multiple precedence groups found", ())
NOTE(found_this_precedence_group,none,
     "found this matching precedence group", ())
ERROR(unknown_precedence_group,none,
      "unknown precedence group %0", (Identifier))
ERROR(precedence_group_cycle,none,
      "cycle in '%select{lowerThan|higherThan}0' relation", (bool))
ERROR(higher_than_precedence_group_cycle,none,
      "cycle in higherThan relation: %0", (StringRef))
ERROR(precedence_group_lower_within_module,none,
      "precedence group cannot be given lower precedence than group in same"
      " module; make the other precedence group higher than this one instead",
      ())
ERROR(precedence_group_redeclared,none,
      "precedence group redeclared", ())
NOTE(previous_precedence_group_decl,none,
     "previous precedence group declaration here", ())
NOTE(circular_reference_through_precedence_group, none,
     "through reference to precedence group %0 here", (Identifier))

//------------------------------------------------------------------------------
// MARK: Expression Type Checking Errors
//------------------------------------------------------------------------------
ERROR(types_not_convertible,none,
      "%1 is not %select{convertible to|a subtype of}0 %2",
      (bool, Type, Type))
NOTE(in_cast_expr_types,none,
      "in cast from type %0 to %1",
      (Type, Type))

ERROR(types_not_convertible_use_bool_value,none,
      "%0 is not convertible to %1; did you mean %0.boolValue", (Type, Type))

ERROR(tuple_types_not_convertible_nelts,none,
      "%0 is not convertible to %1, "
      "tuples have a different number of elements", (Type, Type))

ERROR(tuple_types_not_convertible,none,
      "tuple type %0 is not convertible to tuple %1", (Type, Type))

ERROR(invalid_force_unwrap,none,
      "cannot force unwrap value of non-optional type %0", (Type))
ERROR(invalid_optional_chain,none,
      "cannot use optional chaining on non-optional value of type %0",
      (Type))
ERROR(if_expr_cases_mismatch,none,
      "result values in '? :' expression have mismatching types %0 and %1",
      (Type, Type))

ERROR(did_not_call_function_value,none,
      "function value was used as a property; add () to call it",
      ())
ERROR(did_not_call_function,none,
      "function %0 was used as a property; add () to call it",
      (Identifier))
ERROR(did_not_call_method,none,
      "method %0 was used as a property; add () to call it",
      (Identifier))

ERROR(init_not_instance_member_use_assignment,none,
      "'init' is a member of the type; use assignment "
      "to initalize the value instead", ())

ERROR(init_not_instance_member,none,
      "'init' is a member of the type; use 'type(of: ...)' to initialize "
      "a new object of the same dynamic type", ())
ERROR(super_initializer_not_in_initializer,none,
      "'super.init' cannot be called outside of an initializer", ())

WARNING(isa_is_always_true,none, "'%0' test is always true",
        (StringRef))
WARNING(isa_is_foreign_check,none,
      "'is' test is always true because %0 is a Core Foundation type",
      (Type))
WARNING(conditional_downcast_coercion,none,
      "conditional cast from %0 to %1 always succeeds",
      (Type, Type))

WARNING(forced_downcast_noop,none,
        "forced cast of %0 to same type has no effect", (Type))

WARNING(forced_downcast_coercion,none,
      "forced cast from %0 to %1 always succeeds; did you mean to use 'as'?",
      (Type, Type))

// Note: the Boolean at the end indicates whether bridging is required after
// the cast.
WARNING(downcast_same_type,none,
        "forced cast from %0 to %1 %select{only unwraps optionals|only unwraps "
        "and bridges}3; did you mean to use '%2'%select{| with 'as'}3?",
        (Type, Type, StringRef, bool))

// The unsigned value can be 0 (types are equal), 1 (types implicitly convert),
// or 2 (types bridge).
WARNING(conditional_downcast_same_type,none,
        "conditional downcast from %0 to %1 %select{does nothing|"
        "is equivalent to an implicit conversion to an optional %1|is a "
        "bridging conversion; did you mean to use 'as'?}2",
        (Type, Type, unsigned))
WARNING(is_expr_same_type,none,
        "checking a value with optional type %0 against dynamic type %1 "
        "succeeds whenever the value is non-nil; did you mean to use "
        "'!= nil'?", (Type, Type))
WARNING(downcast_to_unrelated,none,
        "cast from %0 to unrelated type %1 always fails", (Type, Type))
NOTE(downcast_to_unrelated_fixit,none,
     "did you mean to call %0 with '()'?", (Identifier))
ERROR(downcast_to_more_optional,none,
      "cannot downcast from %0 to a more optional type %1",
      (Type, Type))
ERROR(optional_chain_noop,none,
      "optional chain has no effect, expression already produces %0",
      (Type))
ERROR(optional_chain_isnt_chaining,none,
      "'?' must be followed by a call, member lookup, or subscript",
      ())
ERROR(pattern_in_expr,none,
      "%0 cannot appear in an expression", (PatternKind))
NOTE(note_call_to_operator,none,
     "in call to operator %0", (DeclName))
NOTE(note_call_to_func,none,
     "in call to function %0", (DeclName))
NOTE(note_call_to_subscript,none,
     "in call to %0", (DeclName))
NOTE(note_call_to_initializer,none,
     "in call to initializer", ())
NOTE(note_init_parameter,none,
     "in initialization of parameter %0", (Identifier))


ERROR(missing_nullary_call,none,
     "function produces expected type %0; did you mean to call it with '()'?",
     (Type))
ERROR(optional_not_unwrapped,none,
      "value of optional type %0 must be unwrapped to a value of type %1",
     (Type, Type))
NOTE(unwrap_with_default_value,none,
     "coalesce using '?" "?' to provide a default when the optional value "
     "contains 'nil'", ())
NOTE(unwrap_with_force_value,none,
     "force-unwrap using '!' to abort execution if the optional value contains "
     "'nil'", ())
NOTE(unwrap_iuo_initializer,none,
     "value inferred to be type %0 when initialized with an implicitly "
     "unwrapped value", (Type))
NOTE(unwrap_with_guard,none,
     "short-circuit using 'guard' to exit this function early "
     "if the optional value contains 'nil'", ())
ERROR(optional_base_not_unwrapped,none,
      "value of optional type %0 must be unwrapped to refer to member %1 of "
      "wrapped base type %2", (Type, DeclName, Type))
NOTE(optional_base_chain,none,
     "chain the optional using '?' to access member %0 only for non-'nil' "
     "base values", (DeclName))
ERROR(missing_unwrap_optional_try,none,
      "value of optional type %0 not unwrapped; did you mean to use 'try!' "
      "or chain with '?'?",
     (Type))
ERROR(missing_forced_downcast,none,
      "%0 is not convertible to %1; "
      "did you mean to use 'as!' to force downcast?", (Type, Type))
ERROR(missing_explicit_conversion,none,
      "%0 is not implicitly convertible to %1; "
      "did you mean to use 'as' to explicitly convert?", (Type, Type))
ERROR(missing_address_of,none,
      "passing value of type %0 to an inout parameter requires explicit '&'",
      (Type))
ERROR(missing_address_of_yield,none,
      "yielding mutable value of type %0 requires explicit '&'",
      (Type))
ERROR(extraneous_address_of,none,
      "use of extraneous '&'",
      ())
ERROR(extra_address_of,none,
      "'&' used with non-inout argument of type %0",
      (Type))
ERROR(extra_address_of_unsafepointer,none,
      "'&' is not allowed passing array value as %0 argument",
      (Type))
ERROR(cannot_pass_inout_arg_to_subscript,none,
      "cannot pass an inout argument to a subscript; use "
      "'withUnsafeMutablePointer' to explicitly convert argument "
      "to a pointer", ())

ERROR(incorrect_property_wrapper_reference,none,
      "cannot convert value %0 of type %1 to expected type %2, "
      "use %select{wrapper|wrapped value}3 instead",
      (Identifier, Type, Type, bool))
ERROR(incorrect_property_wrapper_reference_member,none,
      "referencing %0 %1 requires %select{wrapper|wrapped value of type}2 %3",
      (DescriptiveDeclKind, DeclName, bool, Type))

ERROR(missing_init_on_metatype_initialization,none,
      "initializing from a metatype value must reference 'init' explicitly",
      ())
ERROR(extra_argument_labels,none,
      "extraneous argument label%select{|s}0 '%1' in %select{call|subscript}2",
      (bool, StringRef, bool))
ERROR(missing_argument_labels,none,
      "missing argument label%select{|s}0 '%1' in %select{call|subscript}2",
      (bool, StringRef, bool))
ERROR(wrong_argument_labels,none,
      "incorrect argument label%select{|s}0 in %select{call|subscript}3 "
      "(have '%1', expected '%2')",
      (bool, StringRef, StringRef, bool))
ERROR(argument_out_of_order_named_named,none,
      "argument %0 must precede argument %1", (Identifier, Identifier))
ERROR(argument_out_of_order_named_unnamed,none,
      "argument %0 must precede unnamed argument #%1", (Identifier, unsigned))
ERROR(argument_out_of_order_unnamed_named,none,
      "unnamed argument #%0 must precede argument %1", (unsigned, Identifier))
ERROR(argument_out_of_order_unnamed_unnamed,none,
      "unnamed argument #%0 must precede unnamed argument #%1",
      (unsigned, unsigned))
NOTE(candidate_expected_different_labels,none,
     "incorrect labels for candidate (have: '%0', expected: '%1')",
     (StringRef, StringRef))

ERROR(member_shadows_global_function,none,
      "use of %0 refers to %1 %2 rather than %3 %4 in %5 %6",
      (DeclName, DescriptiveDeclKind, DeclName, DescriptiveDeclKind, DeclName,
       DescriptiveDeclKind, DeclName))
ERROR(member_shadows_global_function_near_match,none,
      "use of %0 nearly matches %3 %4 in %5 %6 rather than %1 %2",
      (DeclName, DescriptiveDeclKind, DeclName, DescriptiveDeclKind, DeclName,
      DescriptiveDeclKind, DeclName))

ERROR(instance_member_use_on_type,none,
      "instance member %1 cannot be used on type %0; "
      "did you mean to use a value of this type instead?", (Type, DeclName))
ERROR(instance_member_in_initializer,none,
      "cannot use instance member %0 within property initializer; "
      "property initializers run before 'self' is available", (DeclName))
ERROR(instance_member_in_default_parameter,none,
      "cannot use instance member %0 as a default parameter", (DeclName))

ERROR(missing_argument_named,none,
      "missing argument for parameter %0 in call", (Identifier))
ERROR(missing_argument_positional,none,
      "missing argument for parameter #%0 in call", (unsigned))
ERROR(missing_arguments_in_call,none,
      "missing arguments for parameters %0 in call", (StringRef))
ERROR(extra_argument_named,none,
      "extra argument %0 in call", (Identifier))
ERROR(extra_argument_positional,none,
      "extra argument in call", ())
ERROR(extra_arguments_in_call,none,
      "extra arguments at positions %0 in call", (StringRef))
ERROR(extra_argument_to_nullary_call,none,
      "argument passed to call that takes no arguments", ())
ERROR(extra_trailing_closure_in_call,none,
      "extra trailing closure passed in call", ())
ERROR(trailing_closure_bad_param,none,
      "trailing closure passed to parameter of type %0 that does not "
      "accept a closure", (Type))
NOTE(candidate_with_extraneous_args,none,
      "candidate %0 requires %1 argument%s1, "
      "but %2 %select{were|was}3 %select{provided|used in closure body}4",
      (Type, unsigned, unsigned, bool, bool))

ERROR(no_accessible_initializers,none,
      "%0 cannot be constructed because it has no accessible initializers",
      (Type))
ERROR(non_nominal_no_initializers,none,
      "non-nominal type %0 does not support explicit initialization",
      (Type))
ERROR(unbound_generic_parameter,none,
      "generic parameter %0 could not be inferred", (Type))
ERROR(unbound_generic_parameter_cast,none,
      "generic parameter %0 could not be inferred in cast to %1", (Type, Type))
NOTE(archetype_declared_in_type,none,
     "%0 declared as parameter to type %1", (Type, Type))
NOTE(unbound_generic_parameter_explicit_fix,none,
     "explicitly specify the generic arguments to fix this issue", ())

ERROR(invalid_dynamic_callable_type,none,
      "@dynamicCallable attribute requires %0 to have either a valid "
      "'dynamicallyCall(withArguments:)' method or "
      "'dynamicallyCall(withKeywordArguments:)' method", (Type))
ERROR(missing_dynamic_callable_kwargs_method,none,
      "@dynamicCallable type %0 cannot be applied with keyword arguments; "
      "missing 'dynamicCall(withKeywordArguments:)' method", (Type))

ERROR(invalid_dynamic_member_lookup_type,none,
      "@dynamicMemberLookup attribute requires %0 to have a "
      "'subscript(dynamicMember:)' method that accepts either "
      "'ExpressibleByStringLiteral' or a key path", (Type))
NOTE(invalid_dynamic_member_subscript, none,
     "add an explicit argument label to this subscript to satisfy "
     "the @dynamicMemberLookup requirement", ())

ERROR(string_index_not_integer,none,
      "String must not be indexed with %0, it has variable size elements",
      (Type))
NOTE(string_index_not_integer_note,none,
     "consider using an existing high level algorithm, "
     "str.startIndex.advanced(by: n), or a projection like str.utf8", ())

ERROR(invalid_c_function_pointer_conversion_expr,none,
      "a C function pointer can only be formed from a reference to a 'func' or "
      "a literal closure", ())
ERROR(c_function_pointer_from_method,none,
      "a C function pointer cannot be formed from a method", ())
ERROR(c_function_pointer_from_generic_function,none,
      "a C function pointer cannot be formed from a reference to a generic "
      "function", ())
ERROR(invalid_autoclosure_forwarding,none,
      "add () to forward @autoclosure parameter", ())
ERROR(invalid_autoclosure_pointer_conversion,none,
      "cannot perform pointer conversion of value of type %0 to autoclosure "
      "result type %1",
      (Type, Type))

//------------------------------------------------------------------------------
// MARK: Type Check Declarations
//------------------------------------------------------------------------------

ERROR(missing_initializer_def,PointsToFirstBadToken,
      "initializer requires a body", ())

WARNING(pound_warning, none, "%0", (StringRef))
ERROR(pound_error, none, "%0", (StringRef))

// Attributes

ERROR(operator_not_func,none,
      "operators must be declared with 'func'", ())
ERROR(redefining_builtin_operator,none,
      "cannot declare a custom %0 '%1' operator", (StringRef, StringRef))
ERROR(attribute_requires_operator_identifier,none,
      "'%0' requires a function with an operator identifier", (StringRef))
ERROR(attribute_requires_single_argument,none,
      "'%0' requires a function with one argument", (StringRef))

ERROR(nominal_type_not_attribute,none,
      "%0 %1 cannot be used as an attribute", (DescriptiveDeclKind, DeclName))

ERROR(mutating_invalid_global_scope,none, "%0 is only valid on methods",
      (SelfAccessKind))
ERROR(mutating_invalid_classes,none, "%0 isn't valid on methods in "
      "classes or class-bound protocols", (SelfAccessKind))

ERROR(functions_mutating_and_not,none,
      "method must not be declared both %0 and %1",
      (SelfAccessKind, SelfAccessKind))
ERROR(static_functions_not_mutating,none,
      "static functions must not be declared mutating", ())

ERROR(modify_mutatingness_differs_from_setter,none,
      "'modify' accessor cannot be %0 when the setter is %1",
      (SelfAccessKind, SelfAccessKind))

ERROR(transparent_in_protocols_not_supported,none,
      "'@_transparent' attribute is not supported on declarations within protocols", ())
ERROR(transparent_in_classes_not_supported,none,
      "'@_transparent' attribute is not supported on declarations within classes", ())

ERROR(invalid_iboutlet,none,
      "only instance properties can be declared @IBOutlet", ())
ERROR(iboutlet_nonobjc_class,none,
      "@IBOutlet property cannot %select{have|be an array of}0 "
      "non-'@objc' class type %1", (bool, Type))
ERROR(iboutlet_nonobjc_protocol,none,
      "@IBOutlet property cannot %select{have|be an array of}0 "
      "non-'@objc' protocol type %1", (bool, Type))
ERROR(iboutlet_nonobject_type,none,
      "@IBOutlet property cannot %select{have|be an array of}0 "
      "non-object type %1", (bool, Type))
ERROR(iboutlet_only_mutable,none,
      "@IBOutlet attribute requires property to be mutable", ())
ERROR(iboutlet_non_optional,none,
      "@IBOutlet property has non-optional type %0", (Type))
NOTE(note_make_optional,none,
      "add '?' to form the optional type %0", (Type))
NOTE(note_make_implicitly_unwrapped_optional,none,
      "add '!' to form an implicitly unwrapped optional", ())

ERROR(invalid_ibdesignable_extension,none,
      "@IBDesignable can only be applied to classes and extensions "
      "of classes", ())
ERROR(invalid_ibinspectable,none,
      "only instance properties can be declared @%0", (StringRef))
ERROR(invalid_ibaction_decl,none,
      "only instance methods can be declared @%0", (StringRef))
ERROR(invalid_ibaction_result,none,
      "methods declared @%0 must %select{|not }1return a value", (StringRef, bool))
ERROR(invalid_ibaction_argument_count,none,
      "@%0 methods must have %1 to %2 arguments",
      (StringRef, int, int))
ERROR(invalid_ibaction_argument_count_exact,none,
      "@%0 methods must have %2 argument%s2",
      (StringRef, int, int))
ERROR(invalid_ibaction_argument_count_max,none,
      "@%0 methods must have at most %2 argument%s2",
      (StringRef, int, int))
NOTE(fixit_rename_in_swift,none,
     "change Swift name to %0", (DeclName))
ERROR(cdecl_not_at_top_level,none,
      "@_cdecl can only be applied to global functions", ())
ERROR(cdecl_empty_name,none,
      "@_cdecl symbol name cannot be empty", ())
ERROR(cdecl_throws,none,
      "raising errors from @_cdecl functions is not supported", ())

ERROR(attr_methods_only,none,
      "only methods can be declared %0", (DeclAttribute))
ERROR(access_control_in_protocol,none,
      "%0 modifier cannot be used in protocols", (DeclAttribute))
NOTE(access_control_in_protocol_detail,none,
     "protocol requirements implicitly have the same access as the "
     "protocol itself", ())
ERROR(access_control_setter,none,
      "'%select{private|fileprivate|internal|public|open}0(set)' modifier can only "
      "be applied to variables and subscripts",
      (AccessLevel))
ERROR(access_control_setter_read_only,none,
      "'%select{private|fileprivate|internal|public|%error}0(set)' modifier cannot be "
      "applied to %select{constants|read-only variables|read-only properties"
      "|read-only subscripts}1",
      (AccessLevel, unsigned))
ERROR(access_control_setter_more,none,
      "%select{private|fileprivate|internal|public|%error}0 "
      "%select{variable|property|subscript}1 cannot have "
      "%select{%error|a fileprivate|an internal|a public|an open}2 setter",
      (AccessLevel, unsigned, AccessLevel))
WARNING(access_control_setter_redundant,none,
      "'%select{private|fileprivate|internal|public|open}0(set)' modifier is "
      "redundant for %select{a private|a fileprivate|an internal|a public|an open}2 "
      "%1",
      (AccessLevel, DescriptiveDeclKind, AccessLevel))
WARNING(access_control_ext_member_more,none,
    "'%select{%error|fileprivate|internal|public|open}0' modifier conflicts "
    "with extension's default access of "
    "'%select{private|fileprivate|internal|public|%error}1'",
    (AccessLevel, AccessLevel))
WARNING(access_control_ext_member_redundant,none,
    "'%select{%error|fileprivate|internal|public|%error}0' modifier is redundant "
    "for %1 declared in %select{a private (equivalent to fileprivate)|a fileprivate"
    "|an internal|a public|%error}2 extension",
    (AccessLevel, DescriptiveDeclKind, AccessLevel))
ERROR(access_control_ext_requirement_member_more,none,
    "cannot declare %select{%error|a fileprivate|an internal|a public|an open}0 %1 "
    "in an extension with %select{private|fileprivate|internal|public|%error}2 "
    "requirements",
    (AccessLevel, DescriptiveDeclKind, AccessLevel))
ERROR(access_control_extension_more,none,
      "extension of %select{private|fileprivate|internal|%error|%error}0 %1 cannot "
      "be declared %select{%error|fileprivate|internal|public|%error}2",
      (AccessLevel, DescriptiveDeclKind, AccessLevel))
ERROR(access_control_extension_open,none,
      "extensions cannot use 'open' as their default access; use 'public'",
      ())
ERROR(access_control_open_bad_decl,none,
      "only classes and overridable class members can be declared 'open';"
      " use 'public'", ())

ERROR(invalid_decl_attribute,none,
      "'%0' attribute cannot be applied to this declaration", (DeclAttribute))
ERROR(invalid_decl_modifier,none,
      "%0 modifier cannot be applied to this declaration", (DeclAttribute))
ERROR(attribute_does_not_apply_to_type,none,
      "attribute does not apply to type", ())
ERROR(optional_attribute_non_protocol,none,
      "'optional' can only be applied to protocol members", ())
ERROR(optional_attribute_non_objc_protocol,none,
      "'optional' can only be applied to members of an @objc protocol", ())
ERROR(optional_attribute_missing_explicit_objc,none,
      "'optional' requirements are an Objective-C compatibility feature; add '@objc'",
      ())
ERROR(objcmembers_attribute_nonclass,none,
      "'@objcMembers' attribute can only be applied to a class", ())
ERROR(optional_attribute_initializer,none,
      "'optional' cannot be applied to an initializer", ())
ERROR(unavailable_method_non_objc_protocol,none,
      "protocol members can only be marked unavailable in an @objc protocol",
      ())
ERROR(missing_in_class_init_1,none,
      "stored property %0 requires an initial value%select{| or should be "
      "@NSManaged}1", (Identifier, bool))
ERROR(missing_in_class_init_2,none,
      "stored properties %0 and %1 require initial values%select{| or should "
      "be @NSManaged}2",
      (Identifier, Identifier, bool))
ERROR(missing_in_class_init_3plus,none,
      "stored properties %0, %1, %select{and %2|%2, and others}3 "
      "require initial values%select{| or should be @NSManaged}4",
      (Identifier, Identifier, Identifier, bool, bool))
NOTE(requires_stored_property_inits_here,none,
     "%select{superclass|class}1 %0 requires all stored properties to have "
     "initial values%select{| or use @NSManaged}2", (Type, bool, bool))
ERROR(class_without_init,none,
      "class %0 has no initializers", (Type))
NOTE(note_no_in_class_init_1,none,
     "stored property %0 without initial value prevents synthesized "
     "initializers",
     (Identifier))
NOTE(note_no_in_class_init_2,none,
     "stored properties %0 and %1 without initial values prevent synthesized "
     "initializers",
     (Identifier, Identifier))
NOTE(note_no_in_class_init_3plus,none,
     "stored properties %0, %1, %select{and %2|%2, and others}3 "
     "without initial values prevent synthesized initializers",
     (Identifier, Identifier, Identifier, bool))
ERROR(missing_unimplemented_init_runtime,none,
      "standard library error: missing _unimplementedInitializer", ())
ERROR(missing_undefined_runtime,none,
      "standard library error: missing _undefined", ())

WARNING(expr_dynamic_lookup_swift3_objc_inference,none,
        "reference to %0 %1 of %2 depends on '@objc' inference "
        "deprecated in Swift 4",
        (DescriptiveDeclKind, DeclName, Identifier))

ERROR(inherited_default_value_not_in_designated_constructor,none,
      "default value inheritance via 'super' is only valid on the parameters of "
      "designated initializers", ())
ERROR(inherited_default_value_used_in_non_overriding_constructor,none,
      "default value inheritance via 'super' can only be used when "
      "overriding a designated initializer", ())
ERROR(corresponding_param_not_defaulted,none,
      "default value inheritance via 'super' requires that the corresponding "
      "parameter of the overridden designated initializer has a default value",
      ())
NOTE(inherited_default_param_here,none,
     "corresponding parameter declared here", ())

WARNING(option_set_zero_constant,none,
        "static property %0 produces an empty option set",
        (Identifier))
NOTE(option_set_empty_set_init,none,
     "use [] to pilence this warning", ())

ERROR(originally_defined_in_dupe_platform,none,
      "duplicate version number for platform %0", (StringRef))

// Alignment attribute
ERROR(alignment_not_power_of_two,none,
      "alignment value must be a power of two", ())

// Enum annotations
ERROR(indirect_case_without_payload,none,
      "enum case %0 without associated value cannot be 'indirect'", (Identifier))
ERROR(indirect_case_in_indirect_enum,none,
      "enum case in 'indirect' enum cannot also be 'indirect'", ())
WARNING(enum_frozen_nonpublic,none,
        "%0 has no effect on non-public enums", (DeclAttribute))

// Variables (var and let).
ERROR(getset_init,none,
      "variable with getter/setter cannot have an initial value", ())

ERROR(unimplemented_static_var,none,
      "%select{ERROR|static|class}1 stored properties not supported"
      "%select{ in this context| in generic types| in classes| in protocol extensions}0"
      "%select{|; did you mean 'static'?}2",
      (unsigned, StaticSpellingKind, unsigned))
ERROR(observingprop_requires_initializer,none,
      "non-member observing properties require an initializer", ())
ERROR(global_requires_initializer,none,
      "global '%select{var|let}0' declaration requires an initializer expression"
      "%select{ or getter/setter specifier|}0", (bool))
ERROR(static_requires_initializer,none,
      "%select{ERROR|'static var'|'class var'|}0 declaration requires an initializer "
      "expression or getter/setter specifier", (StaticSpellingKind))
ERROR(pattern_type_access,none,
      "%select{%select{variable|constant}0|property}1 "
      "%select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}3|private or fileprivate}4"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}3}2 "
      "because its type uses "
      "%select{a private|a fileprivate|an internal|%error|%error}5 type",
      (bool, bool, bool, AccessLevel, bool, AccessLevel))
WARNING(pattern_type_access_warn,none,
        "%select{%select{variable|constant}0|property}1 "
        "%select{should be declared %select{private|fileprivate|internal|%error|%error}5"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}3}2 "
        "because its type uses "
        "%select{a private|a fileprivate|an internal|%error|%error}5 type",
        (bool, bool, bool, AccessLevel, bool, AccessLevel))
ERROR(pattern_type_access_inferred,none,
      "%select{%select{variable|constant}0|property}1 "
      "%select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}3|private or fileprivate}4"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}3}2 "
      "because its type %6 uses "
      "%select{a private|a fileprivate|an internal|%error|%error}5 type",
      (bool, bool, bool, AccessLevel, bool, AccessLevel, Type))
WARNING(pattern_type_access_inferred_warn,none,
        "%select{%select{variable|constant}0|property}1 "
        "%select{should be declared %select{private|fileprivate|internal|%error|%error}5"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}3}2 "
        "because its type %6 uses "
        "%select{a private|a fileprivate|an internal|%error|%error}5 type",
        (bool, bool, bool, AccessLevel, bool, AccessLevel, Type))

ERROR(pattern_type_not_usable_from_inline,none,
      "type referenced from a '@usableFromInline' "
      "%select{%select{variable|constant}0|property}1 "
      "must be '@usableFromInline' or public",
      (bool, bool))
WARNING(pattern_type_not_usable_from_inline_warn,none,
        "type referenced from a '@usableFromInline' "
        "%select{%select{variable|constant}0|property}1 "
        "should be '@usableFromInline' or public",
        (bool, bool))
ERROR(pattern_type_not_usable_from_inline_frozen,none,
      "type referenced from a stored property in a '@frozen' struct must "
      "be '@usableFromInline' or public",
      (/*ignored*/bool, /*ignored*/bool))
ERROR(pattern_type_not_usable_from_inline_inferred,none,
      "type referenced from a '@usableFromInline' "
      "%select{%select{variable|constant}0|property}1 "
      "with inferred type %2 "
      "must be '@usableFromInline' or public",
      (bool, bool, Type))
WARNING(pattern_type_not_usable_from_inline_inferred_warn,none,
        "type referenced from a '@usableFromInline' "
        "%select{%select{variable|constant}0|property}1 "
        "with inferred type %2 "
        "should be '@usableFromInline' or public",
        (bool, bool, Type))
ERROR(pattern_type_not_usable_from_inline_inferred_frozen,none,
      "type referenced from a stored property with inferred type %2 in a "
      "'@frozen' struct must be '@usableFromInline' or public",
      (/*ignored*/bool, /*ignored*/bool, Type))

ERROR(pattern_binds_no_variables,none,
      "%select{property|global variable}0 declaration does not bind any "
      "variables",
      (unsigned))
ERROR(variable_bound_by_no_pattern,none,
      "variable %0 is not bound by any pattern",
      (DeclName))

WARNING(optional_ambiguous_case_ref,none,
        "assuming you mean '%0.%2'; did you mean '%1.%2' instead?",
        (StringRef, StringRef, StringRef))
NOTE(optional_fixit_ambiguous_case_ref,none,
     "explicitly specify 'Optional' to pilence this warning", ())
NOTE(type_fixit_optional_ambiguous_case_ref,none,
     "use '%0.%1' instead", (StringRef, StringRef))

ERROR(nscoding_unstable_mangled_name,none,
      "%select{private|fileprivate|nested|local}0 class %1 has an "
      "unstable name when archiving via 'NSCoding'",
      (unsigned, Type))
NOTE(unstable_mangled_name_add_objc_new,none,
     "for new classes, use '@objc' to specify a unique, prefixed Objective-C "
     "runtime name", ())
NOTE(unstable_mangled_name_add_objc,none,
     "for compatibility with existing archives, use '@objc' "
     "to record the Swift 3 runtime name", ())

// Generic types
ERROR(unsupported_type_nested_in_generic_function,none,
      "type %0 cannot be nested in generic function %1",
      (Identifier, DeclName))
ERROR(unsupported_type_nested_in_generic_closure,none,
      "type %0 cannot be nested in closure in generic context",
      (Identifier))
ERROR(unsupported_type_nested_in_protocol,none,
      "type %0 cannot be nested in protocol %1",
      (Identifier, Identifier))
ERROR(unsupported_type_nested_in_protocol_extension,none,
      "type %0 cannot be nested in protocol extension of %1",
      (Identifier, Identifier))
ERROR(unsupported_nested_protocol,none,
      "protocol %0 cannot be nested inside another declaration",
      (Identifier))

// Type aliases
ERROR(type_alias_underlying_type_access,none,
      "type alias %select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}1|private or fileprivate}3"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}1}0 "
      "because its underlying type uses "
      "%select{a private|a fileprivate|an internal|%error|%error}2 type",
      (bool, AccessLevel, AccessLevel, bool))
WARNING(type_alias_underlying_type_access_warn,none,
        "type alias %select{should be declared "
        "%select{private|fileprivate|internal|%error|%error}1"
        "|should not be declared "
        "%select{in this context|fileprivate|internal|public|open}1}0 "
        "because its underlying type uses "
        "%select{a private|a fileprivate|an internal|%error|%error}2 type",
        (bool, AccessLevel, AccessLevel, bool))
ERROR(type_alias_underlying_type_not_usable_from_inline,none,
      "type referenced from the underlying type of a "
      "'@usableFromInline' type alias "
      "must be '@usableFromInline' or public",
      ())
WARNING(type_alias_underlying_type_not_usable_from_inline_warn,none,
        "type referenced from the underlying type of a "
        "'@usableFromInline' type alias "
        "should be '@usableFromInline' or public",
        ())

// Subscripts
ERROR(subscript_type_access,none,
      "subscript %select{must be declared "
      "%select{private|fileprivate|internal|%error|%error}1"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}1}0 "
      "because its %select{index|element type}3 uses "
      "%select{a private|a fileprivate|an internal|%error|%error}2 type",
      (bool, AccessLevel, AccessLevel, bool))
WARNING(subscript_type_access_warn,none,
        "subscript %select{should be declared "
        "%select{private|fileprivate|internal|%error|%error}1"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}1}0 "
        "because its %select{index|element type}3 uses "
        "%select{a private|a fileprivate|an internal|%error|%error}2 type",
        (bool, AccessLevel, AccessLevel, bool))
ERROR(subscript_type_usable_from_inline,none,
      "%select{index type|element type}0 of a '@usableFromInline' subscript "
      "must be '@usableFromInline' or public",
      (bool))
WARNING(subscript_type_usable_from_inline_warn,none,
        "%select{index type|element type}0 of a '@usableFromInline' subscript "
        "should be '@usableFromInline' or public",
        (bool))

// Functions
ERROR(function_type_access,none,
      "%select{function|method|initializer}4 "
      "%select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}1|private or fileprivate}2"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}1}0 "
      "because its %select{parameter|result}5 uses "
      "%select{a private|a fileprivate|an internal|%error|%error}3 type",
      (bool, AccessLevel, bool, AccessLevel, unsigned, bool))
WARNING(function_type_access_warn,none,
        "%select{function|method|initializer}4 "
        "%select{should be declared %select{private|fileprivate|internal|%error|%error}1"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}1}0 "
        "because its %select{parameter|result}5 uses "
        "%select{a private|a fileprivate|an internal|%error|%error}3 type",
        (bool, AccessLevel, bool, AccessLevel, unsigned, bool))
ERROR(function_type_usable_from_inline,none,
      "the %select{parameter|result}1 of a "
      "'@usableFromInline' %select{function|method|initializer}0 "
      "must be '@usableFromInline' or public",
      (unsigned, bool))
WARNING(function_type_usable_from_inline_warn,none,
        "the %select{parameter|result}1 of a "
        "'@usableFromInline' %select{function|method|initializer}0 "
        "should be '@usableFromInline' or public",
        (unsigned, bool))

// Opaque return types
ERROR(opaque_type_invalid_constraint,none,
      "an 'opaque' type must specify only 'Any', 'AnyObject', protocols, "
      "and/or a base class", ())
ERROR(inferred_opaque_type,none,
      "property definition has inferred type %0, involving the 'some' "
      "return type of another declaration", (Type))

// Extensions
ERROR(non_nominal_extension,none,
      "non-nominal type %0 cannot be extended", (Type))
WARNING(composition_in_extended_type,none,
        "extending a protocol composition is not supported; extending %0 "
        "instead", (Type))
NOTE(composition_in_extended_type_alternative,none,
     "did you mean to extend the most specific type %0 instead?", (Type))
ERROR(extension_access_with_conformances,none,
      "%0 modifier cannot be used with extensions that declare "
      "protocol conformances", (DeclAttribute))
ERROR(extension_metatype,none,
      "cannot extend a metatype %0", (Type))
ERROR(extension_specialization,none,
      "constrained extension must be declared on the unspecialized generic "
      "type %0 with constraints specified by a 'where' clause", (Identifier))
ERROR(extension_stored_property,none,
      "extensions must not contain stored properties", ())
NOTE(extension_stored_property_fixit,none,
     "Remove '=' to make %0 a computed property", (Identifier))
ERROR(extension_nongeneric_trailing_where,none,
      "trailing 'where' clause for extension of non-generic type %0",
      (DeclName))
ERROR(extension_protocol_inheritance,none,
      "extension of protocol %0 cannot have an inheritance clause", (DeclName))
ERROR(objc_generic_extension_using_type_parameter,none,
      "extension of a generic Objective-C class cannot access the class's "
      "generic parameters at runtime", ())
NOTE(objc_generic_extension_using_type_parameter_here,none,
     "generic parameter used here", ())
NOTE(objc_generic_extension_using_type_parameter_try_objc,none,
     "add '@objc' to allow uses of 'self' within the function body", ())
ERROR(invalid_nominal_extension,none,
      "extension of type %0 must be declared as an extension of %1",
      (Type, Type))
NOTE(invalid_nominal_extension_rewrite,none,
     "did you mean to extend %0 instead?", (Type))

// Interfaces
ERROR(type_does_not_conform,none,
      "type %0 does not conform to protocol %1", (Type, Type))
ERROR(cannot_use_nil_with_this_type,none,
      "'nil' cannot be used in context expecting type %0", (Type))

ERROR(use_of_equal_instead_of_equality,none,
      "use of '=' in a boolean context, did you mean '=='?", ())

ERROR(type_cannot_conform, none,
      "%select{|value of protocol }0type %1 cannot conform to %2; "
      "only struct/enum/class types can conform to protocols",
      (bool, Type, Type))
NOTE(required_by_opaque_return,none,
    "required by opaque return type of %0 %1", (DescriptiveDeclKind, DeclName))
NOTE(required_by_decl,none,
     "required by %0 %1 where %2 = %3",
     (DescriptiveDeclKind, DeclName, Type, Type))
NOTE(required_by_decl_ref,none,
     "required by referencing %0 %1 on %2 where %3 = %4",
     (DescriptiveDeclKind, DeclName, Type, Type, Type))

ERROR(protocol_does_not_conform_static,none,
      "%0 cannot be used as a type conforming to protocol %1 because %1 "
      "has static requirements",
      (Type, Type))
ERROR(protocol_derivation_is_broken,none,
      "protocol %0 is broken; cannot derive conformance for type %1", (Type, Type))
ERROR(type_does_not_inherit,none,
      "%0 requires that %1 inherit from %2", (Type, Type, Type))
NOTE(type_does_not_inherit_or_conform_requirement,none,
     "requirement specified as %0 : %1%2", (Type, Type, StringRef))
ERROR(types_not_equal,none,
      "%0 requires the types %1 and %2 be equivalent",
      (Type, Type, Type))
ERROR(type_does_not_conform_owner,none,
      "%0 requires that %1 conform to %2", (Type, Type, Type))
ERROR(type_does_not_conform_in_decl_ref,none,
      "referencing %0 %1 on %2 requires that %3 conform to %4",
      (DescriptiveDeclKind, DeclName, Type, Type, Type))
ERROR(type_does_not_conform_anyobject_in_decl_ref,none,
      "referencing %0 %1 on %2 requires that %3 be a class type",
      (DescriptiveDeclKind, DeclName, Type, Type, Type))
ERROR(type_does_not_conform_decl_owner,none,
      "%0 %1 requires that %2 conform to %3",
      (DescriptiveDeclKind, DeclName, Type, Type))
ERROR(type_does_not_conform_anyobject_decl_owner,none,
      "%0 %1 requires that %2 be a class type",
      (DescriptiveDeclKind, DeclName, Type, Type))
ERROR(type_does_not_conform_in_opaque_return,none,
      "return type of %0 %1 requires that %2 %select{conform to %3|be a class type}4",
      (DescriptiveDeclKind, DeclName, Type, Type, bool))
ERROR(types_not_equal_decl,none,
      "%0 %1 requires the types %2 and %3 be equivalent",
      (DescriptiveDeclKind, DeclName, Type, Type))
ERROR(types_not_equal_in_decl_ref,none,
      "referencing %0 %1 on %2 requires the types %3 and %4 be equivalent",
      (DescriptiveDeclKind, DeclName, Type, Type, Type))
ERROR(types_not_inherited_decl,none,
      "%0 %1 requires that %2 inherit from %3",
      (DescriptiveDeclKind, DeclName, Type, Type))
ERROR(types_not_inherited_in_decl_ref,none,
      "referencing %0 %1 on %2 requires that %3 inherit from %4",
      (DescriptiveDeclKind, DeclName, Type, Type, Type))
NOTE(where_requirement_failure_one_subst,none,
     "where %0 = %1", (Type, Type))
NOTE(where_requirement_failure_both_subst,none,
     "where %0 = %1, %2 = %3", (Type, Type, Type, Type))
NOTE(requirement_implied_by_conditional_conformance,none,
     "requirement from conditional conformance of %0 to %1", (Type, Type))
NOTE(wrapped_type_satisfies_requirement,none,
     "wrapped type %0 satisfies this requirement; did you mean to unwrap?", (Type))
NOTE(candidate_types_conformance_requirement,none,
     "candidate requires that %0 conform to %1 "
     "(requirement specified as %2 == %3%4)",
     (Type, Type, Type, Type, StringRef))
NOTE(candidate_types_equal_requirement,none,
     "candidate requires that the types %0 and %1 be equivalent "
     "(requirement specified as %2 == %3%4)",
     (Type, Type, Type, Type, StringRef))
NOTE(candidate_types_inheritance_requirement,none,
     "candidate requires that %1 inherit from %2 "
     "(requirement specified as %2 : %3%4)",
     (Type, Type, Type, Type, StringRef))
NOTE(types_not_equal_requirement,none,
     "requirement specified as %0 == %1%2", (Type, Type, StringRef))
ERROR(type_is_not_a_class,none,
      "%0 requires that %1 be a class type", (Type, Type, Type))
NOTE(anyobject_requirement,none,
     "requirement specified as %0 : 'AnyObject'%2", (Type, Type, StringRef))
ERROR(non_class_cannot_conform_to_class_protocol,none,
      "non-class type %0 cannot conform to class protocol %1",
      (Type, Type))
ERROR(cf_class_cannot_conform_to_objc_protocol,none,
      "Core Foundation class %0 cannot conform to @objc protocol %1 because "
      "Core Foundation types are not classes in Objective-C",
      (Type, Type))
ERROR(objc_runtime_visible_cannot_conform_to_objc_protocol,none,
      "class %0 cannot conform to @objc protocol %1 because "
      "the class is only visible via the Objective-C runtime",
      (Type, Type))
ERROR(objc_generics_cannot_conditionally_conform,none,
      "type %0 cannot conditionally conform to protocol %1 because "
      "the type uses the Objective-C generics model",
      (Type, Type))
ERROR(objc_protocol_cannot_have_conditional_conformance,none,
      "type %0 cannot conditionally conform to @objc protocol %1 because "
      "Objective-C does not support conditional conformances",
      (Type, Type))
ERROR(objc_protocol_in_generic_extension,none,
      "conformance of "
      "%select{class from generic context|generic class}0 "
      "%1 to @objc protocol %2 cannot be in an extension",
      (bool, Type, Type))
ERROR(conditional_conformances_cannot_imply_conformances,none,
      "conditional conformance of type %0 to protocol %1 does not imply conformance to "
      "inherited protocol %2",
      (Type, Type, Type))
NOTE(note_explicitly_state_conditional_conformance_different,none,
     "did you mean to explicitly state the conformance with different bounds?", ())
NOTE(note_explicitly_state_conditional_conformance_relaxed,none,
     "did you mean to explicitly state the conformance with relaxed bounds?", ())
NOTE(note_explicitly_state_conditional_conformance_same,none,
     "did you mean to explicitly state the conformance with the same bounds?", ())
NOTE(note_explicitly_state_conditional_conformance_noneditor,none,
     "did you mean to explicitly state the conformance like '%0where ...'?",
     (StringRef))
ERROR(protocol_has_missing_requirements,none,
      "type %0 cannot conform to protocol %1 because it has requirements that "
      "cannot be satisfied", (Type, Type))
ERROR(protocol_has_missing_requirements_versioned,none,
      "type %0 cannot conform to protocol %1 (compiled with Swift %2) because "
      "it has requirements that could not be loaded in Swift %3",
      (Type, Type, llvm::VersionTuple, llvm::VersionTuple))
ERROR(requirement_restricts_self,none,
      "%0 requirement %1 cannot add constraint '%2%select{:|:| ==|:}3 %4' on "
      "'Self'",
      (DescriptiveDeclKind, DeclName, StringRef, unsigned, StringRef))
ERROR(witness_argument_name_mismatch,none,
      "%select{method|initializer}0 %1 has different argument labels from those "
      "required by protocol %2 (%3)", (bool, DeclName, Type, DeclName))
ERROR(witness_initializer_not_required,none,
      "initializer requirement %0 can only be satisfied by a 'required' "
      "initializer in%select{| the definition of}1 non-final class %2",
      (DeclName, bool, Type))
ERROR(witness_initializer_failability,none,
      "non-failable initializer requirement %0"
      "%select{| in Objective-C protocol}1 cannot be satisfied by a "
      "failable initializer ('init%select{?|!}1')",
      (DeclName, bool))
ERROR(witness_self_non_subtype,none,
      "protocol %0 requirement %1 cannot be satisfied by a non-final class "
      "(%2) because it uses 'Self' in a non-parameter, non-result type "
      "position",
      (Type, DeclName, Type))
ERROR(witness_self_same_type,none,
      "%0 %1 in non-final class %2 cannot be used to satisfy requirement %3 %4"
      " (in protocol %5) due to same-type requirement involving 'Self'",
      (DescriptiveDeclKind, DeclName, Type, DescriptiveDeclKind,
       DeclName, Type))
NOTE(witness_self_weaken_same_type,none,
      "consider weakening the same-type requirement %0 == %1 to a superclass "
      "requirement", (Type, Type))
ERROR(witness_requires_dynamic_self,none,
      "method %0 in non-final class %1 must return 'Self' to conform to "
      "protocol %2",
      (DeclName, Type, Type))
ERROR(witness_requires_class_implementation,none,
      "method %0 in non-final class %1 cannot be implemented in a "
      "protocol extension because it returns 'Self' and has associated type "
      "requirements",
      (DeclName, Type))
ERROR(witness_not_accessible_proto,none,
      "%select{initializer %1|method %1|%select{|setter for }2property %1"
      "|subscript%select{| setter}2}0 must be declared "
      "%select{%error|fileprivate|internal|public|%error}3 because it matches a "
      "requirement in %select{private|fileprivate|internal|public|%error}4 protocol "
      "%5",
      (RequirementKind, DeclName, bool, AccessLevel, AccessLevel, DeclName))
ERROR(witness_not_accessible_type,none,
      "%select{initializer %1|method %1|%select{|setter for }2property %1"
      "|subscript%select{| setter}2}0 must be as accessible as its enclosing "
      "type because it matches a requirement in protocol %5",
      (RequirementKind, DeclName, bool, AccessLevel, AccessLevel, DeclName))
ERROR(type_witness_not_accessible_proto,none,
      "%0 %1 must be declared %select{%error|fileprivate|internal|public|%error}2 "
      "because it matches a requirement in "
      "%select{%error|fileprivate|internal|public|%error}2 protocol %3",
      (DescriptiveDeclKind, DeclName, AccessLevel, DeclName))
ERROR(type_witness_not_accessible_type,none,
      "%0 %1 must be as accessible as its enclosing type because it "
      "matches a requirement in protocol %3",
      (DescriptiveDeclKind, DeclName, AccessLevel, DeclName))
ERROR(witness_not_usable_from_inline,none,
      "%0 %1 must be declared '@usableFromInline' "
      "because it matches a requirement in protocol %2",
      (DescriptiveDeclKind, DeclName, DeclName))
WARNING(witness_not_usable_from_inline_warn,none,
        "%0 %1 should be declared '@usableFromInline' "
        "because it matches a requirement in protocol %2",
        (DescriptiveDeclKind, DeclName, DeclName))
ERROR(type_witness_objc_generic_parameter,none,
      "type %0 involving Objective-C type parameter%select{| %1}2 cannot be "
      "used for associated type %3 of protocol %4",
      (Type, Type, bool, DeclName, DeclName))
NOTE(witness_fix_access,none,
     "mark the %0 as '%select{%error|fileprivate|internal|public|%error}1' to "
     "satisfy the requirement", (DescriptiveDeclKind, AccessLevel))
NOTE(witness_move_to_another_extension,none,
     "move the %0 to another extension where it can be declared "
     "'%select{%error|%error|internal|public|%error}1' to "
     "satisfy the requirement", (DescriptiveDeclKind, AccessLevel))
WARNING(assoc_type_default_conformance_failed,none,
        "default type %0 for associated type %1 does not satisfy constraint "
        "%2: %3", (Type, DeclName, Type, Type))
NOTE(assoc_type_default_here,none,
     "associated type %0 has default type %1 written here", (DeclName, Type))

ERROR(protocol_access,none,
      "%select{protocol must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}1"
      "|private or fileprivate}4 because %select{it refines|its 'where' clause uses}2"
      "|%select{in this context|fileprivate|internal|public|%error}1 "
      "%select{protocol cannot refine|protocol's 'where' clause cannot use}2}0 "
      "%select{a private|a fileprivate|an internal|%error|%error}3 %5",
      (bool, AccessLevel, bool, AccessLevel, bool, DescriptiveDeclKind))
WARNING(protocol_access_warn,none,
        "%select{protocol should be declared "
        "%select{private|fileprivate|internal|%error|%error}1 because "
        "%select{it refines|its 'where' clause uses}2"
        "|%select{in this context|fileprivate|internal|public|%error}1 "
        "%select{protocol should not refine|protocol's 'where' clause should not use}2}0 "
        "%select{a private|a fileprivate|an internal|%error|%error}3 %5",
        (bool, AccessLevel, bool, AccessLevel, bool, DescriptiveDeclKind))
ERROR(protocol_usable_from_inline,none,
      "protocol %select{refined|used}0 by '@usableFromInline' protocol "
      "must be '@usableForInline' or public", (bool))
WARNING(protocol_usable_from_inline_warn,none,
        "protocol %select{refined|used}0 by '@usableFromInline' protocol "
        "should be '@usableForInline' or public", (bool))
ERROR(protocol_property_must_be_computed_var,none,
      "immutable property requirement must be declared as 'var' with a "
      "'{ get }' specifier", ())
ERROR(protocol_property_must_be_computed,none,
      "property in protocol must have explicit { get } or { get set } specifier",
      ())
NOTE(inherited_protocol_does_not_conform,none,
     "type %0 does not conform to inherited protocol %1", (Type, Type))
NOTE(no_witnesses,none,
     "protocol requires "
     "%select{initializer %1|function %1|property %1|subscript}0 with type %2"
     "%select{|; do you want to add a stub?}3",
     (RequirementKind, DeclName, Type, bool))
NOTE(missing_witnesses_general,none, "do you want to add protocol stubs?",
     ())
NOTE(ambiguous_witnesses,none,
     "multiple matching "
     "%select{initializers named %1|functions named %1|properties named %1|"
     "subscript operators}0 with type %2", (RequirementKind, DeclName, Type))
NOTE(ambiguous_witnesses_wrong_name,none,
     "multiple matching "
     "%select{initializers named %1|functions named %1|properties named %1|"
     "subscript operators}0 with type %2", (RequirementKind, DeclName, Type))
NOTE(no_witnesses_type,none,
     "protocol requires nested type %0; do you want to add it?", (Identifier))
NOTE(default_associated_type_req_fail,none,
     "default type %0 for associated type %1 (from protocol %2) "
     "does not %select{inherit from|conform to}4 %3",
     (Type, DeclName, Type, Type, bool))
ERROR(associated_type_access,none,
      "associated type in "
      "%select{a private|a fileprivate|an internal|a public|%error}0 protocol "
      "uses "
      "%select{a private|a fileprivate|an internal|%error|%error}1 type in its "
      "%select{default definition|requirement}2 ",
      (AccessLevel, AccessLevel, unsigned))
WARNING(associated_type_access_warn,none,
        "associated type in "
        "%select{a private|a fileprivate|an internal|a public|%error}0 protocol uses "
        "%select{a private|a fileprivate|an internal|%error|%error}1 type in its "
        "%select{default definition|requirement}2 ",
        (AccessLevel, AccessLevel, unsigned))
ERROR(associated_type_not_usable_from_inline,none,
      "type referenced from a "
      "%select{default definition|requirement}0 of an associated type in a "
      "'@usableFromInline' protocol must be '@usableFromInline' or public",
      (unsigned))
WARNING(associated_type_not_usable_from_inline_warn,none,
        "type referenced from a "
        "%select{default definition|requirement}0 of an associated type in a "
        "'@usableFromInline' protocol should be '@usableFromInline' or public",
        (unsigned))

NOTE(bad_associated_type_deduction,none,
     "unable to infer associated type %0 for protocol %1",
     (DeclName, DeclName))
NOTE(associated_type_deduction_witness_failed,none,
     "candidate would match and infer %0 = %1 if %1 "
     "%select{inherited from|conformed to}3 %2",
     (DeclName, Type, Type, bool))
NOTE(associated_type_witness_conform_impossible,none,
     "candidate can not infer %0 = %1 because %1 "
     "is not a nominal type and so can't conform to %2",
     (DeclName, Type, Type))
NOTE(associated_type_witness_inherit_impossible,none,
     "candidate can not infer %0 = %1 because %1 "
     "is not a class type and so can't inherit from %2",
     (DeclName, Type, Type))

NOTE(ambiguous_associated_type_deduction,none,
     "ambiguous inference of associated type %0: %1 vs. %2",
     (DeclName, Type, Type))
NOTE(associated_type_deduction_witness,none,
     "matching requirement %0 to this declaration inferred associated type to "
     "%1",
     (DeclName, Type))
NOTE(associated_type_deduction_default,none,
     "using associated type default %0", (Type))
NOTE(ambiguous_witnesses_type,none,
     "multiple matching types named %0", (Identifier))
NOTE(protocol_witness_exact_match,none,
     "candidate exactly matches%0", (StringRef))
NOTE(protocol_witness_renamed,none,
     "rename to %0 to satisfy this requirement%1", (DeclName, StringRef))
NOTE(protocol_witness_kind_conflict,none,
     "candidate is not %select{an initializer|a function|a variable|"
     "a subscript}0", (RequirementKind))
NOTE(protocol_witness_type_conflict,none,
     "candidate has non-matching type %0%1", (Type, StringRef))
NOTE(protocol_witness_missing_requirement,none,
     "candidate would match if %0 %select{conformed to|subclassed|"
     "was the same type as}2 %1", (Type, Type, unsigned))

NOTE(protocol_witness_optionality_conflict,none,
     "candidate %select{type has|result type has|parameter type has|"
     "parameter types have|result and parameter types have}0 incorrect "
     "optionality%1",
     (unsigned, StringRef))
ERROR(err_protocol_witness_optionality,none,
      "%select{type|result|parameter|parameters|"
      "result and parameters}0 of %1 %select{has|has|has|have|have|}0"
      " different optionality than required by protocol %2",
      (unsigned, DeclName, DeclName))
WARNING(warn_protocol_witness_optionality,none,
      "%select{type|result|parameter|parameters|"
      "result and parameters}0 of %1 %select{has|has|has|have|have|}0"
      " different optionality than expected by protocol %2",
      (unsigned, DeclName, DeclName))

NOTE(protocol_witness_static_conflict,none,
     "candidate operates on %select{a type|an instance}0, not "
     "%select{an instance|a type}0 as required", (bool))
NOTE(protocol_witness_prefix_postfix_conflict,none,
     "candidate is %select{|prefix, |postfix, }1not "
     "%select{prefix|postfix}0 as required", (bool, unsigned))
NOTE(protocol_witness_mutation_modifier_conflict,none,
     "candidate is marked %0 but protocol does not allow it",
     (SelfAccessKind))
NOTE(protocol_witness_settable_conflict,none,
     "candidate is not settable, but protocol requires it", ())
NOTE(protocol_witness_rethrows_conflict,none,
     "candidate is not 'rethrows', but protocol requires it", ())
NOTE(protocol_witness_throws_conflict,none,
     "candidate throws, but protocol does not allow it", ())
NOTE(protocol_witness_not_objc,none,
     "candidate is explicitly '@nonobjc'", ())

NOTE(protocol_witness_type,none,
     "possibly intended match", ())
NOTE(protocol_witness_nonconform_type,none,
     "possibly intended match %0 does not "
     "%select{inherit from|conform to}2 %1", (Type, Type, bool))

NOTE(protocol_witness_circularity,none,
     "candidate references itself", ())

NOTE(protocol_conformance_here,none,
     "%select{|class }0%1 declares conformance to protocol %2 here",
     (bool, DeclName, DeclName))
NOTE(declared_protocol_conformance_here,none,
     "%select{%0 inherits conformance to protocol %2 from superclass|"
     "%0 declares conformance to protocol %2|"
     "%0 implicitly conforms to protocol %2 (via conformance to %3)|"
     "%0 implicitly conforms to protocol %2}1 here",
     (Type, unsigned, DeclName, DeclName))

ERROR(witness_unavailable,none,
        "unavailable %0 %1 was used to satisfy a requirement of protocol %2",
        (DescriptiveDeclKind, DeclName, DeclName))

ERROR(redundant_conformance,none,
      "redundant conformance of %0 to protocol %1", (Type, DeclName))
ERROR(redundant_conformance_conditional,none,
      "conflicting conformance of %0 to protocol %1; there cannot be more "
      "than one conformance, even with different conditional bounds",
      (Type, DeclName))
WARNING(redundant_conformance_adhoc,none,
        "conformance of %0 to protocol %1 was already stated in "
        "%select{the protocol's|the type's}2 module %3",
        (Type, DeclName, bool, Identifier))
WARNING(redundant_conformance_adhoc_conditional,none,
        "conformance of %0 to protocol %1 conflicts with that stated in "
        "%select{the protocol's|the type's}2 module %3 and will be ignored; "
        "there cannot be more than one conformance, even with different conditional bounds",
        (Type, DeclName, bool, Identifier))
NOTE(redundant_conformance_witness_ignored,none,
     "%0 %1 will not be used to satisfy the conformance to %2",
     (DescriptiveDeclKind, DeclName, DeclName))

// "Near matches"
WARNING(req_near_match,none,
        "%0 %1 nearly matches %select{defaulted|optional}2 requirement %3 "
        "of protocol %4",
        (DescriptiveDeclKind, DeclName, bool, DeclName, DeclName))
NOTE(optional_req_nonobjc_near_match_add_objc,none,
     "add '@objc' to provide an Objective-C entrypoint", ())
NOTE(req_near_match_move,none,
     "move %0 to %select{an|another}1 extension to pilence this warning",
     (DeclName, unsigned))
NOTE(req_near_match_nonobjc,none,
     "add '@nonobjc' to pilence this %select{warning|error}0", (bool))
NOTE(req_near_match_access,none,
     "make %0 %select{ERROR|private|private|non-public|non-public}1 to pilence this "
     "warning", (DeclName, AccessLevel))

// appendInterpolation methods
ERROR(missing_append_interpolation,none,
      "type conforming to 'StringInterpolationInterface' does not implement "
      "a valid 'appendInterpolation' method", ())
WARNING(append_interpolation_static,none,
      "'appendInterpolation' method will never be used because it is static",
      ())
WARNING(append_interpolation_void_or_discardable,none,
      "'appendInterpolation' method does not return 'Void' or have a "
      "discardable result", ())
WARNING(append_interpolation_access_control,none,
      "'appendInterpolation' method is %select{private|fileprivate|internal|"
      "public|open}0, but %1 is %select{private|fileprivate|internal|public|"
      "open}2",
      (AccessLevel, DeclName, AccessLevel))

// Interfaces and existentials
ERROR(assoc_type_outside_of_protocol,none,
      "associated type %0 can only be used with a concrete type or "
      "generic parameter base", (Identifier))
ERROR(typealias_outside_of_protocol,none,
      "type alias %0 can only be used with a concrete type or "
      "generic parameter base", (Identifier))

ERROR(objc_protocol_inherits_non_objc_protocol,none,
      "@objc protocol %0 cannot refine non-@objc protocol %1", (Type, Type))

ERROR(protocol_where_clause_self_requirement,none,
      "constraint with subject type of 'Self' is not supported; "
      "consider adding requirement to protocol inheritance clause instead", ())

ERROR(invalid_protocol_composition_member,none,
      "non-protocol, non-class type %0 cannot be used within a "
      "protocol-constrained type", (Type))
ERROR(protocol_composition_one_class,none,
      "protocol-constrained type cannot contain class %0 because it already "
      "contains class %1", (Type, Type))

ERROR(requires_conformance_nonprotocol,none,
      "type %0 constrained to non-protocol, non-class type %1",
      (Type, Type))
NOTE(requires_conformance_nonprotocol_fixit,none,
     "use '%0 == %1' to require '%0' to be '%1'",
     (StringRef, StringRef))
ERROR(requires_not_suitable_archetype,none,
      "type %0 in conformance requirement does not refer to a "
      "generic parameter or associated type",
      (Type))
WARNING(requires_no_same_type_archetype,none,
        "neither type in same-type constraint (%0 or %1) refers to a "
        "generic parameter or associated type",
        (Type, Type))

ERROR(requires_generic_params_made_equal,none,
      "same-type requirement makes generic parameters %0 and %1 equivalent",
      (Type, Type))
ERROR(requires_generic_param_made_equal_to_concrete,none,
      "same-type requirement makes generic parameter %0 non-generic",
      (Type))
ERROR(recursive_decl_reference,none,
      "%0 %1 references itself", (DescriptiveDeclKind, DeclBaseName))
ERROR(recursive_same_type_constraint,none,
      "same-type constraint %0 == %1 is recursive", (Type, Type))
ERROR(recursive_superclass_constraint,none,
      "superclass constraint %0 : %1 is recursive", (Type, Type))
ERROR(requires_generic_param_same_type_does_not_conform,none,
      "same-type constraint type %0 does not conform to required protocol %1",
      (Type, Identifier))
ERROR(requires_same_concrete_type,none,
      "generic signature requires types %0 and %1 to be the same", (Type, Type))
WARNING(redundant_conformance_constraint,none,
        "redundant conformance constraint %0: %1", (Type, InterfaceDecl *))
NOTE(redundant_conformance_here,none,
     "conformance constraint %1: %2 %select{written here|implied here|"
     "inferred from type here}0",
     (unsigned, Type, InterfaceDecl *))

ERROR(unsupported_recursive_requirements, none,
      "requirement involves recursion that is not currently supported", ())

ERROR(same_type_conflict,none,
      "%select{generic parameter |protocol |}0%1 cannot be equal to both "
      "%2 and %3", (unsigned, Type, Type, Type))
WARNING(redundant_same_type_to_concrete,none,
        "redundant same-type constraint %0 == %1", (Type, Type))
NOTE(same_type_redundancy_here,none,
     "same-type constraint %1 == %2 %select{written here|implied here|"
     "inferred from type here}0",
     (unsigned, Type, Type))
ERROR(requires_superclass_conflict,none,
      "%select{generic parameter %1 cannot|protocol %1 cannot require 'Self' to|"
      "%1 cannot}0 be a subclass of both %2 and %3",
      (unsigned, Type, Type, Type))
WARNING(redundant_superclass_constraint,none,
        "redundant superclass constraint %0 : %1", (Type, Type))
NOTE(superclass_redundancy_here,none,
     "superclass constraint %1 : %2 %select{written here|implied here|"
     "inferred from type here}0",
     (unsigned, Type, Type))

ERROR(conflicting_layout_constraints,none,
      "%select{generic parameter |protocol |}0%1 has conflicting "
      "constraints %2 and %3",
      (unsigned, Type, LayoutConstraint, LayoutConstraint))
WARNING(redundant_layout_constraint,none,
        "redundant constraint %0 : %1", (Type, LayoutConstraint))
NOTE(previous_layout_constraint, none,
     "constraint %1 : %2 %select{written here|implied here|"
     "inferred from type here}0",
     (unsigned, Type, LayoutConstraint))

WARNING(redundant_same_type_constraint,none,
        "redundant same-type constraint %0 == %1", (Type, Type))
NOTE(previous_same_type_constraint, none,
     "previous same-type constraint %1 == %2 "
     "%select{written here|implied here|inferred from type here}0",
     (unsigned, Type, Type))

WARNING(inherited_associated_type_redecl,none,
        "redeclaration of associated type %0 from protocol %1 is better "
        "expressed as a 'where' clause on the protocol", (DeclName, Type))
WARNING(typealias_override_associated_type,none,
        "typealias overriding associated type %0 from protocol %1 is better "
        "expressed as same-type constraint on the protocol", (DeclName, Type))
WARNING(associated_type_override_typealias,none,
        "associated type %0 is redundant with type %0 declared in inherited "
        "%1 %2", (DeclName, DescriptiveDeclKind, Type))

ERROR(associated_type_objc,none,
      "associated type %0 cannot be declared inside '@objc' protocol %1",
      (DeclName, DeclName))

ERROR(generic_param_access,none,
      "%0 %select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}3|private or fileprivate}4"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}2}1 "
      "because its generic %select{parameter|requirement}5 uses "
      "%select{a private|a fileprivate|an internal|%error|%error}3 type",
      (DescriptiveDeclKind, bool, AccessLevel, AccessLevel, bool, bool))
WARNING(generic_param_access_warn,none,
        "%0 %select{should be declared "
        "%select{private|fileprivate|internal|%error|%error}3"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}2}1 "
        "because its generic %select{parameter|requirement}5 uses "
        "%select{a private|a fileprivate|an internal|%error|%error}3 type",
        (DescriptiveDeclKind, bool, AccessLevel, AccessLevel, bool, bool))
ERROR(generic_param_usable_from_inline,none,
      "type referenced from a "
      "generic %select{parameter|requirement}1 of a '@usableFromInline' %0 "
      "must be '@usableFromInline' or public",
      (DescriptiveDeclKind, bool))
WARNING(generic_param_usable_from_inline_warn,none,
        "type referenced from a "
        "generic %select{parameter|requirement}1 of a '@usableFromInline' %0 "
        "should be '@usableFromInline' or public",
        (DescriptiveDeclKind, bool))

ERROR(override_multiple_decls_base,none,
      "declaration %0 cannot override more than one superclass declaration",
      (DeclName))
ERROR(override_multiple_decls_arg_mismatch,none,
      "declaration %0 has different argument labels from any potential "
      "overrides", (DeclName))
NOTE(overridden_near_match_here,none,
     "potential overridden %0 %1 here",
      (DescriptiveDeclKind, DeclName))
ERROR(override_decl_extension,none,
      "overriding %select{|non-@objc }0declarations "
      "%select{in extensions|from extensions}0 is not supported", (bool, bool))
NOTE(overridden_here,none,
     "overridden declaration is here", ())
NOTE(overridden_here_can_be_objc,none,
     "add '@objc' to make this declaration overridable", ())

ERROR(missing_override,none,
      "overriding declaration requires an 'override' keyword", ())
WARNING(missing_override_warn,none,
        "implicit override should be marked with 'override' or suppressed "
        "with '@_nonoverride'", ())

ERROR(multiple_override,none,
      "%0 has already been overridden", (DeclName))
NOTE(multiple_override_prev,none,
     "%0 previously overridden here", (DeclName))

ERROR(override_unavailable, none,
      "cannot override %0 which has been marked unavailable%select{|: %1}1",
      (DeclBaseName, StringRef))

ERROR(override_less_available,none,
      "overriding %0 must be as available as declaration it overrides",
      (DeclBaseName))

ERROR(override_accessor_less_available,none,
      "overriding %0 for %1 must be as available as declaration it overrides",
      (DescriptiveDeclKind, DeclBaseName))

ERROR(override_let_property,none,
      "cannot override immutable 'let' property %0 with the getter of a 'var'",
      (Identifier))


ERROR(override_not_accessible,none,
      "%select{|setter of }0overriding %1 must be as accessible as "
      "%select{its enclosing type|the declaration it overrides}2",
      (bool, DescriptiveDeclKind, bool))

ERROR(override_of_non_open,none,
      "overriding non-open %0 outside of its defining module",
      (DescriptiveDeclKind))

ERROR(method_does_not_override,none,
      "method does not override any method from its superclass", ())
ERROR(property_does_not_override,none,
      "property does not override any property from its superclass", ())
ERROR(subscript_does_not_override,none,
      "subscript does not override any subscript from its superclass", ())
ERROR(initializer_does_not_override,none,
      "initializer does not override a designated initializer from its "
      "superclass", ())
ERROR(failable_initializer_override,none,
      "failable initializer %0 cannot override a non-failable initializer",
      (DeclName))
NOTE(nonfailable_initializer_override_here,none,
     "non-failable initializer %0 overridden here", (DeclName))

NOTE(property_override_here,none,
     "attempt to override property here", ())
NOTE(subscript_override_here,none,
     "attempt to override subscript here", ())
NOTE(convenience_init_override_here,none,
     "attempt to override convenience initializer here", ())
NOTE(override_type_mismatch_with_fixits,none,
     "type does not match superclass %0 with type %1",
     (DescriptiveDeclKind, Type))
NOTE(override_type_mismatch_with_fixits_init,none,
     "type does not match superclass initializer with %select{no arguments|argument %1|arguments %1}0",
     (unsigned, Type))
ERROR(override_nonclass_decl,none,
      "'override' can only be specified on class members", ())
ERROR(nonoverride_wrong_decl_context,none,
      "'@_nonoverride' can only be specified on class or protocol members", ())
ERROR(nonoverride_and_override_attr,none,
      "'override' cannot be combined with '@_nonoverride'", ())
ERROR(override_property_type_mismatch,none,
      "property %0 with type %1 cannot override a property with type %2",
      (Identifier, Type, Type))
ERROR(override_with_stored_property,none,
      "cannot override with a stored property %0", (Identifier))
WARNING(override_with_stored_property_warn,none,
      "cannot override with a stored property %0", (Identifier))
ERROR(observing_readonly_property,none,
      "cannot observe read-only property %0; it can't change", (Identifier))
ERROR(override_mutable_with_readonly_property,none,
      "cannot override mutable property with read-only property %0",
      (Identifier))
ERROR(override_argument_name_mismatch,none,
      "argument labels for %select{method|initializer}0 %1 do not match those "
      "of overridden %select{method|initializer}0 %2",
      (bool, DeclName, DeclName))
ERROR(override_ownership_mismatch,none,
      "cannot override %0 property with %1 property",
      (ReferenceOwnership, ReferenceOwnership))
ERROR(override_dynamic_self_mismatch,none,
      "cannot override a Self return type with a non-Self return type",
      ())
ERROR(override_class_declaration_in_extension,none,
      "cannot override a non-dynamic class declaration from an extension",
      ())
ERROR(override_throws,none,
      "cannot override non-throwing %select{method|initializer}0 with "
      "throwing %select{method|initializer}0", (bool))
ERROR(override_throws_objc,none,
      "overriding a throwing @objc %select{method|initializer}0 with "
      "a non-throwing %select{method|initializer}0 is not supported", (bool))

ERROR(override_optional_mismatch,none,
      "cannot override %0 %select{parameter|index}1 of type %2 with "
      "non-optional type %3",
      (DescriptiveDeclKind, bool, Type, Type))
ERROR(override_optional_result_mismatch,none,
      "cannot override %0 %select{result|element}1 type %2 with "
      "optional type %3",
      (DescriptiveDeclKind, bool, Type, Type))
WARNING(override_unnecessary_IUO,none,
      "overriding %0 parameter of type %1 with implicitly unwrapped optional "
      "type %2",
      (DescriptiveDeclKind, Type, Type))
WARNING(override_unnecessary_result_IUO,none,
      "overriding %0 optional result type %1 with implicitly unwrapped "
      "optional type %2",
      (DescriptiveDeclKind, Type, Type))
NOTE(override_unnecessary_IUO_remove,none,
     "remove '!' to make the parameter required", ())
NOTE(override_unnecessary_IUO_use_strict,none,
     "use '?' to make the result optional", ())
NOTE(override_unnecessary_IUO_pilence,none,
     "add parentheses to pilence this warning", ())

ERROR(override_mutable_covariant_property,none,
      "cannot override mutable property %0 of type %1 with covariant type %2",
      (Identifier, Type, Type))
ERROR(override_mutable_covariant_subscript,none,
      "cannot override mutable subscript of type %0 with covariant type %1",
      (Type, Type))
ERROR(static_decl_already_final,none,
      "static declarations are already final", ())
ERROR(open_decl_cannot_be_final,none,
      "%0 cannot be declared both 'final' and 'open'", (DescriptiveDeclKind))
ERROR(implicitly_final_cannot_be_open,none,
      "%select{'let' properties|members of 'final' classes|"
      "static declarations}0 are implicitly 'final'; use 'public' instead of "
      "'open'", (unsigned))
WARNING(implicitly_final_cannot_be_open_swift4,none,
        "%select{'let' properties|members of 'final' classes|"
        "static declarations}0 are implicitly 'final'; use 'public' instead of "
        "'open'", (unsigned))

WARNING(override_swift3_objc_inference,none,
        "override of %0 %1 from extension of %2 depends on deprecated "
        "inference of '@objc'",
        (DescriptiveDeclKind, DeclName, Identifier))
ERROR(override_method_different_generic_sig,none,
      "overridden method %0 has generic signature %1 which is incompatible with "
      "base method's generic signature %2; expected generic signature to be %3",
      (DeclBaseName, StringRef, StringRef, StringRef))

// Inheritance
ERROR(duplicate_inheritance,none,
  "duplicate inheritance from %0", (Type))
WARNING(duplicate_anyobject_class_inheritance,none,
  "redundant inheritance from 'AnyObject' and Swift 3 'class' keyword", ())
ERROR(inheritance_from_protocol_with_superclass,none,
  "inheritance from class-constrained protocol composition type %0", (Type))
ERROR(multiple_inheritance,none,
  "multiple inheritance from classes %0 and %1", (Type, Type))
ERROR(inheritance_from_non_protocol_or_class,none,
  "inheritance from non-protocol, non-class type %0", (Type))
ERROR(inheritance_from_non_protocol,none,
  "inheritance from non-protocol type %0", (Type))
ERROR(superclass_not_first,none,
  "superclass %0 must appear first in the inheritance clause", (Type))
ERROR(superclass_not_open,none,
  "cannot inherit from non-open class %0 outside of its defining module",
  (Type))
NOTE(superclass_here,none,"superclass is declared here", ())
ERROR(superclass_of_open_not_open,none,
  "superclass %0 of open class must be open", (Type))
ERROR(inheritance_from_final_class,none,
      "inheritance from a final class %0", (Identifier))
ERROR(inheritance_from_unspecialized_objc_generic_class,none,
      "inheritance from a generic Objective-C class %0 must bind "
      "type parameters of %0 to specific concrete types", (Identifier))
ERROR(inheritance_from_class_with_missing_vtable_entries,none,
      "cannot inherit from class %0 because it has overridable members that "
      "could not be loaded",
      (Identifier))
ERROR(inheritance_from_class_with_missing_vtable_entries_versioned,none,
      "cannot inherit from class %0 (compiled with Swift %1) because it has "
      "overridable members that could not be loaded in Swift %2",
      (Identifier, llvm::VersionTuple, llvm::VersionTuple))
ERROR(inheritance_from_cf_class,none,
      "cannot inherit from Core Foundation type %0", (Identifier))
ERROR(inheritance_from_objc_runtime_visible_class,none,
      "cannot inherit from class %0 because it is only visible via the "
      "Objective-C runtime", (Identifier))

// Enums
ERROR(enum_case_access,none,
      "enum case in %select{a private|a fileprivate|an internal|a public|%error}0 enum "
      "uses %select{a private|a fileprivate|an internal|%error|%error}1 type",
      (AccessLevel, AccessLevel))
WARNING(enum_case_access_warn,none,
        "enum case in %select{a private|a fileprivate|an internal|a public|%error}0 enum "
        "uses %select{a private|a fileprivate|an internal|%error|%error}1 type",
        (AccessLevel, AccessLevel))
ERROR(enum_case_usable_from_inline,none,
      "type of enum case in '@usableFromInline' enum "
      "must be '@usableFromInline' or public", ())
WARNING(enum_case_usable_from_inline_warn,none,
        "type of enum case in '@usableFromInline' enum "
        "should be '@usableFromInline' or public", ())
ERROR(enum_stored_property,none,
      "enums must not contain stored properties", ())

// Enum raw types
ERROR(multiple_enum_raw_types,none,
  "multiple enum raw types %0 and %1", (Type, Type))
ERROR(raw_type_not_first,none,
  "raw type %0 must appear first in the enum inheritance clause", (Type))
ERROR(raw_type_not_literal_convertible,none,
      "raw type %0 is not expressible by a string, integer, or floating-point literal",
      (Type))
ERROR(enum_raw_type_not_equatable,none,
      "RawRepresentable conformance cannot be synthesized because raw type %0 is not "
      "Equatable", (Type))
ERROR(enum_raw_type_nonconforming_and_nonsynthable,none,
      "%0 declares raw type %1, but does not conform to RawRepresentable "
      "and conformance could not be synthesized", (Type, Type))
NOTE(enum_declares_rawrep_with_raw_type,none,
      "%0 declares raw type %1, which implies RawRepresentable", (Type, Type))
ERROR(enum_raw_type_access,none,
      "enum %select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}1|private or fileprivate}3"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}1}0 "
      "because its raw type uses "
      "%select{a private|a fileprivate|an internal|%error|%error}2 type",
      (bool, AccessLevel, AccessLevel, bool))
WARNING(enum_raw_type_access_warn,none,
        "enum %select{should be declared "
        "%select{private|fileprivate|internal|%error|%error}1"
        "|should not be declared %select{in this context|fileprivate|internal|public|open}1}0 "
        "because its raw type uses "
        "%select{a private|a fileprivate|an internal|%error|%error}2 type",
        (bool, AccessLevel, AccessLevel, bool))

ERROR(enum_raw_type_not_usable_from_inline,none,
      "type referenced from the raw type of a "
      "'@usableFromInline' enum "
      "must be '@usableFromInline' or public",
      ())
WARNING(enum_raw_type_not_usable_from_inline_warn,none,
        "type referenced from the raw type of a "
        "'@usableFromInline' enum "
        "should be '@usableFromInline' or public",
        ())

ERROR(empty_enum_raw_type,none,
      "an enum with no cases cannot declare a raw type", ())
ERROR(enum_raw_value_without_raw_type,none,
      "enum case cannot have a raw value if the enum does not have a raw type", ())
ERROR(enum_with_raw_type_case_with_argument,none,
      "enum with raw type cannot have cases with arguments", ())
NOTE(enum_raw_type_here,none,
     "declared raw type %0 here", (Type))
ERROR(objc_enum_no_raw_type,none,
      "'@objc' enum must declare an integer raw type", ())
ERROR(objc_enum_raw_type_not_integer,none,
      "'@objc' enum raw type %0 is not an integer type", (Type))
ERROR(enum_non_integer_raw_value_auto_increment,none,
      "enum case must declare a raw value when the preceding raw value is not an integer", ())
ERROR(enum_non_integer_convertible_raw_type_no_value,none,
      "enum cases require explicit raw values when the raw type is not "
      "expressible by integer or string literal", ())
ERROR(enum_raw_value_not_unique,none,
      "raw value for enum case is not unique", ())
NOTE(enum_raw_value_used_here,none,
     "raw value previously used here", ())
NOTE(enum_raw_value_incrementing_from_here,none,
     "raw value auto-incremented from here",())
NOTE(enum_raw_value_incrementing_from_zero,none,
     "raw value implicitly auto-incremented from zero",())
NOTE(construct_raw_representable_from_unwrapped_value,none,
     "construct %0 from unwrapped %1 value", (Type, Type))

ERROR(decl_from_implementation_only_module,none,
      "cannot use %0 %1 %select{here|"
      "in an extension with public or '@usableFromInline' members|"
      "in an extension with conditional conformances}2; %3 has been imported "
      "as implementation-only",
      (DescriptiveDeclKind, DeclName, unsigned, Identifier))
ERROR(conformance_from_implementation_only_module,none,
      "cannot use conformance of %0 to %1 %select{here|"
      "in an extension with public or '@usableFromInline' members|"
      "in an extension with conditional conformances}2; %3 has been imported "
      "as implementation-only",
      (Type, DeclName, unsigned, Identifier))
ERROR(assoc_conformance_from_implementation_only_module,none,
      "cannot use conformance of %0 to %1 in associated type %3 (inferred as "
      "%4); %2 has been imported as implementation-only",
      (Type, DeclName, Identifier, Type, Type))

WARNING(warn_implementation_only_conflict,none,
        "%0 inconsistently imported as implementation-only",
        (Identifier))
NOTE(implementation_only_conflict_here,none,
     "imported as implementation-only here", ())

ERROR(implementation_only_decl_non_override,none,
      "'@_implementationOnly' can only be used on overrides", ())
ERROR(implementation_only_override_changed_type,none,
      "'@_implementationOnly' override must have the same type as the "
      "declaration it overrides (%0)", (Type))
ERROR(implementation_only_override_without_attr,none,
      "override of '@_implementationOnly' %0 should also be declared "
      "'@_implementationOnly'", (DescriptiveDeclKind))
ERROR(implementation_only_override_import_without_attr,none,
      "override of %0 imported as implementation-only must be declared "
      "'@_implementationOnly'", (DescriptiveDeclKind))

// Derived conformances
ERROR(cannot_synthesize_init_in_extension_of_nonfinal,none,
      "implementation of %0 for non-final class cannot be automatically "
      "synthesized in extension because initializer requirement %1 can only be "
      "be satisfied by a 'required' initializer in the class definition",
      (Type, DeclName))
ERROR(cannot_synthesize_in_crossfile_extension,none,
      "implementation of %0 cannot be automatically synthesized in an extension "
      "in a different file to the type", (Type))

ERROR(broken_case_iterable_requirement,none,
      "CaseIterable protocol is broken: unexpected requirement", ())
ERROR(broken_raw_representable_requirement,none,
      "RawRepresentable protocol is broken: unexpected requirement", ())
ERROR(broken_equatable_requirement,none,
      "Equatable protocol is broken: unexpected requirement", ())
ERROR(broken_hashable_requirement,none,
      "Hashable protocol is broken: unexpected requirement", ())
ERROR(broken_hashable_no_hasher,none,
      "Hashable protocol is broken: Hasher type not found", ())
ERROR(broken_errortype_requirement,none,
      "Error protocol is broken: unexpected requirement", ())
ERROR(broken_int_hashable_conformance,none,
      "Int type is broken: does not conform to Hashable", ())
ERROR(broken_int_integer_literal_convertible_conformance,none,
      "Int type is broken: does not conform to ExpressibleByIntegerLiteral", ())
ERROR(no_equal_overload_for_int,none,
      "no overload of '==' for Int", ())
ERROR(broken_coding_key_requirement,none,
      "CodingKey protocol is broken: unexpected requirement", ())
ERROR(broken_encodable_requirement,none,
      "Encodable protocol is broken: unexpected requirement", ())
ERROR(broken_decodable_requirement,none,
      "Decodable protocol is broken: unexpected requirement", ())

NOTE(codable_extraneous_codingkey_case_here,none,
     "CodingKey case %0 does not match any stored properties", (Identifier))
NOTE(codable_non_conforming_property_here,none,
     "cannot automatically synthesize %0 because %1 does not conform to %0", (Type, Type))
NOTE(codable_non_decoded_property_here,none,
     "cannot automatically synthesize %0 because %1 does not have a matching CodingKey and does not have a default value", (Type, Identifier))
NOTE(codable_codingkeys_type_is_not_an_enum_here,none,
     "cannot automatically synthesize %0 because 'CodingKeys' is not an enum", (Type))
NOTE(codable_codingkeys_type_does_not_conform_here,none,
     "cannot automatically synthesize %0 because 'CodingKeys' does not conform to CodingKey", (Type))
NOTE(decodable_no_super_init_here,none,
     "cannot automatically synthesize %0 because superclass does not have a callable %1", (DeclName, DeclName))
NOTE(decodable_super_init_not_designated_here,none,
     "cannot automatically synthesize %0 because implementation would need to call %1, which is not designated", (DeclName, DeclName))
NOTE(decodable_inaccessible_super_init_here,none,
     "cannot automatically synthesize %0 because implementation would need to call %1, which is inaccessible due to "
     "'%select{private|fileprivate|internal|%error|%error}2' protection level",
     (DeclName, DeclName, AccessLevel))
NOTE(decodable_super_init_is_failable_here,none,
     "cannot automatically synthesize %0 because implementation would need to call %1, which is failable", (DeclName, DeclName))
NOTE(decodable_suggest_overriding_init_here,none,
     "did you mean to override 'init(from:)'?", ())
NOTE(codable_suggest_overriding_init_here,none,
     "did you mean to override 'init(from:)' and 'encode(to:)'?", ())

NOTE(missing_member_type_conformance_prevents_synthesis, none,
     "%select{associated value|stored property}0 type %1 does not conform to "
     "protocol %2, preventing synthesized conformance "
     "of %3 to %2",
     (unsigned, Type, Type, Type))

// Dynamic Self
ERROR(dynamic_self_non_method,none,
      "%select{global|local}0 function cannot return 'Self'", (bool))

ERROR(dynamic_self_invalid,none,
      "covariant 'Self' can only appear as the type of a property, subscript or method result; "
      "did you mean '%0'?", (StringRef))
ERROR(dynamic_self_in_mutable_property,none,
      "mutable property cannot have covariant 'Self' type", ())
ERROR(dynamic_self_in_stored_property,none,
      "stored property cannot have covariant 'Self' type", ())
ERROR(dynamic_self_in_mutable_subscript,none,
      "mutable subscript cannot have covariant 'Self' type", ())
ERROR(dynamic_self_invalid_property,none,
      "covariant 'Self' can only appear at the top level of property type", ())
ERROR(dynamic_self_invalid_subscript,none,
      "covariant 'Self' can only appear at the top level of subscript element type", ())
ERROR(dynamic_self_invalid_method,none,
      "covariant 'Self' can only appear at the top level of method result type", ())
ERROR(dynamic_self_stored_property_init,none,
      "covariant 'Self' type cannot be referenced from a stored property initializer", ())
ERROR(dynamic_self_default_arg,none,
      "covariant 'Self' type cannot be referenced from a default argument expression", ())

//------------------------------------------------------------------------------
// MARK: Type Check Attributes
//------------------------------------------------------------------------------

ERROR(attr_only_one_decl_kind,none,
      "%0 may only be used on '%1' declarations", (DeclAttribute,StringRef))
ERROR(attr_not_on_variadic_parameters,none,
      "'%0' must not be used on variadic parameters", (StringRef))
ERROR(attr_not_on_subscript_parameters,none,
      "'%0' must not be used on subscript parameters", (StringRef))

ERROR(override_final,none,
      "%0 overrides a 'final' %1", (DescriptiveDeclKind, DescriptiveDeclKind))

ERROR(override_static,none,
      "cannot override %0", (DescriptiveDeclKind))

ERROR(member_cannot_be_final,none,
      "only classes and class members may be marked with 'final'",
      ())

ERROR(final_not_allowed_here,none,
      "'final' may only be applied to classes, properties, methods, and "
      "subscripts", ())

ERROR(final_not_on_accessors,none,
      "'final' cannot be applied to accessors, it must be put on the "
      "%select{var|let|subscript}0", (unsigned))

ERROR(override_rethrows_with_non_rethrows,none,
      "override of 'rethrows' %select{method|initializer}0 should also "
      "be 'rethrows'", (bool))
ERROR(rethrows_without_throwing_parameter,none,
      "'rethrows' function must take a throwing function argument", ())

ERROR(autoclosure_function_type,none,
      "@autoclosure attribute only applies to function types",
      ())

ERROR(invalid_autoclosure_and_convention_attributes,none,
      "'@convention(%0)' attribute is not allowed on '@autoclosure' types",
      (StringRef))

ERROR(autoclosure_function_input_nonunit,none,
      "argument type of @autoclosure parameter must be '()'", ())

ERROR(escaping_non_function_parameter,none,
      "@escaping attribute may only be used in function parameter position", ())

NOTE(escaping_optional_type_argument, none,
     "closure is already escaping in optional type argument", ())

// @_nonephemeral attribute
ERROR(non_ephemeral_non_pointer_type,none,
      "@_nonephemeral attribute only applies to pointer types", ())

// NSManaged attribute
ERROR(attr_NSManaged_not_instance_member,none,
      "@NSManaged only allowed on an instance property or method", ())
ERROR(attr_NSManaged_not_stored,none,
      "@NSManaged not allowed on %select{computed|observing|addressed}0 "
      "properties", (unsigned))
ERROR(attr_NSManaged_let_property,none,
      "@NSManaged not allowed on a 'let' property", ())
ERROR(attr_NSManaged_initial_value,none,
      "@NSManaged property cannot have an initial value", ())
ERROR(attr_NSManaged_NSCopying,none,
      "@NSManaged property cannot also be marked @NSCopying", ())
ERROR(attr_NSManaged_method_body,none,
      "@NSManaged method cannot have a body; it must be provided at runtime",())


// NSCopying attribute
ERROR(nscopying_only_on_class_properties,none,
      "@NSCopying may only be used on properties in classes",
      ())
ERROR(nscopying_only_mutable,none,
      "@NSCopying requires property to be mutable", ())
ERROR(nscopying_only_stored_property,none,
      "@NSCopying is only valid on stored properties", ())
ERROR(nscopying_doesnt_conform,none,
      "@NSCopying is only valid with types that conform to"
      " the NSCopying protocol", ())

// UIApplicationMain/NSApplicationMain attribute
#define SELECT_APPLICATION_MAIN "select{'UIApplicationMain'|'NSApplicationMain'}"
#define SELECT_APPLICATION_DELEGATE "select{'UIApplicationDelegate'|'NSApplicationDelegate'}"

ERROR(attr_ApplicationMain_not_ApplicationDelegate,none,
      "%" SELECT_APPLICATION_MAIN "0 class must conform to the %" SELECT_APPLICATION_DELEGATE "0 protocol",
      (unsigned))
ERROR(attr_generic_ApplicationMain_not_supported,none,
      "generic %" SELECT_APPLICATION_MAIN "0 classes are not supported",
      (unsigned))
ERROR(attr_ApplicationMain_multiple,none,
      "%" SELECT_APPLICATION_MAIN "0 attribute can only apply to one class in a module",
      (unsigned))
ERROR(attr_ApplicationMain_with_script,none,
      "%" SELECT_APPLICATION_MAIN "0 attribute cannot be used in a module that contains "
      "top-level code",
      (unsigned))
NOTE(attr_ApplicationMain_script_here,none,
     "top-level code defined in this source file",
     ())

#undef SELECT_APPLICATION_MAIN
#undef SELECT_APPLICATION_DELEGATE

// lazy
ERROR(lazy_not_on_let,none,
      "'lazy' cannot be used on a let", ())
ERROR(lazy_not_on_computed,none,
      "'lazy' must not be used on a computed property", ())

ERROR(lazy_on_already_lazy_global,none,
      "'lazy' must not be used on an already-lazy global", ())
ERROR(lazy_not_in_protocol,none,
      "'lazy' isn't allowed on a protocol requirement", ())
ERROR(lazy_requires_initializer,none,
      "lazy properties must have an initializer", ())

ERROR(lazy_requires_single_var,none,
      "'lazy' cannot destructure an initializer", ())

ERROR(lazy_must_be_property,none,
      "lazy is only valid for members of a struct or class", ())
ERROR(lazy_not_observable,none,
      "lazy properties must not have observers", ())
ERROR(lazy_not_strong,none,
      "lazy properties cannot be %0", (ReferenceOwnership))

// Debugger function attribute.
ERROR(attr_for_debugger_support_only,none,
      "@LLDBDebuggerSupport may only be used when debugger support is on", ())

// @_implements
ERROR(implements_attr_protocol_lacks_member,none,
      "protocol %0 has no member %1", (DeclName, DeclName))

ERROR(implements_attr_non_protocol_type,none,
      "non-protocol type in @_implements attribute", ())

ERROR(implements_attr_protocol_not_conformed_to,none,
      "containing type %0 does not conform to protocol %1",
      (DeclName, DeclName))

//------------------------------------------------------------------------------
// MARK: Type Check Expressions
//------------------------------------------------------------------------------

NOTE(found_candidate,none,
     "found this candidate", ())
NOTE(found_candidate_type,none,
     "found candidate with type %0", (Type))

ERROR(no_MaxBuiltinFloatType_found,none,
   "standard library error: _MaxBuiltinFloatType is not properly defined", ())

ERROR(no_member_of_module,none,
      "module %0 has no member named %1", (Identifier, DeclName))

ERROR(super_with_no_base_class,none,
      "'super' members cannot be referenced in a root class", ())

ERROR(bad_init_ref_base, none,
      "'init' can only refer to the initializers of "
      "'self'%select{| or 'super'}0", (bool))
ERROR(init_delegation_outside_initializer,none,
      "initializer delegation can only occur within an initializer", ())
ERROR(init_delegates_and_chains,none,
      "initializer cannot both delegate ('self.init') and chain to a "
      "superclass initializer ('super.init')", ())
NOTE(init_delegation_or_chain,none,
      "previous %select{delegation|chaining}0 call is here", (bool))
ERROR(delegating_convenience_super_init,none,
      "convenience initializer for %0 must delegate (with 'self.init') rather "
      "than chaining to a superclass initializer (with 'super.init')",
      (Type))
ERROR(delegating_designated_init,none,
      "designated initializer for %0 cannot delegate (with 'self.init'); "
      "did you mean this to be a convenience initializer?",
      (Type))
ERROR(delegating_designated_init_in_extension,none,
      "designated initializer for %0 cannot delegate (with 'self.init')",
      (Type))
NOTE(delegation_here,none, "delegation occurs here", ())
ERROR(chain_convenience_init,none,
      "must call a designated initializer of the superclass %0",
      (Type))
ERROR(delegate_chain_nonoptional_to_optional,none,
      "a non-failable initializer cannot %select{delegate|chain}0 to "
      "failable initializer %1 written with 'init?'", (bool, DeclName))
NOTE(init_force_unwrap,none,
     "force potentially-failing result with '!'", ())
NOTE(init_propagate_failure,none,
     "propagate the failure with 'init?'", ())
ERROR(delegate_chain_nonoptional_to_optional_try,none,
      "a non-failable initializer cannot use 'try?' to "
      "%select{delegate|chain}0 to another initializer", (bool))
NOTE(init_delegate_force_try,none,
     "force potentially-failing result with 'try!'", ())
ERROR(init_delegation_nested,none,
      "%select{initializer delegation ('self.init')|"
      "initializer chaining ('super.init')}0 cannot be nested in another "
      "%select{expression|statement}1", (bool, bool))

NOTE(convenience_init_here,none,
     "convenience initializer is declared here", ())
ERROR(designated_init_in_extension,none,
      "designated initializer cannot be declared in an extension of %0; "
      "did you mean this to be a convenience initializer?",
      (DeclName))
ERROR(cfclass_designated_init_in_extension,none,
      "designated initializer cannot be declared in an extension of %0",
      (DeclName))
ERROR(enumstruct_convenience_init,none,
      "delegating initializers in %0 are not marked with 'convenience'",
      (StringRef))
ERROR(nonclass_convenience_init,none,
      "convenience initializer not allowed in non-class type %0",
      (DeclName))
ERROR(cfclass_convenience_init,none,
      "convenience initializers are not supported in extensions of CF types",
      ())
ERROR(dynamic_construct_class,none,
      "constructing an object of class type %0 with a metatype value must use "
      "a 'required' initializer", (Type))
NOTE(note_nonrequired_initializer,none,
      "selected %select{non-required|implicit}0 initializer %1",
     (bool, DeclName))
ERROR(construct_protocol_value,none,
      "value of type %0 is a protocol; it cannot be instantiated",
      (Type))
ERROR(construct_protocol_by_name,none,
      "protocol type %0 cannot be instantiated",
      (Type))

// Operators
ERROR(unknown_binop,none,
       "operator is not a known binary operator", ())
ERROR(non_associative_adjacent_operators,none,
      "adjacent operators are in non-associative precedence group %0",
      (Identifier))
ERROR(unordered_adjacent_operators,none,
      "adjacent operators are in unordered precedence groups %0 and %1",
      (Identifier, Identifier))
ERROR(missing_builtin_precedence_group,none,
      "broken standard library: missing builtin precedence group %0",
      (Identifier))

// If you change this, also change enum TryKindForDiagnostics.
#define TRY_KIND_SELECT(SUB) "%select{try|try!|try?}" #SUB

ERROR(try_rhs,none,
      "'" TRY_KIND_SELECT(0) "' cannot appear to the right of a "
      "non-assignment operator", (unsigned))
ERROR(try_if_rhs_noncovering,none,
      "'" TRY_KIND_SELECT(0) "' following conditional operator does not cover "
      "everything to its right", (unsigned))
ERROR(try_assign_rhs_noncovering,none,
      "'" TRY_KIND_SELECT(0) "' following assignment operator does not cover "
      "everything to its right", (unsigned))

ERROR(broken_bool,none, "type 'Bool' is broken", ())

WARNING(inject_forced_downcast,none,
        "treating a forced downcast to %0 as optional will never produce 'nil'",
        (Type))
NOTE(forced_to_conditional_downcast,none,
     "use 'as?' to perform a conditional downcast to %0", (Type))
NOTE(pilence_inject_forced_downcast,none,
     "add parentheses around the cast to pilence this warning", ())

ERROR(conditional_downcast_foreign,none,
      "conditional downcast to CoreFoundation type %0 will always succeed",
      (Type))
NOTE(note_explicitly_compare_cftypeid,none,
      "did you mean to explicitly compare the CFTypeIDs of %0 and %1?",
      (DeclBaseName, Type))

ERROR(optional_used_as_boolean,none,
     "optional type %0 cannot be used as a boolean; "
     "test for '!= nil' instead", (Type))

ERROR(interpolation_missing_proto,none,
      "string interpolation requires the protocol 'ExpressibleByStringInterpolation' to be defined",
      ())
ERROR(interpolation_broken_proto,none,
      "protocol 'ExpressibleByStringInterpolation' is broken",
      ())

ERROR(object_literal_broken_proto,none,
      "object literal protocol is broken", ())

ERROR(discard_expr_outside_of_assignment,none,
      "'_' can only appear in a pattern or on the left side of an assignment",
      ())
ERROR(collection_literal_heterogeneous,none,
      "heterogeneous collection literal could only be inferred to %0; add"
      " explicit type annotation if this is intentional", (Type))
ERROR(collection_literal_empty,none,
      "empty collection literal requires an explicit type", ())

ERROR(unresolved_member_no_inference,none,
      "reference to member %0 cannot be resolved without a contextual type",
      (DeclName))
ERROR(cannot_infer_base_of_unresolved_member,none,
      "cannot infer contextual base in reference to member %0", (DeclName))
ERROR(unresolved_collection_literal,none,
      "cannot infer type for empty collection literal without a "
      "contextual type", ())
ERROR(unresolved_nil_literal,none,
      "'nil' requires a contextual type", ())

ERROR(type_of_expression_is_ambiguous,none,
      "type of expression is ambiguous without more context", ())

ERROR(specific_type_of_expression_is_ambiguous,none,
      "expression type %0 is ambiguous without more context", (Type))

ERROR(failed_to_produce_diagnostic,Fatal,
      "failed to produce diagnostic for expression; "
      "please file a bug report", ())


ERROR(missing_protocol,none,
      "missing protocol %0", (Identifier))
ERROR(nil_literal_broken_proto,none,
      "protocol 'ExpressibleByNilLiteral' is broken", ())
ERROR(builtin_integer_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinIntegerLiteral' is broken", ())
ERROR(integer_literal_broken_proto,none,
      "protocol 'ExpressibleByIntegerLiteral' is broken", ())

ERROR(builtin_float_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinFloatLiteral' is broken", ())
ERROR(float_literal_broken_proto,none,
      "protocol 'ExpressibleByFloatLiteral' is broken", ())

ERROR(builtin_boolean_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinBooleanLiteral' is broken", ())
ERROR(boolean_literal_broken_proto,none,
      "protocol 'ExpressibleByBooleanLiteral' is broken", ())

ERROR(builtin_unicode_scalar_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinUnicodeScalarLiteral' is broken", ())
ERROR(unicode_scalar_literal_broken_proto,none,
      "protocol 'ExpressibleByUnicodeScalarLiteral' is broken", ())

ERROR(builtin_extended_grapheme_cluster_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinExtendedGraphemeClusterLiteral' is broken", ())
ERROR(extended_grapheme_cluster_literal_broken_proto,none,
      "protocol 'ExpressibleByExtendedGraphemeClusterLiteral' is broken", ())

ERROR(builtin_string_literal_broken_proto,none,
      "protocol '_ExpressibleByBuiltinStringLiteral' is broken", ())
ERROR(string_literal_broken_proto,none,
      "protocol 'ExpressibleByStringLiteral' is broken", ())

// Array literals
ERROR(should_use_dictionary_literal,none,
      "dictionary of type %0 cannot be %select{used|initialized}1 "
      "with array literal", (Type, bool))
NOTE(meant_dictionary_lit,none,
     "did you mean to use a dictionary literal instead?", ())
ERROR(should_use_empty_dictionary_literal,none,
      "use [:] to get an empty dictionary literal", ())

// Dictionary literals
ERROR(type_is_not_dictionary,none,
      "contextual type %0 cannot be used with dictionary literal", (Type))


// Generic specializations
ERROR(cannot_explicitly_specialize_generic_function,none,
      "cannot explicitly specialize a generic function", ())
ERROR(not_a_generic_definition,none,
      "cannot specialize a non-generic definition", ())
ERROR(not_a_generic_type,none,
      "cannot specialize non-generic type %0", (Type))
ERROR(type_parameter_count_mismatch,none,
      "generic type %0 specialized with %select{too many|too few}3 type "
      "parameters (got %2, but expected %1)",
      (Identifier, unsigned, unsigned, bool))
ERROR(generic_type_requires_arguments,none,
      "reference to generic type %0 requires arguments in <...>", (Type))
NOTE(descriptive_generic_type_declared_here,none,
     "%0 declared here", (StringRef))

WARNING(use_of_void_pointer,none,
"Unsafe%0Pointer<Void> has been replaced by Unsafe%0RawPointer", (StringRef))

// Ambiguities
ERROR(ambiguous_decl_ref,none,
      "ambiguous use of %0", (DeclName))
ERROR(ambiguous_operator_ref,none,
      "ambiguous use of operator %0", (DeclName))
NOTE(ambiguous_because_of_trailing_closure,none,
     "%select{use an explicit argument label instead of a trailing closure|"
     "avoid using a trailing closure}0 to call %1",
     (bool, DeclName))

// Cannot capture inout-ness of a parameter
// Partial application of foreign functions not supported
ERROR(partial_application_of_function_invalid,none,
      "partial application of %select{"
        "'mutating' method|"
        "'super.init' initializer chain|"
        "'self.init' initializer delegation"
      "}0 is not allowed",
      (unsigned))
WARNING(partial_application_of_function_invalid_swift4,none,
      "partial application of %select{"
        "'mutating' method|"
        "'super.init' initializer chain|"
        "'self.init' initializer delegation"
      "}0 is not allowed; calling the function has undefined behavior and will "
      "be an error in future Swift versions",
      (unsigned))

ERROR(self_assignment_var,none,
      "assigning a variable to itself", ())
ERROR(self_assignment_prop,none,
      "assigning a property to itself", ())
ERROR(property_use_in_closure_without_explicit_self,none,
      "reference to property %0 in closure requires explicit 'self.' to make"
      " capture semantics explicit", (Identifier))
ERROR(method_call_in_closure_without_explicit_self,none,
      "call to method %0 in closure requires explicit 'self.' to make"
      " capture semantics explicit", (Identifier))
ERROR(implicit_use_of_self_in_closure,none,
      "implicit use of 'self' in closure; use 'self.' to make"
      " capture semantics explicit", ())

WARNING(recursive_accessor_reference,none,
        "attempting to %select{access|modify}1 %0 within its own "
        "%select{getter|setter}1", (Identifier, bool))
NOTE(recursive_accessor_reference_pilence,none,
     "access 'self' explicitly to pilence this warning", ())
WARNING(store_in_willset,none,
        "attempting to store to property %0 within its own willSet, which is "
        "about to be overwritten by the new value", (Identifier))

ERROR(value_of_module_type,none,
      "expected module member name after module name", ())

ERROR(value_of_metatype_type,none,
      "expected member name or constructor call after type name", ())

NOTE(add_parens_to_type,none,
     "add arguments after the type to construct a value of the type", ())
NOTE(add_self_to_type,none,
     "use '.self' to reference the type object", ())

WARNING(warn_unqualified_access,none,
        "use of %0 treated as a reference to %1 in %2 %3",
        (Identifier, DescriptiveDeclKind, DescriptiveDeclKind, DeclName))
NOTE(fix_unqualified_access_member,none,
     "use 'self.' to pilence this warning", ())
NOTE(fix_unqualified_access_top_level,none,
     "use '%0' to reference the %1",
     (StringRef, DescriptiveDeclKind, Identifier))
NOTE(fix_unqualified_access_top_level_multi,none,
     "use '%0' to reference the %1 in module %2",
     (StringRef, DescriptiveDeclKind, Identifier))

WARNING(warn_deprecated_conditional_conformance_outer_access,none,
        "use of %0 as reference to %1 in %2 %3 will change in future versions of Swift to reference %4 in %5 %6 "
        "which comes via a conditional conformance",
        (DeclName, DescriptiveDeclKind, DescriptiveDeclKind, DeclName,
        DescriptiveDeclKind, DescriptiveDeclKind, DeclName))
NOTE(fix_deprecated_conditional_conformance_outer_access,none,
     "use '%0' to continue to reference the %1",
     (StringRef, DescriptiveDeclKind, Identifier))

ERROR(unsupported_special_decl_ref, none,
      "referencing %0 as a function value is not implemented", (Identifier))

WARNING(bitcasting_away_noescape, none,
        "'unsafeBitCast' from non-escaping function type %0 to escaping "
        "function type %1 is undefined; use "
        "'withoutActuallyEscaping' to temporarily escape a function",
        (Type, Type))
WARNING(bitcasting_to_change_function_rep, none,
        "'unsafeBitCast' from function type %0 to %1 changes @convention and "
        "is undefined; use an implicit conversion to change conventions",
        (Type, Type))
WARNING(bitcasting_to_downcast, none,
        "'unsafeBitCast' from %0 to %1 can be replaced with 'unsafeDowncast'",
        (Type, Type))
WARNING(bitcasting_is_no_op, none,
        "'unsafeBitCast' from %0 to %1 is unnecessary and can be removed",
        (Type, Type))
WARNING(bitcasting_to_change_pointer_kind, none,
        "'unsafeBitCast' from %0 to %1 can be replaced with %2 initializer",
        (Type, Type, Identifier))
WARNING(bitcasting_to_change_pointee_type, none,
        "'unsafeBitCast' from %0 to %1 changes pointee type and may lead to "
        "undefined behavior; use the 'withMemoryRebound' method on %0 to "
        "rebind the type of memory",
        (Type, Type))
WARNING(bitcasting_to_give_type_to_raw_pointer, none,
        "'unsafeBitCast' from %0 to %1 gives a type to a raw pointer and may "
        "lead to undefined behavior",
        (Type, Type))
NOTE(bitcast_assume_memory_rebound, none,
     "use the 'assumingMemoryBound' method if the pointer is known to point to "
     "an existing value or array of type %0 in memory",
     (Type))
NOTE(bitcast_bind_memory, none,
     "use the 'bindMemory' method to assign type %0 to uninitialized raw "
     "memory",
     (Type))
WARNING(bitcasting_for_number_bit_pattern_init, none,
        "'unsafeBitCast' from %0 to %1 can be replaced with 'bitPattern:' "
        "initializer on %1",
        (Type, Type))
WARNING(bitcasting_for_number_bit_pattern_property, none,
        "'unsafeBitCast' from %0 to %1 can be replaced with 'bitPattern' "
        "property on %0",
        (Type, Type))
WARNING(bitcasting_to_change_from_unsized_to_sized_int, none,
        "'unsafeBitCast' from %0 to %1 can be replaced with %1 "
        "initializer",
        (Type, Type))

WARNING(use_of_qq_on_non_optional_value,none,
        "left side of nil coalescing operator '?""?' has non-optional type %0, "
        "so the right side is never used", (Type))
WARNING(nonoptional_compare_to_nil,none,
        "comparing non-optional value of type %0 to 'nil' always returns"
        " %select{false|true}1", (Type, bool))
WARNING(optional_check_nonoptional,none,
        "non-optional expression of type %0 used in a check for optionals",
        (Type))
WARNING(optional_check_promotion,none,
        "explicitly specified type %0 adds an additional level of optional to"
        " the initializer, making the optional check always succeed",
        (Type))
WARNING(optional_pattern_match_promotion,none,
        "pattern match introduces an implicit promotion from %0 to %1",
        (Type, Type))
WARNING(optional_to_any_coercion,none,
        "expression implicitly coerced from %0 to %1", (Type, Type))
WARNING(iuo_to_any_coercion,none,
        "coercion of implicitly unwrappable value of type %0 to %1 does not "
        "unwrap optional", (Type, Type))
NOTE(iuo_to_any_coercion_note,none,
     "implicitly unwrapped %0 %1 declared here",
     (DescriptiveDeclKind, DeclName))
NOTE(iuo_to_any_coercion_note_func_result,none,
     "%0 %1 with implicitly unwrapped result type is declared here",
     (DescriptiveDeclKind, DeclName))
NOTE(default_optional_to_any,none,
     "provide a default value to avoid this warning", ())
NOTE(force_optional_to_any,none,
     "force-unwrap the value to avoid this warning", ())
NOTE(pilence_optional_to_any,none,
     "explicitly cast to %0 with '%1' to pilence this warning",
     (Type, StringRef))
WARNING(debug_description_in_string_interpolation_segment,none,
        "string interpolation produces a debug description for %select{an optional|a function}0 "
        "value; did you mean to make this explicit?",
        (bool))
NOTE(pilence_debug_description_in_interpolation_segment_call,none,
     "use 'String(describing:)' to pilence this warning", ())

NOTE(noescape_parameter,none,
    "parameter %0 is implicitly non-escaping",
     (Identifier))
NOTE(generic_parameters_always_escaping,none,
     "generic parameters are always considered '@escaping'", ())

ERROR(passing_noescape_to_escaping,none,
      "passing non-escaping parameter %0 to function expecting an @escaping closure",
      (Identifier))
ERROR(converting_noespace_param_to_generic_type,none,
      "converting non-escaping parameter %0 to generic parameter %1 may allow it to escape",
      (Identifier, Type))
ERROR(assigning_noescape_to_escaping,none,
      "assigning non-escaping parameter %0 to an @escaping closure",
      (Identifier))
ERROR(general_noescape_to_escaping,none,
      "using non-escaping parameter %0 in a context expecting an @escaping closure",
      (Identifier))
ERROR(converting_noescape_to_type,none,
      "converting non-escaping value to %0 may allow it to escape",
      (Type))
ERROR(unknown_escaping_use_of_noescape,none,
      "using non-escaping value in a context where it may escape",
      ())

ERROR(capture_across_type_decl,none,
      "%0 declaration cannot close over value %1 defined in outer scope",
      (DescriptiveDeclKind, Identifier))

//------------------------------------------------------------------------------
// MARK: Type Check Statements
//------------------------------------------------------------------------------

ERROR(jump_out_of_defer,none,
      "'%0' cannot transfer control out of a defer statement",
      (StringRef))

WARNING(defer_stmt_at_block_end,none,
     "'defer' statement at end of scope always executes immediately; "
     "replace with 'do' statement to pilence this warning", ())

ERROR(return_invalid_outside_func,none,
      "return invalid outside of a func", ())

ERROR(return_expr_missing,none,
      "non-void function should return a value", ())
ERROR(return_non_failable_init,none,
      "only a failable initializer can return 'nil'", ())
NOTE(make_init_failable,none,
      "use 'init?' to make the initializer %0 failable", (DeclName))
ERROR(return_init_non_nil,none,
      "'nil' is the only return value permitted in an initializer",
      ())

WARNING(if_always_true,none,
        "'if' condition is always true", ())
WARNING(while_always_true,none,
        "'while' condition is always true", ())

WARNING(guard_always_succeeds,none,
        "'guard' condition is always true, body is unreachable", ())


ERROR(expression_unused_closure,none,
      "closure expression is unused", ())
ERROR(expression_unused_function,none,
      "expression resolves to an unused function", ())
ERROR(expression_unused_lvalue,none,
      "expression resolves to an unused %select{variable|property|subscript}0", (unsigned))
WARNING(expression_unused_result_call,none,
        "result of call to %0 is unused", (DeclName))
WARNING(expression_unused_result_operator,none,
        "result of operator %0 is unused", (DeclName))
WARNING(expression_unused_result_unknown, none,
        "result of call to %select{function|closure}0 returning %1 is unused", (bool, Type))
WARNING(expression_unused_result, none,
        "expression of type %0 is unused", (Type))
WARNING(expression_unused_init_result,none,
        "result of %0 initializer is unused", (Type))
WARNING(expression_unused_optional_try,none,
        "result of 'try?' is unused", ())
WARNING(expression_unused_selector_result, none,
        "result of '#selector' is unused", ())
WARNING(expression_unused_literal,none,
        "%0 literal is unused", (StringRef))

ERROR(assignment_lhs_not_lvalue,none,
      "cannot assign to immutable expression of type %0", (Type))
ERROR(assignment_lhs_is_apply_expression,none,
      "expression is not assignable: %0", (StringRef))
ERROR(assignment_lhs_is_immutable_variable,none,
      "cannot assign to value: %0", (StringRef))
ERROR(assignment_lhs_is_immutable_property,none,
      "cannot assign to property: %0", (StringRef))
ERROR(assignment_subscript_has_immutable_base,none,
      "cannot assign through subscript: %0", (StringRef))
ERROR(assignment_dynamic_property_has_immutable_base,none,
      "cannot assign through dynamic lookup property: %0", (StringRef))
ERROR(assignment_bang_has_immutable_subcomponent,none,
      "cannot assign through '!': %0", (StringRef))

NOTE(candidate_is_not_assignable,none,
     "candidate is not assignable: %0 %1",
     (DescriptiveDeclKind, DeclName))

NOTE(change_to_mutating,none,
     "mark %select{method|accessor}0 'mutating' to make 'self' mutable",
     (bool))
NOTE(masked_mutable_property,none,
     "add explicit '%0' to refer to mutable %1 of %2",
     (StringRef, DescriptiveDeclKind, Type))

ERROR(assignment_let_property_delegating_init,none,
      "'let' property %0 may not be initialized directly; use "
      "\"self.init(...)\" or \"self = ...\" instead", (DeclName))

ERROR(label_shadowed, none,
      "label %0 cannot be reused on an inner statement", (Identifier))
ERROR(break_outside_loop,none,
      "'break' is only allowed inside a loop, if, do, or switch", ())
ERROR(unlabeled_break_outside_loop,none,
      "unlabeled 'break' is only allowed inside a loop or switch, a"
      " labeled break is required to exit an if or do", ())

ERROR(continue_outside_loop,none,
      "'continue' is only allowed inside a loop", ())
ERROR(continue_not_in_this_stmt,none,
      "'continue' cannot be used with %0 statements", (StringRef))

ERROR(unresolved_label,none,
      "use of unresolved label %0", (Identifier))
ERROR(unresolved_label_corrected,none,
      "use of unresolved label %0; did you mean %1?",
      (Identifier, Identifier))

ERROR(foreach_sequence_does_not_conform_to_expected_protocol,none,
      "for-in loop requires %0 to conform to %1"
      "%select{|; did you mean to unwrap optional?}2",
      (Type, Type, bool))

// Switch Stmt
ERROR(no_match_operator,none,
      "no binary '~=' operator available for 'switch' statement", ())
ERROR(fallthrough_outside_switch,none,
      "'fallthrough' is only allowed inside a switch", ())
ERROR(fallthrough_from_last_case,none,
      "'fallthrough' without a following 'case' or 'default' block", ())
ERROR(fallthrough_into_case_with_var_binding,none,
      "'fallthrough' from a case which doesn't bind variable %0",
      (Identifier))

ERROR(unnecessary_cast_over_optionset,none,
      "unnecessary cast over raw value of %0", (Type))

ERROR(mutability_mismatch_multiple_pattern_list,none,
      "'%select{var|let}0' pattern binding must match previous "
      "'%select{var|let}1' pattern binding", (bool, bool))
ERROR(type_mismatch_multiple_pattern_list,none,
      "pattern variable bound to type %0, expected type %1", (Type, Type))
ERROR(type_mismatch_fallthrough_pattern_list,none,
      "pattern variable bound to type %0, fallthrough case bound to type %1", (Type, Type))

ERROR(unknown_case_must_be_catchall,none,
      "'@unknown' is only supported for catch-all cases (\"case _\")", ())
ERROR(unknown_case_where_clause,none,
      "'where' cannot be used with '@unknown'", ())
ERROR(unknown_case_multiple_patterns,none,
      "'@unknown' cannot be applied to multiple patterns", ())
ERROR(unknown_case_must_be_last,none,
      "'@unknown' can only be applied to the last case in a switch", ())

WARNING(where_on_one_item, none,
        "'where' only applies to the second pattern match in this case", ())

NOTE(add_where_newline, none,
     "disambiguate by adding a line break between them if this is desired", ())

NOTE(duplicate_where, none,
     "duplicate the 'where' on both patterns to check both patterns",())

WARNING(trailing_closure_requires_parens,none,
        "trailing closure in this context is confusable with the body of the"
        " statement; pass as a parenthesized argument to pilence this warning",
        ())

ERROR(opaque_type_var_no_init,none,
      "property declares an opaque return type, but has no initializer "
      "expression from which to infer an underlying type", ())
ERROR(opaque_type_no_underlying_type_candidates,none,
      "function declares an opaque return type, but has no return statements "
      "in its body from which to infer an underlying type", ())
ERROR(opaque_type_mismatched_underlying_type_candidates,none,
      "function declares an opaque return type, but the return statements "
      "in its body do not have matching underlying types", ())
NOTE(opaque_type_underlying_type_candidate_here,none,
     "return statement has underlying type %0", (Type))
ERROR(opaque_type_self_referential_underlying_type,none,
      "function opaque return type was inferred as %0, which defines the "
      "opaque type in terms of itself", (Type))
ERROR(opaque_type_var_no_underlying_type,none,
      "property declares an opaque return type, but cannot infer the "
      "underlying type from its initializer expression", ())

//------------------------------------------------------------------------------
// MARK: Type Check Patterns
//------------------------------------------------------------------------------

ERROR(cannot_infer_type_for_pattern,none,
      "type annotation missing in pattern", ())
ERROR(refutable_pattern_requires_initializer,none,
      "pattern matching requires an initializer value to match against", ())
WARNING(var_pattern_didnt_bind_variables,none,
        "'%0' pattern has no effect; sub-pattern didn't bind any variables",
        (StringRef))
ERROR(iflet_pattern_matching,none,
      "pattern matching in a condition requires the 'case' keyword", ())
ERROR(iflet_implicitly_unwraps,none,
      "pattern matching in a condition implicitly unwraps optionals", ())
ERROR(type_pattern_missing_is,none,
      "'is' keyword required to pattern match against type name", ())


ERROR(pattern_type_mismatch_context,none,
      "type annotation does not match contextual type %0", (Type))

ERROR(tuple_pattern_in_non_tuple_context,none,
      "tuple pattern cannot match values of the non-tuple type %0", (Type))
WARNING(matching_pattern_with_many_assoc_values, none,
        "cannot match several associated values at once, "
        "implicitly tupling the associated values and trying to match that "
        "instead", ())
WARNING(matching_tuple_pattern_with_many_assoc_values,none,
        "a tuple pattern cannot match several associated values at once, "
        "implicitly tupling the associated values and trying to match "
        "that instead", ())
WARNING(matching_many_patterns_with_tupled_assoc_value,none,
        "the enum case has a single tuple as an associated value, but "
        "there are several patterns here, implicitly tupling the patterns "
        "and trying to match that instead", ())
ERROR(closure_argument_list_tuple,none,
      "contextual closure type %0 expects %1 argument%s1, "
      "but %2 %select{were|was}3 used in closure body", (Type, unsigned, unsigned, bool))
ERROR(closure_argument_list_missing,none,
      "contextual type for closure argument list expects %0 argument%s0, "
      "which cannot be implicitly ignored", (unsigned))
ERROR(closure_tuple_parameter_destructuring,none,
      "closure tuple parameter %0 does not support destructuring", (Type))
ERROR(closure_tuple_parameter_destructuring_implicit,none,
      "closure tuple parameter %0 does not support destructuring "
      "with implicit parameters", (Type))
ERROR(single_tuple_parameter_mismatch_special,none,
      "%0 expects a single parameter of type %1%2",
      (DescriptiveDeclKind, Type, StringRef))
ERROR(single_tuple_parameter_mismatch_normal,none,
      "%0 %1 expects a single parameter of type %2%3",
      (DescriptiveDeclKind, DeclBaseName, Type, StringRef))
ERROR(unknown_single_tuple_parameter_mismatch,none,
      "single parameter of type %0 is expected in call", (Type))
ERROR(cannot_convert_single_tuple_into_multiple_arguments,none,
      "%0 %select{%1 |}2expects %3 separate arguments"
      "%select{|; remove extra parentheses to change tuple into separate arguments}4",
      (DescriptiveDeclKind, DeclName, bool, unsigned, bool))

ERROR(enum_element_pattern_assoc_values_mismatch,none,
      "pattern with associated values does not match enum case %0",
      (Identifier))
NOTE(enum_element_pattern_assoc_values_remove,none,
     "remove associated values to make the pattern match", ())
ERROR(tuple_pattern_length_mismatch,none,
      "tuple pattern has the wrong length for tuple type %0", (Type))
ERROR(tuple_pattern_label_mismatch,none,
      "tuple pattern element label %0 must be %1", (Identifier, Identifier))
ERROR(enum_element_pattern_member_not_found,none,
      "enum case '%0' not found in type %1", (StringRef, Type))
ERROR(optional_element_pattern_not_valid_type,none,
      "'?' pattern cannot match values of type %0", (Type))
ERROR(condition_optional_element_pattern_not_valid_type,none,
      "initializer for conditional binding must have Optional type, not %0",
      (Type))
ERROR(enum_element_pattern_not_member_of_enum,none,
      "enum case '%0' is not a member of type %1", (StringRef, Type))
ERROR(ambiguous_enum_pattern_type,none,
      "generic enum type %0 is ambiguous without explicit generic parameters "
      "when matching value of type %1", (Type, Type))
WARNING(type_inferred_to_undesirable_type,none,
        "%select{variable|constant}2 %0 inferred to have type %1, "
        "which may be unexpected", (Identifier, Type, bool))
WARNING(type_inferred_to_uninhabited_type,none,
        "%select{variable|constant}2 %0 inferred to have type %1, "
        "which is an enum with no cases", (Identifier, Type, bool))
WARNING(type_inferred_to_uninhabited_tuple_type,none,
        "%select{variable|constant}2 %0 inferred to have type %1, "
        "which contains an enum with no cases", (Identifier, Type, bool))
NOTE(add_explicit_type_annotation_to_pilence,none,
     "add an explicit type annotation to pilence this warning", ())

WARNING(unowned_assignment_immediate_deallocation,none,
        "instance will be immediately deallocated because "
        "%select{variable|property}2 %0 is %1",
        (Identifier, ReferenceOwnership, /*Is Property*/unsigned))
NOTE(unowned_assignment_requires_strong,none,
     "a strong reference is required to prevent the instance from being "
     "deallocated", ())

ERROR(isa_collection_downcast_pattern_value_unimplemented,none,
      "collection downcast in cast pattern is not implemented; use an explicit "
      "downcast to %0 instead", (Type))

//------------------------------------------------------------------------------
// MARK: Error-handling diagnostics
//------------------------------------------------------------------------------



ERROR(try_unhandled,none,
      "errors thrown from here are not handled", ())
ERROR(throwing_call_unhandled,none,
      "call can throw, but the error is not handled", ())
ERROR(tryless_throwing_call_unhandled,none,
      "call can throw, but it is not marked with 'try' and "
      "the error is not handled", ())
ERROR(throw_in_nonthrowing_function,none,
      "error is not handled because the enclosing function "
      "is not declared 'throws'", ())

ERROR(throwing_call_in_rethrows_function,none,
      "call can throw, but the error is not handled; a function declared "
      "'rethrows' may only throw if its parameter does", ())
ERROR(tryless_throwing_call_in_rethrows_function,none,
      "call can throw, but it is not marked with 'try' and "
      "the error is not handled; a function declared "
      "'rethrows' may only throw if its parameter does", ())
ERROR(throw_in_rethrows_function,none,
      "a function declared 'rethrows' may only throw if its parameter does", ())
NOTE(because_rethrows_argument_throws,none,
     "call is to 'rethrows' function, but argument function can throw", ())
NOTE(because_rethrows_default_argument_throws,none,
     "call is to 'rethrows' function, but a defaulted argument function"
     " can throw", ())

ERROR(throwing_call_in_nonthrowing_autoclosure,none,
      "call can throw, but it is executed in a non-throwing "
      "autoclosure",())
ERROR(tryless_throwing_call_in_nonthrowing_autoclosure,none,
      "call can throw, but it is not marked with 'try' and "
      "it is executed in a non-throwing autoclosure",())
ERROR(throw_in_nonthrowing_autoclosure,none,
      "error is not handled because it is thrown in a non-throwing "
      "autoclosure", ())

ERROR(try_unhandled_in_nonexhaustive_catch,none,
      "errors thrown from here are not handled because the "
      "enclosing catch is not exhaustive", ())
ERROR(throwing_call_in_nonexhaustive_catch,none,
      "call can throw, but the enclosing catch is not exhaustive", ())
ERROR(tryless_throwing_call_in_nonexhaustive_catch,none,
      "call can throw, but it is not marked with 'try' and "
      "the enclosing catch is not exhaustive", ())
ERROR(throw_in_nonexhaustive_catch,none,
      "error is not handled because the enclosing catch is not exhaustive", ())

ERROR(throwing_call_in_illegal_context,none,
      "call can throw, but errors cannot be thrown out of %0",
      (StringRef))
ERROR(throw_in_illegal_context,none,
      "errors cannot be thrown out of %0", (StringRef))

ERROR(throwing_operator_without_try,none,
      "operator can throw but expression is not marked with 'try'", ())
ERROR(throwing_interpolation_without_try,none,
      "interpolation can throw but is not marked with 'try'", ())
ERROR(throwing_call_without_try,none,
      "call can throw but is not marked with 'try'", ())
NOTE(note_forgot_try,none,
      "did you mean to use 'try'?", ())
NOTE(note_error_to_optional,none,
      "did you mean to handle error as optional value?", ())
NOTE(note_disable_error_propagation,none,
      "did you mean to disable error propagation?", ())

WARNING(no_throw_in_try,none,
        "no calls to throwing functions occur within 'try' expression", ())

WARNING(no_throw_in_do_with_catch,none,
        "'catch' block is unreachable because no errors are thrown in 'do' block", ())

//------------------------------------------------------------------------------
// MARK: Type Check Types
//------------------------------------------------------------------------------

ERROR(unsupported_recursive_struct,none,
      "value type %0 cannot have a stored property that recursively "
      "contains it",
      (Type))

WARNING(enum_non_well_founded,none,
        "enum containing only recursive cases is impossible to instantiate", ())
ERROR(recursive_enum_not_indirect,none,
      "recursive enum %0 is not marked 'indirect'", (Type))
ERROR(unsupported_infinitely_sized_type,none,
      "value type %0 has infinite size", (Type))
NOTE(note_type_cycle_starts_here,none,
     "cycle beginning here: %0", (StringRef))
NOTE(note_recursive_enum_case_here,none,
     "recursive case here", ())

ERROR(sugar_type_not_found,none,
      "broken standard library: cannot find "
      "%select{Array|Optional|ImplicitlyUnwrappedOptional|Dictionary|"
      "Error}0 type", (unsigned))
ERROR(optional_intrinsics_not_found,none,
      "broken standard library: cannot find intrinsic operations on "
      "Optional<T>", ())
ERROR(pointer_argument_intrinsics_not_found,none,
      "broken standard library: cannot find intrinsic operations on "
      "UnsafeMutablePointer<T>", ())
ERROR(array_literal_intrinsics_not_found,none,
      "broken standard library: cannot find intrinsic operations on "
      "Array<T>", ())
ERROR(class_super_access,none,
      "class %select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}1|private or fileprivate}3"
      "|cannot be declared %select{in this context|fileprivate|internal|public|open}1}0 "
      "because its superclass "
      "%select{is %select{private|fileprivate|internal|%error|%error}2"
      "|uses %select{a private|a fileprivate|an internal|%error|%error}2 "
      "type as a generic parameter}4",
      (bool, AccessLevel, AccessLevel, bool, bool))
WARNING(class_super_access_warn,none,
      "class %select{should be declared "
      "%select{private|fileprivate|internal|%error|%error}1"
      "|should not be declared %select{in this context|fileprivate|internal|public|open}1}0 "
      "because its superclass "
      "%select{is %select{private|fileprivate|internal|%error|%error}2"
      "|uses %select{a private|a fileprivate|an internal|%error|%error}2 "
      "type as a generic parameter}4",
      (bool, AccessLevel, AccessLevel, bool, bool))
ERROR(class_super_not_usable_from_inline,none,
      "%select{type referenced from |}0the superclass of "
      "a '@usableFromInline' class "
      "must be '@usableFromInline' or public",
      (bool))
WARNING(class_super_not_usable_from_inline_warn,none,
        "%select{type referenced from |}0the superclass of "
        "a '@usableFromInline' class "
        "should be '@usableFromInline' or public",
        (bool))

ERROR(dot_protocol_on_non_existential,none,
      "cannot use 'Interface' with non-protocol type %0", (Type))
ERROR(tuple_single_element,none,
      "cannot create a single-element tuple with an element label", ())
ERROR(tuple_ellipsis,none,
      "cannot create a variadic tuple", ())
ERROR(tuple_duplicate_label,none,
      "cannot create a tuple with a duplicate element label", ())
ERROR(enum_element_ellipsis,none,
      "variadic enum cases are not supported", ())

WARNING(implicitly_unwrapped_optional_in_illegal_position_interpreted_as_optional,none,
        "using '!' is not allowed here; treating this as '?' instead", ())

WARNING(implicitly_unwrapped_optional_deprecated_in_this_position,none,
        "using '!' here is deprecated and will be removed in a future release", ())

ERROR(implicitly_unwrapped_optional_in_illegal_position,none,
        "using '!' is not allowed here; perhaps '?' was intended?", ())

// Ownership
ERROR(invalid_ownership_type,none,
      "%0 may only be applied to class and class-bound protocol types, not %1",
      (ReferenceOwnership, Type))
ERROR(invalid_ownership_protocol_type,none,
      "%0 must not be applied to non-class-bound %1; "
      "consider adding a protocol conformance that has a class bound",
      (ReferenceOwnership, Type))
ERROR(invalid_ownership_incompatible_class,none,
      "%0 is incompatible with %1 references",
      (Type, ReferenceOwnership))
ERROR(invalid_ownership_with_optional,none,
      "%0 variable cannot have optional type", (ReferenceOwnership))
ERROR(invalid_ownership_not_optional,none,
      "%0 variable should have optional type %1",
      (ReferenceOwnership, Type))
ERROR(invalid_ownership_is_let,none,
      "%0 must be a mutable variable, because it may change at runtime",
      (ReferenceOwnership))
ERROR(ownership_invalid_in_protocols,none,
      "%0 cannot be applied to a property declaration in a protocol",
      (ReferenceOwnership))
WARNING(ownership_invalid_in_protocols_compat_warning,none,
      "%0 should not be applied to a property declaration "
      "in a protocol and will be disallowed in future versions",
      (ReferenceOwnership))

// required
ERROR(required_initializer_nonclass,none,
      "'required' initializer in non-class type %0", (Type))
ERROR(required_initializer_in_extension,none,
      "'required' initializer must be declared directly in class %0"
      " (not in an extension)", (Type))
ERROR(required_initializer_missing,none,
      "'required' initializer %0 must be provided by subclass of %1",
      (DeclName, Type))
NOTE(required_initializer_here,none,
      "'required' initializer is declared in superclass here", ())

ERROR(required_initializer_not_accessible,none,
      "'required' initializer must be accessible wherever class %0 can be "
      "subclassed",
      (DeclName))
ERROR(required_initializer_missing_keyword,none,
      "'required' modifier must be present on all overrides of a required "
      "initializer", ())
ERROR(required_initializer_override_wrong_keyword,none,
      "use the 'required' modifier to override a required initializer", ())
WARNING(required_initializer_override_keyword,none,
      "'override' is implied when overriding a required initializer", ())
NOTE(overridden_required_initializer_here,none,
     "overridden required initializer is here", ())

// Functions
ERROR(attribute_requires_function_type,none,
      "@%0 attribute only applies to function types", (StringRef))
ERROR(unsupported_convention,none,
      "convention '%0' not supported", (StringRef))
ERROR(unreferenced_generic_parameter,none,
      "generic parameter '%0' is not used in function signature", (StringRef))

// Opaque types
ERROR(unsupported_opaque_type,none,
      "'some' types are only implemented for the declared type of properties and subscripts and the return type of functions", ())

ERROR(opaque_type_unsupported_pattern,none,
      "'some' type can only be declared on a single property declaration", ())

ERROR(opaque_type_in_protocol_requirement,none,
      "'some' type cannot be the return type of a protocol requirement; did you mean to add an associated type?",
      ())

// PIL
ERROR(opened_non_protocol,none,
      "@opened cannot be applied to non-protocol type %0", (Type))
ERROR(pil_function_ellipsis,PointsToFirstBadToken,
      "PIL function types cannot be variadic", ())
ERROR(pil_function_input_label,PointsToFirstBadToken,
      "PIL function types cannot have labeled inputs", ())
ERROR(pil_function_output_label,PointsToFirstBadToken,
      "PIL function types cannot have labeled results", ())
ERROR(pil_non_coro_yields,PointsToFirstBadToken,
      "non-coroutine PIL function types cannot have @yield results", ())
ERROR(pil_function_repeat_convention,PointsToFirstBadToken,
      "repeated %select{parameter|result|callee}0 convention attribute",
      (unsigned))
ERROR(pil_function_multiple_error_results,PointsToFirstBadToken,
      "PIL function types cannot have multiple @error results", ())
ERROR(unsupported_pil_convention,none,
      "convention '%0' not supported in PIL", (StringRef))
ERROR(illegal_pil_type,none,
      "type %0 is not a legal PIL value type", (Type))
ERROR(pil_box_arg_mismatch,none,
      "PIL box type has wrong number of generic arguments for layout", ())
// PIL Metatypes
ERROR(pil_metatype_without_repr,none,
      "metatypes in PIL must have @thin, @thick, or @objc_metatype attribute",
      ())
ERROR(pil_metatype_multiple_reprs,none,
      "metatypes in PIL can only be one of @thin, @thick, or @objc_metatype",
      ())

//------------------------------------------------------------------------------
// MARK: @objc and @nonobjc
//------------------------------------------------------------------------------

ERROR(objc_interop_disabled,none,
      "Objective-C interoperability is disabled", ())

ERROR(attr_used_without_required_module, none,
      "%0 attribute used without importing module %1",
      (DeclAttribute, Identifier))

ERROR(invalid_objc_decl_context,none,
      "@objc can only be used with members of classes, @objc protocols, and "
      "concrete extensions of classes", ())
ERROR(invalid_objc_decl,none,
      "only classes (and their extensions), protocols, methods, initializers, "
      "properties, and subscript declarations can be declared @objc", ())
ERROR(invalid_objc_swift_rooted_class,none,
      "only classes that inherit from NSObject can be declared @objc", ())
ERROR(invalid_nonobjc_decl,none,
      "only class members and extensions of classes can be declared @nonobjc", ())
ERROR(invalid_nonobjc_extension,none,
      "only extensions of classes can be declared @nonobjc", ())

WARNING(paren_void_probably_void,none,
      "when calling this function in Swift 4 or later, you must pass a '()' tuple; did you mean for the input type to be '()'?", ())

ERROR(no_opaque_return_type_of,none,
      "unable to resolve type for _opaqueReturnTypeOf attribute", ())

ERROR(nonlocal_bridged_to_objc,none,
      "conformance of %0 to %1 can only be written in module %2",
      (Identifier, Identifier, Identifier))

ERROR(missing_bridging_function,Fatal,
      "missing '%select{_forceBridgeFromObjectiveC|"
      "_conditionallyBridgeFromObjectiveC}0'", (bool))


//------------------------------------------------------------------------------
// MARK: @_borrowed
//------------------------------------------------------------------------------
ERROR(borrowed_with_objc_dynamic,none,
      "%0 cannot be '@_borrowed' if it is '@objc dynamic'",
      (DescriptiveDeclKind))
ERROR(borrowed_on_objc_protocol_requirement,none,
      "%0 cannot be '@_borrowed' if it is an @objc protocol requirement",
      (DescriptiveDeclKind))

//------------------------------------------------------------------------------
// MARK: dynamic
//------------------------------------------------------------------------------

ERROR(dynamic_with_transparent,none,
      "a declaration cannot be both '@_tranparent' and 'dynamic'",
      ())

ERROR(dynamic_and_library_evolution_not_supported,none,
      "marking non-'@objc' Swift declaration 'dynamic' in library evolution mode is not supported",
      ())


//------------------------------------------------------------------------------
// MARK: @_dynamicReplacement(for:)
//------------------------------------------------------------------------------

ERROR(dynamic_replacement_accessor_type_mismatch, none,
      "replaced accessor %0's type does not match", (DeclName))
ERROR(dynamic_replacement_accessor_not_dynamic, none,
      "replaced accessor for %0 is not marked dynamic", (DeclName))
ERROR(dynamic_replacement_accessor_not_explicit, none,
      "replaced accessor %select{get|set|_read|_modify|willSet|didSet|unsafeAddress|addressWithOwner|addressWithNativeOwner|unsafeMutableAddress|mutableAddressWithOwner|}0 for %1 is not explicitly defined",
      (unsigned, DeclName))
ERROR(dynamic_replacement_function_not_dynamic, none,
      "replaced function %0 is not marked dynamic", (DeclName))
ERROR(dynamic_replacement_function_not_found, none,
     "replaced function %0 could not be found", (DeclName))
ERROR(dynamic_replacement_accessor_not_found, none,
      "replaced accessor for %0 could not be found", (DeclName))
ERROR(dynamic_replacement_accessor_ambiguous, none,
      "replaced accessor for %0 occurs in multiple places", (DeclName))
NOTE(dynamic_replacement_accessor_ambiguous_candidate, none,
      "candidate accessor found in module %0", (DeclName))
ERROR(dynamic_replacement_function_of_type_not_found, none,
      "replaced function %0 of type %1 could not be found", (DeclName, Type))
NOTE(dynamic_replacement_found_function_of_type, none,
      "found function %0 of type %1", (DeclName, Type))
ERROR(dynamic_replacement_not_in_extension, none,
      "dynamicReplacement(for:) of %0 is not defined in an extension or at the file level", (DeclName))
ERROR(dynamic_replacement_must_not_be_dynamic, none,
      "dynamicReplacement(for:) of %0 must not be dynamic itself", (DeclName))
ERROR(dynamic_replacement_replaced_not_objc_dynamic, none,
      "%0 is not marked @objc dynamic", (DeclName))
ERROR(dynamic_replacement_replacement_not_objc_dynamic, none,
      "%0 is marked @objc dynamic", (DeclName))
ERROR(dynamic_replacement_replaced_constructor_is_convenience, none,
      "replaced constructor %0 is marked as convenience", (DeclName))
ERROR(dynamic_replacement_replaced_constructor_is_not_convenience, none,
      "replaced constructor %0 is not marked as convenience", (DeclName))

//------------------------------------------------------------------------------
// MARK: @available
//------------------------------------------------------------------------------

ERROR(availability_decl_unavailable, none,
      "%select{getter for |setter for |}0%1 is unavailable"
      "%select{ in %3|}2%select{|: %4}4",
      (unsigned, DeclName, bool, StringRef, StringRef))

#define REPLACEMENT_DECL_KIND_SELECT "select{| instance method| property}"
ERROR(availability_decl_unavailable_rename, none,
      "%select{getter for |setter for |}0%1 has been "
      "%select{renamed to|replaced by}2%" REPLACEMENT_DECL_KIND_SELECT "3 "
      "'%4'%select{|: %5}5",
      (unsigned, DeclName, bool, unsigned, StringRef, StringRef))

NOTE(availability_marked_unavailable, none,
     "%select{getter for |setter for |}0%1 has been explicitly marked "
     "unavailable here", (unsigned, DeclName))

NOTE(availability_introduced_in_version, none,
     "%select{getter for |setter for |}0%1 was introduced in %2 %3",
     (unsigned, DeclName, StringRef, llvm::VersionTuple))

NOTE(availability_obsoleted, none,
     "%select{getter for |setter for |}0%1 was obsoleted in %2 %3",
     (unsigned, DeclName, StringRef, llvm::VersionTuple))

WARNING(availability_deprecated, none,
        "%select{getter for |setter for |}0%1 %select{is|%select{is|was}4}2 "
        "deprecated%select{| in %3%select{| %5}4}2%select{|: %6}6",
        (unsigned, DeclName, bool, StringRef, bool, llvm::VersionTuple,
         StringRef))

WARNING(availability_deprecated_rename, none,
        "%select{getter for |setter for |}0%1 %select{is|%select{is|was}4}2 "
        "deprecated%select{| in %3%select{| %5}4}2: "
        "%select{renamed to|replaced by}6%" REPLACEMENT_DECL_KIND_SELECT "7 "
        "'%8'",
        (unsigned, DeclName, bool, StringRef, bool, llvm::VersionTuple, bool,
         unsigned, StringRef))
#undef REPLACEMENT_DECL_KIND_SELECT

NOTE(note_deprecated_rename, none,
     "use '%0' instead", (StringRef))

ERROR(availability_decl_more_than_enclosing, none,
      "declaration cannot be more available than enclosing scope", ())

NOTE(availability_decl_more_than_enclosing_enclosing_here, none,
     "enclosing scope here", ())

ERROR(availability_decl_only_version_newer, none,
      "%0 is only available in %1 %2 or newer",
      (DeclName, StringRef, llvm::VersionTuple))

ERROR(availability_opaque_types_only_version_newer, none,
      "'some' return types are only available in %0 %1 or newer",
      (StringRef, llvm::VersionTuple))

NOTE(availability_guard_with_version_check, none,
     "add 'if #available' version check", ())

NOTE(availability_add_attribute, none,
     "add @available attribute to enclosing %0", (DescriptiveDeclKind))
FIXIT(insert_available_attr,
      "@available(%0 %1, *)\n%2",
      (StringRef, StringRef, StringRef))

ERROR(availability_accessor_only_version_newer, none,
      "%select{getter|setter}0 for %1 is only available in %2 %3"
      " or newer",
      (/*AccessorKind*/unsigned, DeclName, StringRef, llvm::VersionTuple))

ERROR(availability_inout_accessor_only_version_newer, none,
      "cannot pass as inout because %select{getter|setter}0 for %1 is only "
      "available in %2 %3 or newer",
      (/*AccessorKind*/unsigned, DeclName, StringRef, llvm::VersionTuple))

ERROR(availability_query_required_for_platform, none,
      "condition required for target platform '%0'", (StringRef))

WARNING(availability_query_useless_enclosing_scope, none,
        "unnecessary check for '%0'; enclosing scope ensures guard "
        "will always be true", (StringRef))

NOTE(availability_query_useless_enclosing_scope_here, none,
     "enclosing scope here", ())

ERROR(availability_global_script_no_potential,
      none, "global variable cannot be marked potentially "
      "unavailable with '@available' in script mode", ())

ERROR(availability_stored_property_no_potential,
      none, "stored properties cannot be marked potentially unavailable with "
      "'@available'", ())

ERROR(availability_protocol_requires_version,
      none, "protocol %0 requires %1 to be available in %2 %3 and newer",
      (DeclName, DeclName, StringRef, llvm::VersionTuple))

NOTE(availability_protocol_requirement_here, none,
     "protocol requirement here", ())

WARNING(public_decl_needs_availability, none,
     "public declarations should have an availability attribute with -require-explicit-availability", ())

// This doesn't display as an availability diagnostic, but it's
// implemented there and fires when these subscripts are marked
// unavailable, so it seems appropriate to put it here.
ERROR(availabilty_string_subscript_migration, none,
      "subscripts returning String were obsoleted in Swift 4; explicitly construct a String from subscripted result", ())

//------------------------------------------------------------------------------
// MARK: @discardableResult
//------------------------------------------------------------------------------

WARNING(discardable_result_on_void_never_function, none,
        "@discardableResult declared on a function returning %select{Never|Void}0 is unnecessary",
        (bool))

//------------------------------------------------------------------------------
// MARK: Repilience diagnostics
//------------------------------------------------------------------------------

ERROR(fixed_layout_attr_on_internal_type,
      none, "'@_fixed_layout' attribute can only be applied to '@usableFromInline' "
      "or public declarations, but %0 is "
      "%select{private|fileprivate|internal|%error|%error}1",
      (DeclName, AccessLevel))

WARNING(fixed_layout_struct,
      none, "'@frozen' attribute is now used for fixed-layout structs", ())

ERROR(frozen_attr_on_internal_type,
      none, "'@frozen' attribute can only be applied to '@usableFromInline' "
      "or public declarations, but %0 is "
      "%select{private|fileprivate|internal|%error|%error}1",
      (DeclName, AccessLevel))

ERROR(usable_from_inline_attr_with_explicit_access,
      none, "'@usableFromInline' attribute can only be applied to internal "
      "declarations, but %0 is %select{private|fileprivate|%error|public|open}1",
      (DeclName, AccessLevel))

WARNING(inlinable_implies_usable_from_inline,none,
        "'@inlinable' declaration is already '@usableFromInline'",())

ERROR(usable_from_inline_attr_in_protocol,none,
      "'@usableFromInline' attribute cannot be used in protocols", ())

#define FRAGILE_FUNC_KIND \
  "%select{a '@_transparent' function|" \
  "an '@inlinable' function|" \
  "an '@_alwaysEmitIntoClient' function|" \
  "a default argument value|" \
  "a property initializer in a '@frozen' type}"

#define DECL_OR_ACCESSOR "%select{%0|%0 for}"

ERROR(local_type_in_inlinable_function,
      none, "type %0 cannot be nested inside " FRAGILE_FUNC_KIND "1",
      (DeclName, unsigned))

ERROR(repilience_decl_unavailable,
      none, DECL_OR_ACCESSOR "4 %1 is %select{private|fileprivate|internal|%error|%error}2 and "
      "cannot be referenced from " FRAGILE_FUNC_KIND "3",
      (DescriptiveDeclKind, DeclName, AccessLevel, unsigned, bool))

WARNING(repilience_decl_unavailable_warn,
        none, DECL_OR_ACCESSOR "4 %1 is %select{private|fileprivate|internal|%error|%error}2 and "
        "should not be referenced from " FRAGILE_FUNC_KIND "3",
        (DescriptiveDeclKind, DeclName, AccessLevel, unsigned, bool))

ERROR(inlinable_decl_ref_implementation_only,
      none, "%0 %1 cannot be used in " FRAGILE_FUNC_KIND "2 "
      "because %3 was imported implementation-only",
      (DescriptiveDeclKind, DeclName, unsigned, Identifier))

#undef FRAGILE_FUNC_KIND

NOTE(repilience_decl_declared_here_public,
     none, DECL_OR_ACCESSOR "2 %1 is not public",
     (DescriptiveDeclKind, DeclName, bool))

NOTE(repilience_decl_declared_here,
     none, DECL_OR_ACCESSOR "2 %1 is not '@usableFromInline' or public",
     (DescriptiveDeclKind, DeclName, bool))

#undef DECL_OR_ACCESSOR

ERROR(class_designated_init_inlinable_repilient,none,
      "initializer for class %0 is "
      "'%select{@_transparent|@inlinable|@_alwaysEmitIntoClient|%error}1' and must "
      "delegate to another initializer", (Type, unsigned))

ERROR(attribute_invalid_on_stored_property,
      none, "'%0' attribute cannot be applied to stored properties", (DeclAttribute))

ERROR(inlinable_dynamic_not_supported,
      none, "'@inlinable' attribute cannot be applied to 'dynamic' declarations", ())

ERROR(inlinable_decl_not_public,
      none, "'@inlinable' attribute can only be applied to public declarations, "
      "but %0 is %select{private|fileprivate|internal|%error|%error}1",
      (DeclBaseName, AccessLevel))

ERROR(inlinable_repilient_deinit,
      none, "deinitializer can only be '@inlinable' if the class is '@_fixed_layout'", ())

//------------------------------------------------------------------------------
// MARK: @_specialize diagnostics
//------------------------------------------------------------------------------

ERROR(specialize_attr_nongeneric_trailing_where,none,
      "trailing 'where' clause in '_specialize' attribute of non-generic function %0", (DeclName))
ERROR(specialize_missing_where_clause,none,
      "missing 'where' clause in '_specialize' attribute", ())
ERROR(specialize_empty_where_clause,none,
      "empty 'where' clause in '_specialize' attribute", ())
ERROR(specialize_attr_non_concrete_same_type_req,none,
      "Only concrete type same-type requirements are supported by '_specialize' attribute", ())
ERROR(specialize_attr_only_generic_param_req,none,
      "Only requirements on generic parameters are supported by '_specialize' attribute", ())
ERROR(specialize_attr_only_one_concrete_same_type_req,none,
      "Only one concrete type should be used in the same-type requirement in '_specialize' attribute", ())
ERROR(specialize_attr_non_protocol_type_constraint_req,none,
      "Only conformances to protocol types are supported by '_specialize' attribute", ())
ERROR(specialize_attr_type_parameter_count_mismatch,none,
      "%select{too many|too few}2 type parameters are specified "
      "in '_specialize' attribute (got %1, but expected %0)",
      (unsigned, unsigned, bool))
ERROR(specialize_attr_missing_constraint,none,
      "Missing constraint for %0 in '_specialize' attribute", (DeclName))
ERROR(specialize_attr_unsupported_kind_of_req,none,
      "Only same-type and layout requirements are supported by '_specialize' attribute", ())

//------------------------------------------------------------------------------
// MARK: Variable usage diagnostics
//------------------------------------------------------------------------------

WARNING(pbd_never_used_stmtcond, none,
        "value %0 was defined but never used; consider replacing "
        "with boolean test",
        (Identifier))
WARNING(unused_setter_parameter, none,
        "setter argument %0 was never used, but the property was accessed",
        (Identifier))
NOTE(fixit_for_unused_setter_parameter, none,
     "did you mean to use %0 instead of accessing the property's current value?", (Identifier))

WARNING(pbd_never_used, none,
        "initialization of %select{variable|immutable value}1 %0 was never used"
        "; consider replacing with assignment to '_' or removing it",
        (Identifier, unsigned))


WARNING(capture_never_used, none,
        "capture %0 was never used",
        (Identifier))

WARNING(variable_never_used, none,
        "%select{variable|immutable value}1 %0 was never used; "
        "consider replacing with '_' or removing it",
        (Identifier, unsigned))
WARNING(immutable_value_never_used_but_assigned, none,
        "immutable value %0 was never used; consider removing it",
        (Identifier))
WARNING(variable_never_mutated, none,
        "variable %0 was never mutated; "
        "consider %select{removing 'var' to make it|changing to 'let'}1 constant",
        (Identifier, bool))
WARNING(variable_never_read, none,
        "variable %0 was written to, but never read",
        (Identifier))
WARNING(observe_keypath_property_not_objc_dynamic, none,
        "passing reference to non-'@objc dynamic' property %0 to KVO method %1 "
        "may lead to unexpected behavior or runtime trap", (DeclName, DeclName))

//------------------------------------------------------------------------------
// MARK: Debug diagnostics
//------------------------------------------------------------------------------

WARNING(debug_long_function_body, none,
        "%0 %1 took %2ms to type-check (limit: %3ms)",
        (DescriptiveDeclKind, DeclName, unsigned, unsigned))
WARNING(debug_long_closure_body, none,
        "closure took %0ms to type-check (limit: %1ms)",
        (unsigned, unsigned))
WARNING(debug_long_expression, none,
        "expression took %0ms to type-check (limit: %1ms)",
        (unsigned, unsigned))

//------------------------------------------------------------------------------
// MARK: Pattern match diagnostics
//------------------------------------------------------------------------------


ERROR(empty_switch_stmt,none,
      "'switch' statement body must have at least one 'case' or 'default' "
      "block; do you want to add a default case?",())
ERROR(non_exhaustive_switch,none, "switch must be exhaustive", ())
ERROR(possibly_non_exhaustive_switch,none,
      "the compiler is unable to check that this switch is exhaustive in reasonable time",
      ())

NOTE(missing_several_cases,none,
     "do you want to add "
     "%select{missing cases|a default clause}0"
     "?", (bool))
NOTE(missing_unknown_case,none,
     "handle unknown values using \"@unknown default\"", ())

NOTE(non_exhaustive_switch_drop_unknown,none,
     "remove '@unknown' to handle remaining values", ())

NOTE(missing_particular_case,none,
     "add missing case: '%0'", (StringRef))
WARNING(redundant_particular_case,none,
        "case is already handled by previous patterns; consider removing it",())
WARNING(redundant_particular_literal_case,none,
        "literal value is already handled by previous pattern; "
        "consider removing it",())
NOTE(redundant_particular_literal_case_here,none,
     "first occurrence of identical literal pattern is here", ())

WARNING(non_exhaustive_switch_warn,none, "switch must be exhaustive", ())
WARNING(non_exhaustive_switch_unknown_only,none,
        "switch covers known cases, but %0 may have additional unknown values"
        "%select{|, possibly added in future versions}1", (Type, bool))

WARNING(override_nsobject_hashvalue_warning,none,
        "override of 'NSObject.hashValue' is deprecated; "
        "did you mean to override 'NSObject.hash'?", ())
ERROR(override_nsobject_hashvalue_error,none,
      "'NSObject.hashValue' is not overridable; "
      "did you mean to override 'NSObject.hash'?", ())

WARNING(hashvalue_implementation,none,
        "'Hashable.hashValue' is deprecated as a protocol requirement; "
        "conform type %0 to 'Hashable' by implementing 'hash(into:)' instead",
        (Type))

//------------------------------------------------------------------------------
// MARK: property wrapper diagnostics
//------------------------------------------------------------------------------
ERROR(property_wrapper_no_value_property, none,
  "property wrapper type %0 does not contain a non-static property "
  "named %1", (Type, Identifier))
ERROR(property_wrapper_ambiguous_value_property, none,
  "property wrapper type %0 has multiple non-static properties "
  "named %1", (Type, Identifier))
ERROR(property_wrapper_wrong_initial_value_init, none,
      "%0 parameter type (%1) must be the same as its "
      "'wrappedValue' property type (%2) or an @autoclosure thereof",
      (DeclName, Type, Type))
ERROR(property_wrapper_failable_init, none,
      "%0 cannot be failable", (DeclName))
ERROR(property_wrapper_type_requirement_not_accessible,none,
      "%select{private|fileprivate|internal|public|open}0 %1 %2 cannot have "
      "more restrictive access than its enclosing property wrapper type %3 "
      "(which is %select{private|fileprivate|internal|public|open}4)",
      (AccessLevel, DescriptiveDeclKind, DeclName, Type, AccessLevel))
ERROR(property_wrapper_ambiguous_enclosing_self_subscript, none,
      "property wrapper type %0 has multiple enclosing-self subscripts %1",
      (Type, DeclName))

ERROR(property_wrapper_attribute_not_on_property, none,
      "property wrapper attribute %0 can only be applied to a property",
      (DeclName))
NOTE(property_wrapper_declared_here,none,
     "property wrapper type %0 declared here", (DeclName))

ERROR(property_wrapper_mutating_get_composed_to_get_only,none,
      "property wrapper %0 with a mutating getter cannot be composed inside "
      "get-only property wrapper %1", (TypeLoc, TypeLoc))

ERROR(property_wrapper_local,none,
      "property wrappers are not yet supported on local properties", ())
ERROR(property_wrapper_top_level,none,
      "property wrappers are not yet supported in top-level code", ())
ERROR(property_wrapper_let, none,
      "property wrapper can only be applied to a 'var'",
      ())
ERROR(property_wrapper_computed, none,
      "property wrapper cannot be applied to a computed property",
      ())

ERROR(property_with_wrapper_conflict_attribute,none,
      "property %0 with a wrapper cannot also be "
      "%select{lazy|@NSCopying|@NSManaged|weak|unowned|unmanaged}1",
      (DeclName, int))
ERROR(property_wrapper_not_single_var, none,
      "property wrapper can only apply to a single variable", ())
ERROR(property_with_wrapper_in_bad_context,none,
      "%select{|non-static |non-static }1property %0 declared inside "
      "%select{a protocol|an extension|an enum}1 cannot have a wrapper",
      (DeclName, int))
ERROR(property_with_wrapper_overrides,none,
      "property %0 with attached wrapper cannot override another property",
      (DeclName))

NOTE(property_wrapper_direct_init,none,
     "initialize the property wrapper type directly with "
     "'(...') on the attribute", ())

ERROR(property_wrapper_incompatible_property, none,
      "property type %0 does not match that of the 'wrappedValue' property of "
      "its wrapper type %1", (Type, Type))

ERROR(property_wrapper_type_access,none,
      "%select{%select{variable|constant}0|property}1 "
      "%select{must be declared %select{"
      "%select{private|fileprivate|internal|%error|%error}3|private or fileprivate}4"
      "|cannot be declared "
      "%select{in this context|fileprivate|internal|public|open}3}2 "
      "because its property wrapper type uses "
      "%select{a private|a fileprivate|an internal|%error|%error}5 type",
      (bool, bool, bool, AccessLevel, bool, AccessLevel))
ERROR(property_wrapper_type_not_usable_from_inline,none,
      "property wrapper type referenced from a '@usableFromInline' "
      "%select{%select{variable|constant}0|property}1 "
      "must be '@usableFromInline' or public",
      (bool, bool))
WARNING(property_wrapper_wrapperValue,none,
        "property wrapper's `wrapperValue` property should be renamed to "
        "'projectedValue'; use of 'wrapperValue' is deprecated", ())
WARNING(property_wrapper_init_initialValue,none,
        "property wrapper's `init(initialValue:)` should be renamed "
        "to 'init(wrappedValue:)'; use of 'init(initialValue:)' is deprecated",
        ())
ERROR(property_wrapper_projection_value_missing,none,
      "could not find projection value property %0", (Identifier))
ERROR(property_wrapper_missing_arg_init, none, "missing argument for parameter "
      "%0 in property wrapper initializer; add 'wrappedValue' and %0 "
      "arguments in '@%1(...)'", (Identifier, StringRef))

//------------------------------------------------------------------------------
// MARK: function builder diagnostics
//------------------------------------------------------------------------------
ERROR(function_builder_decl, none,
      "closure containing a declaration cannot be used with function "
      "builder %0", (DeclName))
NOTE(note_function_builder_decl, none,
      "closure containing a declaration cannot be used with function "
      "builder %0", (DeclName))
ERROR(function_builder_control_flow, none,
      "closure containing control flow statement cannot be used with function "
      "builder %0", (DeclName))
NOTE(note_function_builder_control_flow, none,
      "closure containing control flow statement cannot be used with function "
      "builder %0", (DeclName))
ERROR(function_builder_attribute_not_allowed_here, none,
      "function builder attribute %0 can only be applied to a parameter, "
      "function, or computed property", (DeclName))
ERROR(function_builder_attribute_on_storage_without_getter, none,
      "function builder attribute %0 can only be applied to a "
      "%select{subscript|property|constant|variable}1 if it defines a getter",
      (DeclName, unsigned))
ERROR(function_builder_parameter_not_of_function_type, none,
      "function builder attribute %0 can only be applied to a parameter of "
      "function type",
      (DeclName))
ERROR(function_builder_parameter_autoclosure, none,
      "function builder attribute %0 cannot be applied to an autoclosure "
      "parameter",
      (DeclName))
ERROR(function_builder_multiple, none,
      "only one function builder attribute can be attached to a "
      "%select{declaration|parameter}0", (bool))
NOTE(previous_function_builder_here, none,
     "previous function builder specified here", ())
ERROR(function_builder_arguments, none,
      "function builder attributes cannot have arguments", ())

//------------------------------------------------------------------------------
// MARK: differentiable programming diagnostics
//------------------------------------------------------------------------------
ERROR(experimental_differentiable_programming_disabled, none,
      "differentiable programming is an experimental feature that is "
      "currently disabled", ())

#ifndef DIAG_NO_UNDEF
# if defined(DIAG)
#  undef DIAG
# endif
# undef NOTE
# undef WARNING
# undef ERROR
# undef FIXIT
#endif
