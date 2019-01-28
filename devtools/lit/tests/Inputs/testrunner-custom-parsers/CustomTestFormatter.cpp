// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarphp software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2018/11/18.

#include "CustomTestFormatter.h"
#include "polarphp/basic/adt/StringRef.h"
#include "Test.h"

namespace polar {
namespace lit {

ResultPointer CustomTestFormatter::execute(TestPointer, LitConfigPointer)
{
   return std::make_shared<Result>(PASS, "");
}

} // lit
} // polar
