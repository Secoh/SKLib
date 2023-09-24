// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Function edivrem() for C/C++ integers.
// This is internal SKLib file and must NOT be included directly.

SKLIB_TEMPLATE_IF_INT(T)  T edivrem(T A, T B, T* R = nullptr)
{
    T r1 = 0;
    T q1 = 0;

    if (B)
    {
        if (A >= 0)
        {
            if (B > 0) q1 = sklib::implementation::uidivrem(A, B, r1);
            else       q1 = -sklib::implementation::uidivrem(A, -B, r1);
        }
        else
        {
            if (B > 0) { q1 = -(sklib::implementation::uidivrem(-A, B, r1)+1);  r1=B-r1; }
            else       { q1 = sklib::implementation::uidivrem(-A, -B, r1)+1;    r1=-B-r1; }
        }
    }

    if (R) *R = r1;
    return q1;
}

