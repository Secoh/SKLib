// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_UTILITY_TST_SPECIAL_HPP
#define SKLIB_INCLUDED_UTILITY_TST_SPECIAL_HPP

#include "../configure.hpp"
#ifdef SKLIB_TARGET_TEST

#include <string>
#include <iostream>
#include <iomanip>

namespace sklib {

// Provides helpers for SKLib testing.
// This is internal SKLib file and must NOT be included directly.

namespace aux
{
    template<class T>
    constexpr void table256_print(const std::string& title, const T* U, int width, bool hex = false, int hex_digits = 2)
    {
        std::cout << title << "\n";

        for (int k=0; k<256; )
        {
            for (int i=0; i<width; i++)
            {
                if (i) std::cout << " ";

                if (!hex)
                {
                    std::cout << (U[k++] * 1);    // <-- use integer promotion to eliminate "char" conversion on output
                }
                else
                {
                    std::cout << "0x" << std::setfill('0') << std::setw(hex_digits) << std::hex << std::uppercase << (U[k++] * 1);
                }

                if (k<256) std::cout << ",";
            }
            std::cout << "\n";
        }
    }
};

}; // namespace sklib

#endif // SKLIB_TARGET_TEST

#endif // SKLIB_INCLUDED_UTILITY_TST_SPECIAL_HPP

