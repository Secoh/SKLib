// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2025] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_CONFIGURE_HPP
#define SKLIB_INCLUDED_CONFIGURE_HPP

// Verifies that requested compilation mode is correct
//   possible control macros:
//    SKLIB_TARGET_MCU   - Restricted library, suitable for microcontrollers (also, simple microcomputers)
//    SKLIB_TARGET_ATMEL - Specific to Atmel SAM microcontrollers
//    SKLIB_TARGET_TEST  - Program is designed to run tests from console, predominantly under Windows
//    SKLIB_TARGET_QT    - (Future) Add QT support
// This is internal SKLib file and must NOT be included directly.

// Atmel target implies MCU
#if defined(SKLIB_TARGET_ATMEL) && !defined(SKLIB_TARGET_MCU)
#define SKLIB_TARGET_MCU
#endif

// Test mode is incompatible with microcontrollers
#if defined(SKLIB_TARGET_MCU) && defined(SKLIB_TARGET_TEST)
#error Test Mode is incompatible with Microcontroller restrictions
#endif

// QT is incompatible with microcontrollers
#if defined(SKLIB_TARGET_MCU) && defined(SKLIB_TARGET_QT)
#error Test Mode is incompatible with Microcontroller restrictions
#endif

#endif // SKLIB_INCLUDED_CONFIGURE_HPP

