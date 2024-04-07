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

// conventions:  count of bits  - unsigned
//               count of bytes - size_t

#ifndef SKLIB_INCLUDED_BITWISE_HPP
#define SKLIB_INCLUDED_BITWISE_HPP

#include "types.hpp"
#include "utility.hpp"

namespace sklib
{

#include "bitwise/bmanip.hpp"
#include "bitwise/bstream.hpp"
#include "bitwise/base64.hpp"
#include "bitwise/bconf.hpp"

};

#ifndef SKLIB_TARGET_MCU

#include <iostream>
#include <fstream>
// conditional include of <string> is done in types.hpp

namespace sklib
{
#include "bitwise/bfstream.hpp"
};

#endif // SKLIB_TARGET_MCU

#endif // SKLIB_INCLUDED_BITWISE_HPP
