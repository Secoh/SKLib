// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides standard character-related functions that can directly accept Unicode and its variety.
// This is internal SKLib file and must NOT be included directly.

template<class T>
constexpr bool is_ascii_upper(T c)
{
    return (c >= 'A' && c <= 'Z');
}

template<class T>
constexpr bool is_ascii_lower(T c)
{
    return (c >= 'a' && c <= 'z');
}

template<class T>
constexpr bool is_ascii_alpha(T c)
{
    return (is_ascii_upper<T>(c) || is_ascii_lower<T>(c));
}

template<class T>
constexpr bool is_num(T c)
{
    return (c >= '0' && c <= '9');
}

template<class T>
constexpr bool is_ascii_alpha_num(T c)
{
    return (is_ascii_alpha<T>(c) || is_num<T>(c));
}

template<class T>
constexpr bool is_space(T c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

template<class T>
constexpr auto ascii_toupper(T c)
{
    static_assert('a' > 'A', "SKLIB ** INTERNAL ERROR ** Alphabet does not start with ASCII, this cannot happen.");
    return (is_ascii_lower(c) ? c - ('a'-'A') : c);
}

template<class T>
constexpr auto ascii_tolower(T c)
{
    static_assert('a' > 'A', "SKLIB ** INTERNAL ERROR ** Alphabet does not start with ASCII, this cannot happen.");
    return (is_ascii_upper(c) ? c + ('a'-'A') : c);
}

