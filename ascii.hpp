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

#ifndef SKLIB_INCLUDED_ASCII_HPP
#define SKLIB_INCLUDED_ASCII_HPP

template<class T>
bool is_alpha_upper(T c)
{
    return (c >= 'A' && c <= 'Z');
}

template<class T>
bool is_alpha_lower(T c)
{
    return (c >= 'a' && c <= 'z');
}

template<class T>
bool is_alpha(T c)
{
    return (is_alpha_upper<T>(c) || is_alpha_lower<T>(c));
}

template<class T>
bool is_num(T c)
{
    return (c >= '0' && c <= '9');
}

template<class T>
bool is_alpha_num(T c)
{
    return (is_alpha<T>(c) || is_num<T>(c));
}

template<class T>
bool is_space(T c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

#endif // SKLIB_INCLUDED_ASCII_HPP
