// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// This file defines public interface to communication functions and places them under sklib namespace entirely.
//
// The actual working code is moved to separate CODE file in order to eliminate namespace pollution coming from global
// System API headers (such as Windows.h). CODE files are specific to the subsystem. Currently, the following
// subsystems are available:
// - TCP/IP Sockets   (code file: socket-code.hpp)
// - RS-232 Serial I/O; USB to Serial support (rs232-code.hpp)
// SKLib uses headers-only approach where possible. Instead of using precompiled libraries, the user shall dedicate
// compilation unit for CODE file, so there can be no name pollution at compilation time, only has potential at linking
// where C++ compiler provides mitigation measures. Example, we use Sockets. We need to:
// - include this file: "comms.hpp" anywhere where you USE sockets;
// - dedicate a single .cpp file for "library implementation", and place the single line in it:
//   #include <SKLib/source/comms-code.hpp>
// This way, any user code "doesn't know" about system-specific libraries, and the system functionality is accessed
// at the link time.

#ifndef SKLIB_INCLUDED_COMMS_HPP
#define SKLIB_INCLUDED_COMMS_HPP

#include<cstdint>
#include<memory>
#include<string>

/*  //sk
#include<type_traits>
#include<limits>
#include<utility>

#include"types.hpp"
#include"math.hpp"
*/

namespace sklib
{

#include "comms/socket.hpp"
// #include "comms/rs232.hpp"

};

#endif // SKLIB_INCLUDED_COMMS_HPP

