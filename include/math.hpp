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

#ifndef SKLIB_INCLUDED_MATH_HPP
#define SKLIB_INCLUDED_MATH_HPP

#include <random>
#include <type_traits>
//sk ... #include <intrin.h>
//sk ... #include <immintrin.h>
#include <array>
#include <functional>

#include "types.hpp"
#include "utility.hpp"


// for primes
#include "bitwise.hpp"
#include "checksum.hpp"
#include "timer.hpp"


// for debug!
#include <iostream>
#include <map>

namespace sklib
{

#include "math/cpu-support.hpp"
#include "math/algebra.hpp"
#include "math/primes.hpp"
#include "math/geometry.hpp"

};

#endif // SKLIB_INCLUDED_MATH_HPP

