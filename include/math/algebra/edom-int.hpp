// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Functions edivrem() for C/C++ integers.
// This is internal SKLib file and must NOT be included directly.

SKLIB_TEMPLATE_IF_UINT(T) T edivrem(T A, T B, T* R = nullptr)
{
    if (B) return sklib::opaque::uidivrem(A, B, R);

    if (R) *R = 0;
    return 0;
}

SKLIB_TEMPLATE_IF_SINT(T) T edivrem(T A, T B, T* R = nullptr)
{
    T r1 = 0;
    T q1 = 0;
    auto B_neg = (B < 0);

    if (!B)
    {
        if (R) *R = 0;
        return 0;
    }

    if (B_neg) B = -B;

    if (A >= 0)
    {
        q1 = sklib::opaque::uidivrem(A, B, &r1);
    }
    else
    {
        q1= -sklib::opaque::uidivrem(-A, B, &r1);
        if (r1) { q1--; r1=B-r1; }
    }

    if (R) *R = r1;
    return (B_neg ? -q1 : q1);
}

namespace supplement
{
    SKLIB_TEMPLATE_IF_UINT(T) bool e_isnegative(T)   { return false; }
    SKLIB_TEMPLATE_IF_SINT(T) bool e_isnegative(T x) { return (x < 0); }
}; // namespace supplement

