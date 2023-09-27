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
    inline T uidivrem(T A, T B, T* R)
    {
        if (R) *R = A % B;
        return A / B;   //sk TODO: rewrite in explicit assembly form?
    }

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, assuming the quotent fits 64-bit (!)
    // Returns remainder in R, quotent in the name
    inline uint64_t cpu_udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t* R)
    {
#if defined(_MSC_VER) && defined(_M_AMD64) && !defined(_M_ARM64EC)
        return _udiv128(Ahi, Alo, B, R);
#elif defined(__GNUC__) && defined(__x86_64__)
        static_assert(false, "Not implemented yet");
#else
        static_assert(false, "ERROR ** SKLib only supports 128-64 operations on x64 architecture and for GNU C++ or MSVC compilers");
#endif
    }

    // Multiplies two 64-bit unsigned integers into the 128-bit unsigned integer
    inline void cpu_umul128_64(uint64_t A, uint64_t B, uint64_t* Mhi, uint64_t* Mlo)
    {
#if defined(_MSC_VER) && defined(_M_AMD64) && !defined(_M_ARM64EC)
        auto lo = _umul128(A, A, Mhi);
        if (Mlo) *Mlo = lo;
#elif defined(__GNUC__) && defined(__x86_64__)
        static_assert(false, "Not implemented yet");
#endif
    }

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, quotent may exceed 64 bit
    // Returns remainder in R, quotent in Qhi, Qlo
    inline uint64_t udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t& Qhi, uint64_t& Qlo, uint64_t& R)
    {
        Qhi = uidivrem(Ahi, B, &Ahi);
        Qlo = cpu_udiv128_64(Ahi, Alo, B, &R);
    }

    // "Extended" uint limited support
    // data unit to represent addition or multiplication of two uint64 values
    // NB: not intended to support true 128 bit operations

    template<class T, class T_ex, unsigned N_shift>    //sk later: maybe use bitwise
    class uNNex
    {
        static_assert(SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), "Error: uint_extend<> is only defined for unsigned integers");

    private:
        static constexpr T_ex T_carry = (T_ex(1) << N_shift);
        static constexpr T_ex T_mask = T_carry - 1;
        T_ex data = 0;

    public:
        uNNex(T lo) : data(lo) {}
        uNNex(const uNNex&) = default;
        uNNex() = default;

        T hi() const { return T((data >> N_shift) & T_mask); }
        T lo() const { return T(data & T_mask); }

        uNNex& add(T x) { data += x; }

        uNNex& sub(T x, bool carry_hi = false)
        {
            if (carry_hi) data += T_carry - x;
            else data -= x;
        }

        // warning: ignores high portion of the data, if any
        uNNex& mul_lo(T x) { data = (data & T_mask) * x; }

        // special version of divide: ignores high portion of the data,
        // replaces it by "hi" parameter, then makes the divide
        // high portion of the quotent may be nonzero
        uNNex& div_lo(T hi, T x, T* rem = nullptr)
        {
            uidivrem((T_ex(hi) << N_shift) + (data & T_mask), x, rem);
        }
    };

    struct u64ex
    {
        uint64_t Hi=0, Lo=0;

        u64ex(uint64_t data) : Lo(data) {}
        u64ex(const u64ex&) = default;
        u64ex() = default;

        u64ex& operator+= (uint64_t x)
        {}

        u64ex& operator-= (uint64_t x)
        {}

    };

    template<class T> struct uint_extend
    {
        static_assert(SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), "Error: uint_extend<> is only defined for unsigned integers");
        typedef uint16_t type;
    };
    template<> struct uint_extend<uint16_t> { typedef uint32_t type; };
    template<> struct uint_extend<uint32_t> { typedef uint64_t type; };
    template<> struct uint_extend<uint64_t> { typedef u64ex type; };




}; // namespace implementation

