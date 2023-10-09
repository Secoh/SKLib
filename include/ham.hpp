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

#pragma once

// various definitions concerning amateur radio

#include <utility>

#include "bitwise.hpp"

namespace sklib
{
    static constexpr uint16_t ham_morse_code_point_base = ::sklib::ascii::SPC;
    static constexpr int      ham_morse_code_point_width = 6;
    static constexpr uint16_t ham_morse_code_point_range = sklib::bits_data_cap_v<int16_t, ham_morse_code_point_width>;
    static constexpr uint16_t ham_morse_code_max_length = 7;
    static constexpr int      ham_morse_code_length_width = ::sklib::bits_rank(ham_morse_code_max_length);
    static_assert(ham_morse_code_point_width + ham_morse_code_max_length + ham_morse_code_length_width <= sklib::bits_width_v<uint16_t>,
        "SKLIB ** INTERNAL ERROR ** Data type must be large enough to hold Morse encoding package");

    // === 16 bit encoding ===
    // Provides packaged encoding for the Morse code point, having the following data:
    // 1) Latin letter, digit, or punctuation (from capital ASCII table), represented by 6 bits, 0x20-base, MSB placement
    // 2) Length of the Morse sequence (0-7) - 3 bits
    // 3) Morse sequence up to 7 elements, in LSB-first orser - lowest 7 bits
    // 4) Any excess bit registes are padded by 0 and unused
    // Function returns 0 for error or if encoding is not possible
    // Argument ditdah is human-readable sequence of dots and dashes, for examp[le, ".-.." for letter 'L'
    // Any character other than dot or dash is error
    // Dot is represented by 0, dash is 1, written in opposite direction (see rule 3)
    // Thus, overall, L is represented by pack 101100 100 000 0010 or number 0xB202
    // Non-latin letters are normally represented by ASCII letters having similar shape (few exceptions)
    // For letters not represented by latin alphabet one shall assign them to unused code points within the range (64 places)
    // Use of translation table is recommended to convert from non-latin latters to ASCII code points
    //
    constexpr uint16_t ham_package_morse_symbol(char letter, const char* ditdah)
    {
        constexpr auto total_bits = sklib::bits_width_v<decltype(ham_package_morse_symbol(0, ""))>;

        if (letter >= 'a' && letter <= 'z') letter -= ('a'-'A');
        uint16_t code_point = std::bit_cast<unsigned char>(letter);
        if (code_point < ham_morse_code_point_base) return 0;
        code_point -= ham_morse_code_point_base;
        if (code_point >= ham_morse_code_point_range) return 0;

        code_point <<= (total_bits - ham_morse_code_point_width);

        const size_t size = ::sklib::strlen(ditdah);
        if (size > ham_morse_code_max_length) return 0;
        const uint16_t sz = static_cast<uint16_t>(size);

        code_point |= sz << (total_bits - ham_morse_code_point_width - ham_morse_code_length_width);

        uint16_t mk = 1;
        for (uint16_t k=0; k<sz; k++, mk<<=1)
        {
            auto c = ditdah[k];
            if (c == '-') code_point |= mk;
            else if (c != '.') return 0;
        }

        return code_point;
    }

    // === 8 bit encoding ===
    // Provides information only on dot-dash sequence; the size limitation is 7 elements
    // 1) The length 0-7 is indicated by the rank of the byte, thence
    // 2) Content of the remaining bits represent the Morse code sequence, in LSB-first orser
    // That is, MSBs are padded with 0, then one start bit, then content to the end
    // Function returns 0 for error or if encoding is not possible
    // Argument ditdah is human-readable sequence of dots and dashes, for examp[le, ".-.." for letter 'L'
    // Any character other than dot or dash is error
    // Dot is represented by 0, dash is 1, written in opposite direction (see rule 3)
    // Thus, overall, L is represented by pack 0001 0010 or number 0x12
    //
    constexpr uint8_t ham_package_morse_sequence(const char* ditdah)
    {
        constexpr auto total_bits = sklib::bits_width_v<decltype(ham_package_morse_sequence(""))>;

        const size_t size = ::sklib::strlen(ditdah);
        if (size >= total_bits) return 0;
        const uint8_t sz = static_cast<uint16_t>(size);

        uint8_t code = (1 << sz);

        uint16_t mk = 1;
        for (uint16_t k = 0; k < sz; k++, mk <<= 1)
        {
            auto c = ditdah[k];
            if (c == '-') code |= mk;
            else if (c != '.') return 0;
        }

        return code;
    }

    static constexpr void generate_table(const std::initializer_list<std::pair<char, const char*>>& config, sklib::supplement::encapsulated_array_type<uint8_t, ham_morse_code_point_range>& table)
    {
        for (const auto& elem : config)
        {
            int letter = std::bit_cast<unsigned char>(elem.first);
            if (letter >= 'a' && letter <= 'z') letter -= ('a' - 'A');

            if (letter < ham_morse_code_point_base) continue;

            letter -= ham_morse_code_point_base;
            if (letter >= ham_morse_code_point_range) continue;

            table.data[letter] = ham_package_morse_sequence(elem.second);
        }
    }

    static constexpr auto create_table(const std::initializer_list<std::pair<char, const char*>>& config)
    {
        sklib::supplement::encapsulated_array_type<uint8_t, ham_morse_code_point_range> R{ 0 };
        generate_table(config, R);
        return R;
    }

    static constexpr auto create_standard_latin_table()
    {
        return create_table({
                { 'A', ".-" },      { 'M', "--" },      { 'Y', "-.--" },    { '.', ".-.-.-" },
                { 'B', "-..." },    { 'N', "-." },      { 'Z', "--.." },    { '/', "-..-." },
                { 'C', "-.-." },    { 'O', "---" },     { '1', ".----" },   { '?', "..--.." },
                { 'D', "-.." },     { 'P', ".--." },    { '2', "..---" },   {',', "--..--"},
                { 'E', "." },       { 'Q', "--.-" },    { '3', "...--" },   {'+', ".-.-."},
                { 'F', "..-." },    { 'R', ".-." },     { '4', "....-" },   { '=', "-...-" },
                { 'G', "-.." },     { 'S', "..." },     { '5', "....." },   { '&', "...-.-" },
                { 'H', "...." },    { 'T', "-" },       { '6', "-...." },   { ':', "---..." },
                { 'I', ".." },      { 'U', "..-" },     { '7', "--..." },   { ';', "-.-.-." },
                { 'J', ".---" },    { 'V', "...-" },    { '8', "---.." },   { '\"', ".-..-." },
                { 'K', "-.-" },     { 'W', ".--" },     { '9', "----." },   { '-', "-.-.-" },
                { 'L', ".-.." },    { 'X', "-..-" },    { '0', "-----" }  });
    }


    static constexpr uint16_t L2 = ham_package_morse_symbol('L', ".-..");
    static constexpr uint8_t L1 = ham_package_morse_sequence(".-..");

    struct ham_morse_code_standard_table
    {
        static constexpr sklib::supplement::encapsulated_array_type<uint8_t, ham_morse_code_point_range> Codes = create_standard_latin_table();
    };

    struct ham_morse_code_table
    {
        const sklib::supplement::encapsulated_array_type<uint8_t, ham_morse_code_point_range> Codes = create_standard_latin_table();
        constexpr ham_morse_code_table(const std::initializer_list<std::pair<char, const char*>>& config) : Codes(create_table(config)) {}
        constexpr ham_morse_code_table() = default;
    };

    static constexpr auto XYZ = ham_morse_code_table().Codes;
};

