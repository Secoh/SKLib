// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides template basic math functions that can be constexpr.
// This is internal SKLib file and must NOT be included directly.

template<class R, class V, SKLIB_INTERNAL_ENABLE_IF_INT(V)>   //sk  std::enable_if_t<sklib::is_integer_v<V>, bool> = true>
constexpr auto upow(R x, V p)
{
    R z = R(1);
    if (p<0) return z;

    //sk!! need to search for nonzero bit starting from above
    // then go from 0 to the limit, may be lower than max

    unsigned N = sklib::bits_width_v<R>;
    for (; )
    while (p)
    {
        if (bit_test<R, 0>(p)) z *= x;
        x *= x;
        p /= 2;
    }

    return z;
}

template<class R, class V, std::enable_if_t<sklib::is_signed_integer_v<V>, bool> = true>
constexpr auto ipow(R x, V p)
{
    typedef sklib::make_unsigned_if_integer_type<V> uV;
    return (p<0) ? upow(1/x, (uV)-p) : upow(x, (uV)p);
}

