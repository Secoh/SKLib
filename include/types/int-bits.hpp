// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides constants and functions related to bits in integers, placement, count, etc.
// This is internal SKLib file and must NOT be included directly.

static constexpr unsigned OCTET_BITS = 8;

static_assert(sizeof(char) == 1, "SKLIB ** INTERNAL ERROR ** sizeof(char) must be equal to 1");
static_assert(CHAR_BIT >= OCTET_BITS, "SKLIB ** INTERNAL ERROR ** char size must be at least 8 bits");

SKLIB_TEMPLATE_IF_INT(T) inline constexpr unsigned bits_width_v = OCTET_BITS * (unsigned)sizeof(T);
SKLIB_TEMPLATE_IF_INT(T) inline constexpr unsigned bits_width_less_sign_v = bits_width_v<T> - (std::is_signed_v<T> ? 1 : 0);

namespace opaque
{
    template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
    static constexpr T bits_data_cap()
    {
        static_assert(Width < bits_width_less_sign_v<T>,
                      "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");
        return (T(1) << Width);
    }

    template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
    static constexpr T bits_data_high_1()
    {
        static_assert(Width > 0, "SKLIB ** INTERNAL ERROR ** Bit count must be nonzero");
        static_assert(Width <= bits_width_less_sign_v<T>, "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");
        return (T(1) << (Width-1));
    }

}; // namespace opaque

// bits by count (for container)

template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
inline constexpr T bits_data_cap_v = sklib::opaque::bits_data_cap<T, Width>();

template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
inline constexpr T bits_data_high_1_v = sklib::opaque::bits_data_high_1<T, Width>();

template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
inline constexpr T bits_data_mask_v = bits_data_high_1_v<T, Width> | T(bits_data_high_1_v<T, Width>-1);

// bits for the type

SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_high_1_v = bits_data_high_1_v<T, bits_width_v<T>>;
SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_mask_v = bits_high_1_v<T> | T(bits_high_1_v<T> - 1);

SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_low_half_v = (T(1) << (bits_width_v<T>/2)) - 1;
SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_high_half_v = (bits_low_half_v<T> << (bits_width_v<T>/2));

static constexpr uint8_t OCTET_MASK         = bits_mask_v<uint8_t>;
static constexpr size_t  OCTET_ADDRESS_SPAN = OCTET_MASK + 1;

// bits by count as function argument

SKLIB_TEMPLATE_IF_INT(T) static constexpr T bits_data_cap(unsigned width)
{
    return (T(1) << width);
}
SKLIB_TEMPLATE_IF_INT(T) static constexpr T bits_data_high_1(unsigned width)
{
    return (T(1) << (width-1));
}
SKLIB_TEMPLATE_IF_INT(T) static constexpr T bits_data_mask(unsigned width)
{
    const auto A = bits_data_high_1<T>(width);
    return A | (A-1);
}

