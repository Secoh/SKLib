// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides useful functions missed from C/C++ standard, and/or a flavor of standard C function(s).
// This is internal SKLib file and must NOT be included directly.

namespace opaque
{
    // workaround for notorious windows.h definition
    template<class T>
    constexpr auto alt_max(const T& a, const T& b)
    {
        return (a < b ? b : a);
    }

    // workaround for notorious windows.h definition
    template<class T>
    constexpr auto alt_min(const T& a, const T& b)
    {
        return (a < b ? a : b);
    }
};

// test x is in range, inclusive
template<class T>
constexpr bool is_clamped(const T& x, const T& low, const T& high)
{
    return (x >= low && x <= high);
}

// test x is in range, inclusive low, not inclusive cap
template<class T>
constexpr bool is_clamped_cap(const T& x, const T& low, const T& cap)
{
    return (x >= low && x < cap);
}

// constexpr version of numeric abs()
template<class T, std::enable_if_t<sklib::is_signed_integer_v<T> || std::is_floating_point_v<T>, bool> = true>
constexpr T abs(const T& what)
{
    return ((what < 0) ? -what : what);
}

// expanding C standard with logical XOR
constexpr auto bool_xor(bool A, bool B)
{
    return (A ? !B : B);
}

