// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides the Extended Euclid's Algorithm. Reference: https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
// This is internal SKLib file and must NOT be included directly.

// Finds D=GCD(A,B) in a Euclidean ring, and their Bezout coefficients in form D=A*ka+B*kb
// The following operations must be defined for this function to work:
//   - Euclidean division in form ::sklib::supplement::divmod() (see examples)
//   - comparison to zero: operator!
//   - multiplication: operator*
//   - subtraction: operator-=
// 
// Divides A >= 0 by B > 0 and returns quotent in the name, remainder in R

template<class T>
T bezout(const T& A, const T& B, T& ka, T& kb)
{
    ka=1;
    kb=0;
    int ua=0, ub=1, k=1;
    int *r[2] = { &A, &B };
    int *s[2] = { &ka, &ua };
    int *t[2] = { &kb, &ub };

    while (true)
    {
        int q = ::sklib::edivrem(*r[1-k], *r[k], *r[1-k]);
        if (!*r[1-k]) break;

        *s[1-k] -= q * *s[k];
        *t[1-k] -= q * *t[k];
        k = 1-k;
    }

    if (k) { ka=ua; kb=ub;}
    return *r[k];
}
