// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides various supporting functions, mostly hardware-bound.
// This is internal SKLib file and must NOT be included directly.

namespace implementation
{
    // Divides A >= 0 by B > 0 and returns quotent in the name, remainder in R
    // The function attempts to use the fact that hardware DIV instruction computes both in the same time
    // Undefined behavior for A < 0 or B < 0 or B == 0
    template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>
    inline T uidivrem(T A, T B, T& R)
    {
        R = A % B;
        return A / B;
    }

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, assuming the quotent fits 64-bit (!)
    // Returns remainder in R, quotent in the name
    inline uint64_t low_udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t& R)
    {
    }

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, quotent may exceed 64 bit
    // Returns remainder in R, quotent in Qhi, Qlo
    inline uint64_t udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t& Qhi, uint64_t& Qlo, uint64_t& R)
    {
        Qhi = uidivrem(Ahi, B, Ahi);
        Qlo = low_udiv128_64(Ahi, Alo, B, R);
    }

}; // namespace implementation

