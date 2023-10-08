// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// SKLib is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 2.1 of the License, or (at your option) any
// later version.
//
// SKLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with SKLib.
// If not, see https://www.gnu.org/licenses/

// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// The function stoi() is heavily modified derivative work based on standard C library code for strtol(), credits
// below. See (1). Reference as of 24 June 2022: https://github.com/gcc-mirror/gcc/blob/master/libiberty/strtol.c
//
// The function stod() is influenced by standard C library, as well as SKLib/stoi(), however, its implementation
// is independent.

// Motivation: to make a flavor of standard C functions strtol()/strtod() that can directly accept Unicode and its
// variety, as a template. Enable parsing unsigned integers directly by the same code. Provide no-throw guarantee and
// no-memory-allocation guarantee. (Remark: C++ function std::stoi() implies usage of std::string arguments which can
// pollute the resulting code with memory allocations.)

// Opinions on mixing BSD 3-Clause licensed code with LGPL 2.1 licenced code, which also covers derivative works:
// https://law.stackexchange.com/questions/77460/open-source-project-change-license-from-bsd-3-clause-to-lgpl-2-1
// https://en.wikipedia.org/wiki/GNU_General_Public_License; paragraph Compatibility and multi-licensing
//
// (1) Original notice (also known as BSD-4-Clause-UC) follows. Please note that original BSD license was
// retroactively amended on 22 July 1999, when the copyright holder is the University of California. See (2).
//
// Copyright (c) 1990, 1993  The Regents of the University of California.  All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
// following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
//    disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. [rescinded 22 July 1999, see (2)]
// 4. Neither the name of the University nor the names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// (2) 4.4BSD, BSD-4-Clause-UC, becomes BSD-3-Clause, compatible with LGPL 2.1 or later:
// https://www.freebsd.org/copyright/license/
// https://spdx.org/licenses/BSD-4-Clause-UC.html
// Reference: ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change [Remark: (3)]
//
// July 22, 1999
// To All Licensees, Distributors of Any Version of BSD:
//
// As you know, certain of the Berkeley Software Distribution ("BSD") source code files require that further
// distributions of products containing all or portions of the software, acknowledge within their advertising materials
// that such products contain software developed by UC Berkeley and its contributors.
//
// Specifically, the provision reads:
//   3. All advertising materials mentioning features or use of this software must display the following
//      acknowledgement:
//      This product includes software developed by the University of California, Berkeley and its contributors.
//
// Effective immediately, licensees and distributors are no longer required to include the acknowledgement within
// advertising materials. Accordingly, the foregoing paragraph of those BSD Unix files containing it is hereby deleted
// in its entirety.
//
// William Hoskins
// Director, Office of Technology Licensing
// University of California, Berkeley
//
// (3) As of 24 June 2022, the original amendment notice was not available, however, extensive references were readily
// available with internet search engines.

// This is internal SKLib file and must NOT be included directly.

namespace opaque   // stoi-specific helpers
{
    // Compute the cutoff value between legal numbers and illegal numbers. That is the largest legal value, divided by
    // the base. An input number that is greater than this value, if followed by a legal input character, is too big.
    // One that is equal to this value may be valid or not; the limit between valid and invalid numbers is then based
    // on the last digit. For instance, if the range for longs is [-2147483648..2147483647] and the input base is 10,
    // cutoff will be set to 214748364 and cutlim to either 7 (neg==0) or 8 (neg==1), meaning that if we have
    // accumulated a value > 214748364, or equal but the next digit is > 7 (or 8), the number is too big, the parser
    // stops before the offending digit and returns previous, legal, number.
    //
    // Conversion from large negative integer to unsigned integer of the same size (1 bit wider), avoiding overflow
    // Reference: C++11 Rule Book; as of 24 Jun 2022, https://www.open-std.org/JTC1/SC22/WG14/www/docs/n1570.pdf
    // 6.3.1.3 Signed and unsigned integers, paragraph 2:
    // Otherwise [when signed integer value cannot be represented by unsigned integer type of the same size - Secoh],
    // if the new type is unsigned, the value is converted by repeatedly adding or subtracting one more than the
    // maximum value that can be represented in the new type until the value is in the range of the new type.
    // Example: unsigned int n_abs = UINT_MAX - ((unsigned int)(n)) + 1U
    //
    // While data types shall be all fundamental at this point, keep using template safety on type casting

    template<class target_type>
    struct stoi_bounds_positive_type
    {
        typedef supplement::integer_or_int_type<target_type> bounds_type;
        static constexpr bounds_type typemax = std::numeric_limits<bounds_type>::max();
        static_assert(typemax > 0, "SKLIB ** INTERNAL ERROR ** Maximum value of a numerical type must be positive.");
        static constexpr bounds_type cutmax = (std::is_integral_v<target_type> ? typemax : 0);
        const bounds_type cutoff = 0;
        const int8_t      cutlim = 0;

        constexpr stoi_bounds_positive_type(int8_t base) : cutoff(cutmax / base), cutlim(int8_t(cutmax% base)) {}
    };

    template<class target_type>
    struct stoi_bounds_negative_type
    {
        typedef supplement::integer_or_int_type<target_type> bounds_type;
        typedef make_unsigned_if_integer_type<bounds_type> bounds_unsigned_type;
        static constexpr bounds_type typemin = std::numeric_limits<bounds_type>::min();
        static_assert(std::is_unsigned_v<target_type> || typemin < 0, "SKLIB ** INTERNAL ERROR ** Minimum value of a signed type must be negative.");
        static constexpr bounds_type cutmin = (std::is_integral_v<target_type> ? typemin : 0);
        static constexpr bounds_unsigned_type cutabs = std::numeric_limits<bounds_unsigned_type>::max() - static_cast<bounds_unsigned_type>(typemin) + 1;
        const int8_t      cutlim = 0;   // 1st - used to calculate cutoff below
        const bounds_type cutoff = 0;   // 2nd

        // for cutoff, make division without remainder
        constexpr stoi_bounds_negative_type(int8_t base) : cutlim(int8_t(cutabs% base)), cutoff((cutmin + cutlim) / base) {}
    };

    // turn ASCII letter into integer digit
    template<class letter_type>
    constexpr int8_t stoi_make_digit(letter_type c, int8_t b)
    {
        if (is_num(c))         return (int8_t)(c - '0');
        if (b <= 10)           return b;
        if (is_ascii_upper(c)) return (int8_t)(c - 'A' + 10);
        if (is_ascii_lower(c)) return (int8_t)(c - 'a' + 10);
        return b;
    }

    // common operation of detecting sign
    // remark: caller must set neg = false
    template<class letter_type, class length_type, bool is_signed>
    constexpr void stoi_update_sign(const letter_type* str, bool& neg, length_type& pos)
    {
        if (is_signed && str[pos] == '-')   // negation is undefined for unsigned types
        {
            neg = true;
            pos++;
        }
        else if (str[pos] == '+')
        {
            pos++;
        }
    }

    // take types and bounds, input string and position on it, base, and run "positive" loop
    // starting with pos position
    // return: the digit in accumulator; last valid position plus one; true if success, false if didn't read
    // remark: accumulator shall be reset by caller
    template<class target_type, class letter_type, class length_type>
    constexpr bool stoi_convert_positive(const letter_type* str, target_type& acc, length_type& pos, int8_t base)
    {
        const auto limits = stoi_bounds_positive_type<target_type>(base);

        bool any = false;
        while (true)
        {
            int8_t v = stoi_make_digit(str[pos], base);

            if (v >= base ||
                std::is_integral_v<target_type> && (acc > limits.cutoff || acc == limits.cutoff && v > limits.cutlim)) return any;

            acc = acc * base + v;
            any = true;
            pos++;
        }
    }

    // same as above stoi_convert_positive(), but runs "negative" loop
    template<class target_type, class letter_type, class length_type>
    constexpr bool stoi_convert_negative(const letter_type* str, target_type& acc, length_type& pos, int8_t base)
    {
        const auto limits = stoi_bounds_negative_type<target_type>(base);

        bool any = false;
        while (true)
        {
            int8_t v = stoi_make_digit(str[pos], base);

            if (v >= base ||
                std::is_integral_v<target_type> && (acc < limits.cutoff || acc == limits.cutoff && v > limits.cutlim)) return any;

            acc = acc * base - v;
            any = true;
            pos++;
        }
    }

    // flavor of stoi-like parser, specific to reading fraction after a dot to the floating point target type
    // remark: accumulator shall be (re)set by caller
    template<class target_type, class letter_type, class length_type>
    constexpr bool stod_convert_decimal(const letter_type* str, target_type& acc, length_type& pos, int8_t base)
    {
        bool any = false;
        target_type mult = 1.;
        while (true)
        {
            int8_t v = stoi_make_digit(str[pos], base);
            if (v >= base) return any;

            mult /= base;
            acc += v * mult;
            any = true;
            pos++;
        }
    }
};

//sk TODO add description
//
template<class target_type, class letter_type, class length_type = size_t>
constexpr auto stoi(const letter_type* str, length_type* endpos = nullptr, int8_t base = 0)
{
    typedef std::decay_t<target_type> result_type;

    static_assert(is_numeric_v<result_type>, "SKLIB ** Result of stoi() must be a numerical value");
    static_assert(is_integer_v<letter_type>, "SKLIB ** String letter must be represented by integer value");
    static_assert(is_integer_v<std::decay_t<length_type>>, "SKLIB **- String position/length must be represented by integer value");

    // new: treat invalid base as 0 (no base)
    // still, like in the original code, base=1 is UB and is not handled

    if (base < 0) base = 0;

    // Skip white space and pick up leading +/- sign if any.
    // If base is 0, allow 0x for hex and 0 for octal, else assume decimal; if base is already 16, allow 0x.
    // Add 2-base inputs similar to 16-base ones.

    length_type pos = 0;
    while (is_space(str[pos])) pos++;

    bool neg = false;
    opaque::stoi_update_sign< letter_type, length_type, std::is_signed_v<result_type>>(str, neg, pos);

    // This note and the next one are for legal reasons. All other folks may ignore.
    // Note the pattern "0x", prefix for hexadecimal number, and the code to handle it.
    // The following code belongs to the original Berkeley source:

    if ((!base || base == 16) && str[pos] == '0' && (str[pos+1] == 'x' || str[pos+1] == 'X'))
    {
        pos += 2;
        base = 16;
    }

    // Similar to hexadecimal numbers, ability to input binary numbers directly is considered useful.
    // In similarity to C-like "0x" pattern, the "0b" pattern as a concept was introduced independently by
    // multiple people and various software products, including the C++ standard, to indicate binary digit.
    // The following code is obvious reproduction of the "0x" Berkeley code above, and was copied from it
    // independently:

    else if ((!base || base == 2) && str[pos] == '0' && (str[pos+1] == 'b' || str[pos+1] == 'B'))
    {
        pos += 2;
        base = 2;
    }

    // ---end legal
    // ---by adding restrictive license, you guys just excluded yourselves from the honorable mention

    else if (!base)
    {
        base = (str[pos] == '0') ? 8 : 10;
    }

    // main loop is done externally
    // pos is established by parser; if parser fails, pos is reset

    result_type acc = 0;
    if (!((std::is_signed_v<result_type> && neg)
            ? opaque::stoi_convert_negative(str, acc, pos, base)
            : opaque::stoi_convert_positive(str, acc, pos, base)))
    {
        pos = 0;
    }

    if (endpos) *endpos = pos;
    return acc;
}

// Radix base is discovered automatically using extended C++17 rules
//sk TODO add description
//
template<class target_type, class letter_type, class length_type = size_t>
constexpr std::decay_t<target_type> stod(const letter_type* str, length_type* endpos = nullptr)
{
    typedef std::decay_t<target_type> result_type;

    static_assert(std::is_floating_point_v<result_type>, "SKLIB -- Result of stod() must be floating-point value");
    static_assert(std::is_integral_v<letter_type>, "SKLIB -- String letter must be represented by integer value");
    static_assert(std::is_integral_v<std::decay_t<length_type>>, "SKLIB -- String position/length must be represented by integer value");

    int8_t base = 10;
    length_type pos = 0;

    constexpr char nan[] = "nan";
    constexpr auto nan_len = strlen<int>(nan);

    constexpr char inf[] = "inf";
    constexpr auto inf_len = strlen<int>(inf);

    while (is_space(str[pos])) pos++;

    if ((std::numeric_limits<result_type>::has_quiet_NaN || std::numeric_limits<result_type>::has_signaling_NaN)
        && stranequ(str+pos, nan, nan_len))
    {
        pos += nan_len;
        return (std::numeric_limits<result_type>::has_quiet_NaN
                  ? std::numeric_limits<result_type>::quiet_NaN()
                  : std::numeric_limits<result_type>::signaling_NaN());
    }

    bool neg = false;
    opaque::stoi_update_sign< letter_type, length_type, true>(str, neg, pos);

    if (std::numeric_limits<result_type>::has_infinity && stranequ(str+pos, inf, inf_len))
    {
        pos += inf_len;
        return ((std::numeric_limits<result_type>::is_iec559 && neg)   // crude test whether signed infinity is available
                  ? -std::numeric_limits<result_type>::infinity()      // if not, use "just infinity" for any sign
                  : std::numeric_limits<result_type>::infinity());
    }

    // whole part of mantissa goes first, also providing the exponent base

    if (str[pos] == '0')
    {
        auto cnext = ascii_toupper(str[pos + 1]);
        if (cnext == 'X')
        {
            pos += 2;
            base = 16;
        }
        else if (cnext == 'B')
        {
            // see legal remark above for stoi()
            pos += 2;
            base = 2;
        }
        else if (cnext != '.' && cnext != 'E')
        {
            // two exceptions from 0nnn rule: 0.123 and 0e123 are considered decimal
            base = 8;
        }
    }

    result_type M = 0;
    bool any = sklib::opaque::stoi_convert_positive(str, M, pos, base);

    // if dot is present, read fraction part

    if (str[pos] == '.')
    {
        pos++;
        any = sklib::opaque::stod_convert_decimal(str, M, pos, base) || any;
    }

    // check for exponent which is signed integer
    // single dot without any digits is invalid as number

    if (any)
    {
        int8_t base_exp = 0;
        length_type pos_exp = pos;

        if      (base == 10 && ascii_toupper(str[pos_exp]) == 'E')  base_exp = 10;
        else if (base != 10 && ascii_toupper(str[pos_exp]) == 'P')  base_exp = 2;

        if (base_exp)
        {
            pos_exp++;

            bool neg_exp = false;
            sklib::opaque::stoi_update_sign< letter_type, length_type, true>(str, neg_exp, pos_exp);

            unsigned rise = 0;
            if (sklib::opaque::stoi_convert_positive(str, rise, pos_exp, base))
            {
                pos = pos_exp;
                M = (neg_exp ? M / sklib::upow<result_type>(base_exp, rise) : M * sklib::upow<result_type>(base_exp, rise));
            }
        }
    }
    else
    {
        pos = 0;
    }

    if (endpos) *endpos = pos;
    return (neg ? -M : M);
}


