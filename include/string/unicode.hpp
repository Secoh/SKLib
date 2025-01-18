// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides Unicode-related functions, as well as limited language support, other than English.
// This is internal SKLib file and must NOT be included directly.

namespace unicode   // UNICODE designators (useful subset)
{
    inline constexpr uint16_t SURROGATE_START = 0xD800u;
    inline constexpr uint16_t SURROGATE_CAP   = 0xE000u;
    inline constexpr uint16_t HIGH_SURROGATE  = SURROGATE_START;
    inline constexpr uint16_t HIGH_SURROGATE_LENGTH = (SURROGATE_CAP - SURROGATE_START) / 2;
    inline constexpr uint16_t LOW_SURROGATE = HIGH_SURROGATE + HIGH_SURROGATE_LENGTH;
    inline constexpr uint16_t LOW_SURROGATE_LENGTH = SURROGATE_CAP - LOW_SURROGATE;
    inline constexpr uint16_t HIGH_SURROGATE_CAP = HIGH_SURROGATE + HIGH_SURROGATE_LENGTH;
    inline constexpr uint16_t LOW_SURROGATE_CAP  = LOW_SURROGATE + LOW_SURROGATE_LENGTH;
    static_assert(HIGH_SURROGATE_LENGTH == LOW_SURROGATE_LENGTH, "SKLIB ** INTERNAL ERROR ** UTF-16 Surrogate areas defined incorrectly. This cannot happen.");

    inline constexpr uint16_t REPLACEMENT_MARK = 0xFFFDu;   // also symbol for encoding error
    inline constexpr uint16_t BYTE_ORDER = 0xFEFFu;
    inline constexpr uint16_t NOT_CHAR_A = 0xFFFEu;
    inline constexpr uint16_t NOT_CHAR_B = 0xFFFFu;

    inline constexpr int PLANE_SIZE_BITS  = 16;
    inline constexpr uint32_t PLANE_SIZE  = 0x10000u;
    inline constexpr uint32_t UNICODE_CAP = 0x110000u;

    static_assert(PLANE_SIZE % LOW_SURROGATE_LENGTH == 0, "SKLIB ** INTERNAL ERROR ** Unicode: Plane Size must be multiplies of Low Surrogate span");
};

template<class T>
constexpr bool is_unicode_surrogate(T ch)
{
    return is_clamped<sklib::make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(ch), unicode::SURROGATE_START, unicode::SURROGATE_CAP-1);
}

template<class T>
constexpr bool is_unicode_high_surrogate(T ch)
{
    return is_clamped<sklib::make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(ch), unicode::HIGH_SURROGATE, unicode::HIGH_SURROGATE_CAP-1);
}

template<class T>
constexpr bool is_unicode_low_surrogate(T ch)
{
    return is_clamped<sklib::make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(ch), unicode::LOW_SURROGATE, unicode::LOW_SURROGATE_CAP-1);
}

template<class T>
constexpr bool is_unicode(T ch)
{
    return is_clamped<sklib::make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(ch), 0, unicode::UNICODE_CAP-1);
}

template<class T>
constexpr bool is_unicode_bmp(T ch)
{
    return is_clamped<sklib::make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(ch), 0, unicode::PLANE_SIZE-1);
}

namespace aux
{
    // assumes without assertion, entering high surrorage, low surrogate
    constexpr uint32_t utf16_combine(uint16_t high, uint16_t low)
    {
        return (high - sklib::unicode::HIGH_SURROGATE) * sklib::unicode::LOW_SURROGATE_LENGTH
               + low + (sklib::unicode::PLANE_SIZE - sklib::unicode::LOW_SURROGATE);
    }

    // assumes without assertion, ch is valid unicode letter in supplemental planes
    constexpr void utf16_split(uint32_t ch, uint16_t& high, uint16_t& low)
    {
        high = (uint16_t)((ch - sklib::unicode::PLANE_SIZE) / sklib::unicode::LOW_SURROGATE_LENGTH) + sklib::unicode::HIGH_SURROGATE;
        low  = (uint16_t)(ch % sklib::unicode::LOW_SURROGATE_LENGTH) + sklib::unicode::LOW_SURROGATE;
    };

    // assumes without assertion, ch is valid unicode letter in supplemental planes
    constexpr std::pair<uint16_t, uint16_t> utf16_split(uint32_t ch)
    {
        std::pair<uint16_t, uint16_t> R;
        utf16_split(ch, R.first, R.second);
        return R;
    }
};

// ------------- certain national encodings, BOTH Unicode and single byte (when available)

#include "lang/russian.hpp"

