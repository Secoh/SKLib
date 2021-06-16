// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later.
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s).
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#pragma once

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#define SKLIB_PRELOADED_COMMON_HEADERS

#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <mutex>
#include <thread>
#include <chrono>

#else
#error SkLib: This cannot happen. Common Headers are loaded twice despite pragma once.
#endif

