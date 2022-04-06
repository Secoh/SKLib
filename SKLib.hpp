// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2019-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#ifndef SKLIB_INCLUDED_ALL
#define SKLIB_INCLUDED_ALL

// Lets preload first all the standard C++ headers ever used in SKLib

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#define SKLIB_PRELOADED_COMMON_HEADERS

#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <type_traits>
#include <mutex>
#include <thread>
#include <chrono>
#include <ratio>

#endif

// To keep all header-only SkLib code in one place
// (e.g. to accelerate loading precompiled headers in MSVC)
// just list all available SkLib modules in one file

#include "helpers.hpp"
#include "bitwise.hpp"
#include "checksum.hpp"
#include "timer.hpp"

#include "comms.hpp"



#endif // SKLIB_INCLUDED_ALL
