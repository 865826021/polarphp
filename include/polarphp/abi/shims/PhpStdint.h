//===--- SwiftStdint.h ------------------------------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef POLRPHP_STDLIB_SHIMS_POLARPHP_STDINT_H
#define POLRPHP_STDLIB_SHIMS_POLARPHP_STDINT_H

// stdint.h is provided by Clang, but it dispatches to libc's stdint.h.  As a
// result, using stdint.h here would pull in Darwin module (which includes
// libc). This creates a dependency cycle, so we can't use stdint.h in
// SwiftShims.
// On Linux, the story is different. We get the error message
// "/usr/include/x86_64-linux-gnu/sys/types.h:146:10: error: 'stddef.h' file not
// found"
// This is a known Clang/Ubuntu bug.

// Clang has been defining __INTxx_TYPE__ macros for a long time.
// __UINTxx_TYPE__ are defined only since Clang 3.5.
#if !defined(__APPLE__) && !defined(__linux__)
#include <stdint.h>
typedef int64_t __polarphp_int64_t;
typedef uint64_t __polarphp_uint64_t;
typedef int32_t __polarphp_int32_t;
typedef uint32_t __polarphp_uint32_t;
typedef int16_t __polarphp_int16_t;
typedef uint16_t __polarphp_uint16_t;
typedef int8_t __polarphp_int8_t;
typedef uint8_t __polarphp_uint8_t;
typedef intptr_t __polarphp_intptr_t;
typedef uintptr_t __polarphp_uintptr_t;
#else
typedef __INT64_TYPE__ __polarphp_int64_t;
#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ __polarphp_uint64_t;
#else
typedef unsigned __INT64_TYPE__ __polarphp_uint64_t;
#endif

typedef __INT32_TYPE__ __polarphp_int32_t;
#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ __polarphp_uint32_t;
#else
typedef unsigned __INT32_TYPE__ __polarphp_uint32_t;
#endif

typedef __INT16_TYPE__ __polarphp_int16_t;
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ __polarphp_uint16_t;
#else
typedef unsigned __INT16_TYPE__ __polarphp_uint16_t;
#endif

typedef __INT8_TYPE__ __polarphp_int8_t;
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ __polarphp_uint8_t;
#else
typedef unsigned __INT8_TYPE__ __polarphp_uint8_t;
#endif

#define __polarphp_join3(a,b,c) a ## b ## c

#define __polarphp_intn_t(n) __polarphp_join3(__polarphp_int, n, _t)
#define __polarphp_uintn_t(n) __polarphp_join3(__polarphp_uint, n, _t)

#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_WIN32)
typedef __polarphp_int32_t __polarphp_intptr_t;
typedef __polarphp_uint32_t __polarphp_uintptr_t;
#elif defined(_WIN64)
typedef __polarphp_int64_t __polarphp_intptr_t;
typedef __polarphp_uint64_t __polarphp_uintptr_t;
#else
#error unknown windows pointer width
#endif
#else
typedef __polarphp_intn_t(__INTPTR_WIDTH__) __polarphp_intptr_t;
typedef __polarphp_uintn_t(__INTPTR_WIDTH__) __polarphp_uintptr_t;
#endif
#endif

#endif // POLRPHP_STDLIB_SHIMS_POLARPHP_STDINT_H
