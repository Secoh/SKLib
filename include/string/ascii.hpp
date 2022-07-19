// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides Unicode-aware constexpr templates for classical ASCII characters support functions.
// This is internal SKLib file and must NOT be included directly.

namespace ascii   // ASCII designators (useful subset)
{
    static constexpr uint8_t NUL = '\0';
    static constexpr uint8_t STX = '\x02';   // Start of Text
    static constexpr uint8_t ETX = '\x03';   // End of Text
    static constexpr uint8_t EOT = '\x04';   // End of Transmission
    static constexpr uint8_t ENQ = '\x05';   // Enquiry
    static constexpr uint8_t ACK = '\x06';   // Acknowledge
    static constexpr uint8_t NAK = '\x15';   // Negative Acknowledgement
    static constexpr uint8_t BEL = '\a';     // Bell
    static constexpr uint8_t BS  = '\b';
    static constexpr uint8_t TAB = '\t';
    static constexpr uint8_t LF  = '\n';
    static constexpr uint8_t CR  = '\r';
    static constexpr uint8_t VTAB= '\v';    // Vertical Tab
    static constexpr uint8_t FF  = '\f';      // Form Feed
    static constexpr uint8_t XON = '\x11';
    static constexpr uint8_t XOFF= '\x13';
    static constexpr uint8_t CAN = '\x18';   // Cancel
    static constexpr uint8_t ESC = '\x1B';
    static constexpr uint8_t DEL = '\x7F';
    static constexpr uint8_t SPC = ' ';      // Space

    static constexpr uint8_t A = 'A';
    static constexpr uint8_t a = 'a';
    static constexpr uint8_t Z = 'Z';
    static constexpr uint8_t z = 'z';

    static_assert(a > A, "SKLIB ** INTERNAL ERROR ** Alphabet does not start with ASCII, this cannot happen.");
    static constexpr uint8_t high_to_low_offset = a - A;
    static_assert(z == Z + high_to_low_offset, "SKLIB ** INTERNAL ERROR ** Alphabet does not start with ASCII, this cannot happen.");

    static constexpr uint8_t n0 = '0';
    static constexpr uint8_t n9 = '9';
    static_assert(n9 == n0 + 9, "SKLIB ** INTERNAL ERROR ** Alphabet does not start with ASCII, this cannot happen.");
};

template<class T>
constexpr bool is_ascii_upper(T c)
{
    return is_clamped<make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(c), ascii::A, ascii::Z);
}

template<class T>
constexpr bool is_ascii_lower(T c)
{
    return is_clamped<make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(c), ascii::a, ascii::z);
}

template<class T>
constexpr bool is_ascii_alpha(T c)
{
    return (is_ascii_upper<T>(c) || is_ascii_lower<T>(c));
}

template<class T>
constexpr bool is_num(T c)
{
    return is_clamped<make_unsigned_if_integer_type<T>>(to_unsigned_if_integer(c), ascii::n0, ascii::n9);
}

template<class T>
constexpr bool is_ascii_alpha_num(T c)
{
    return (is_ascii_alpha<T>(c) || is_num<T>(c));
}

template<class T>
constexpr bool is_c_name_token(T c)
{
    return (is_ascii_alpha_num<T>(c) || c == '_');
}

template<class T>
constexpr bool is_space(T c)
{
    return (c == ascii::SPC || c == ascii::TAB || c == ascii::LF || c == ascii::CR);
}

template<class T>
constexpr auto ascii_toupper(T c)
{
    return (is_ascii_lower(c) ? c - ascii::high_to_low_offset : c);
}

template<class T>
constexpr auto ascii_tolower(T c)
{
    return (is_ascii_upper(c) ? c + ascii::high_to_low_offset : c);
}

