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

#ifndef SKLIB_INCLUDED_STRING_HPP
#define SKLIB_INCLUDED_STRING_HPP

#include <cstdint>
#include <type_traits>
#include <limits>
#include <utility>

#include "types.hpp"
#include "math.hpp"


//sk move inside safe-std-string
#include "configure.hpp"
#ifndef SKLIB_TARGET_MCU
#include <string>
#endif // SKLIB_TARGET_MCU

namespace sklib
{

#include "string/ascii.hpp"
#include "string/unicode.hpp"
#include "string/checks.hpp"
#include "string/numbers.hpp"
#include "string/collection.hpp"

//sk move inside safe-std-string
#ifndef SKLIB_TARGET_MCU
#include "string/safe-std-string.hpp"
#endif // SKLIB_TARGET_MCU

};

#endif // SKLIB_INCLUDED_STRING_HPP

