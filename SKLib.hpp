// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2019-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

//sk TODO: mention here all #defines that control SKLib's behavior
// Also: need dedicated project to verify compile-time static types, variables, etc

// Currently supported language flavor is C++20

#ifndef SKLIB_INCLUDED_ALL
#define SKLIB_INCLUDED_ALL

// To keep all header-only SkLib code in one place
// (e.g. to accelerate loading precompiled headers in MSVC)
// just list all available SkLib modules in one file

#include "include/configure.hpp"

#include "include/types.hpp"
#include "include/utility.hpp"

#include "include/bitwise.hpp"
#include "include/checksum.hpp"
#include "include/timer.hpp"

#include "include/cmdpar.hpp"
#include "include/comms.hpp"

#include "include/math.hpp"


#endif // SKLIB_INCLUDED_ALL

