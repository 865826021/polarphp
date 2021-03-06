// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/11/26.

#ifndef POLARPHP_BASIC_SANITIZERS_H
#define POLARPHP_BASIC_SANITIZERS_H

namespace polar {

// Enabling bitwise masking.
enum class SanitizerKind : unsigned
{
#define SANITIZER(enum_bit, kind, name, file) kind = (1 << enum_bit),
#include "polarphp/basic/SanitizersDef.h"
};

} // polar

#endif // POLARPHP_BASIC_SANITIZERS_H
