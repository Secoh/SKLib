// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides constants, macros, and templates for detection of C/C++ types.
// This is internal SKLib file and must NOT be included directly.

namespace supplement
{
    // if integer and not bool, return self; otherwise, return int (as just any random integer type)
    template<class T>
    struct integer_or_int
    {
        typedef typename std::conditional_t<sklib::is_integer_v<T>, T, int> type;
    };

    template<class T> using integer_or_int_type = integer_or_int<T>::type;

    template<class T>
    class make_unsigned_if_integer
    {
    private:
        using T2 = typename std::make_unsigned_t<sklib::supplement::integer_or_int_type<T>>;
    public:
        using type = typename std::conditional_t<sklib::is_integer_v<T>, T2, T>;
    };

    template<class T>
    class make_signed_if_integer
    {
    private:
        using T2 = typename std::make_signed_t<sklib::supplement::integer_or_int_type<T>>;
    public:
        using type = typename std::conditional_t<sklib::is_integer_v<T>, T2, T>;
    };

}; // namespace supplement

// if integer, not bool, provide matching unsigned type, otherwise, leave type unchanged
template<class T>
using make_unsigned_if_integer_type = typename sklib::supplement::make_unsigned_if_integer<T>::type;

// if integer, not bool, provide matching signed type, otherwise, leave type unchanged
// notably, it preserves signed status for floating-point types
template<class T>
using make_signed_if_integer_type = typename sklib::supplement::make_signed_if_integer<T>::type;

// Casts to itself unless T is signed integer type, in which case it returns unsigned value
// Note C++ rule 6.3.1.3 Signed and unsigned integers, paragraph 2:
// It adds (MAX+1) at conversion to any Negative value
template<class T>
constexpr auto to_unsigned_if_integer(const T& v)
{
    return static_cast<make_unsigned_if_integer_type<T>>(v);
}

