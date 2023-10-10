// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#ifndef SKLIB_INCLUDED_TYPES_HPP
#define SKLIB_INCLUDED_TYPES_HPP

#include <climits>
#include <type_traits>

#ifndef SKLIB_TARGET_MCU
#include <string>
#endif

// October 2023
// Current supported language flavor is C++20

namespace sklib
{

#include "types/detect-type.hpp"
#include "types/int-conv.hpp"
#include "types/int-bits.hpp"
#include "types/misc-types.hpp"

}; // namespace sklib

#endif // SKLIB_INCLUDED_TYPES_HPP

