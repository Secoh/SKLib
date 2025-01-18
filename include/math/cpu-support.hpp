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

#define SKLIB_INTERNAL_PLATFORM_KNOWN_X64
#if defined(_MSC_VER) && defined(_M_AMD64) && !defined(_M_ARM64EC)
#define SKLIB_INTERNAL_PLATFORM_MSVC_X64
#elif defined(__GNUC__) && defined(__x86_64__)
#define SKLIB_INTERNAL_PLATFORM_GCC_X64
#else
#undef SKLIB_INTERNAL_PLATFORM_KNOWN_X64
//static_assert(false, "ERROR ** SKLib only supports 128-64 operations on x64 architecture and for GNU C++ or MSVC compilers");
#endif

namespace priv
{
    // Divides A >= 0 by B > 0 and returns quotent in the name, remainder in R
    // The function attempts to use the fact that hardware DIV instruction computes both in the same time
    // Undefined behavior for A < 0 or B < 0 or B == 0
    SKLIB_TEMPLATE_IF_INT(T)            //sk template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>
    inline T uidivrem(T A, T B, T* R)
    {
        if (R) *R = A % B;
        return A / B;   //sk TODO: rewrite in explicit assembly form?
    }

    namespace platform_AMD_x64
    {

#ifdef SKLIB_INTERNAL_PLATFORM_KNOWN_X64

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, assuming the quotent fits 64-bit (!)
    // Returns remainder in R, quotent in the name
    inline uint64_t cpu_udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t* R)
    {
#if defined(SKLIB_INTERNAL_PLATFORM_MSVC_X64)
        return _udiv128(Ahi, Alo, B, R);
#elif defined(SKLIB_INTERNAL_PLATFORM_GCC_X64)
        static_assert(false, "Not implemented yet");
#else
        if (R) *R = 0;
        return 0;
#endif
    }

    // Multiplies two 64-bit unsigned integers into the 128-bit unsigned integer
    inline void cpu_umul128_64(uint64_t A, uint64_t B, uint64_t* Mhi, uint64_t* Mlo)
    {
#if defined(SKLIB_INTERNAL_PLATFORM_MSVC_X64)
        auto lo = _umul128(A, B, Mhi);
        if (Mlo) *Mlo = lo;
#elif defined(SKLIB_INTERNAL_PLATFORM_GCC_X64)
        static_assert(false, "Not implemented yet");
#else
        if (Mhi) *Mhi = 0;
        if (Mlo) *Mlo = 0;
#endif
    }

    // Adds two 128-bit unsigned integers
    inline void cpu_uadd128_64(uint64_t Ahi, uint64_t Alo, uint64_t Bhi, uint64_t Blo, uint64_t& Rhi, uint64_t& Rlo)
    {
#if defined(SKLIB_INTERNAL_PLATFORM_MSVC_X64)
        char c = _addcarry_u64(0, Alo, Blo, &Rlo);
        _addcarry_u64(c, Ahi, Bhi, &Rhi);
#elif defined(SKLIB_INTERNAL_PLATFORM_GCC_X64)
        static_assert(false, "Not implemented yet");
#else
        Rhi = 0;
        Rlo = 0;
#endif
    }

    // Subtracts two 128-bit unsigned integers, R := A - B
    inline void cpu_usub128_64(uint64_t Ahi, uint64_t Alo, uint64_t Bhi, uint64_t Blo, uint64_t& Rhi, uint64_t& Rlo)
    {
#if defined(SKLIB_INTERNAL_PLATFORM_MSVC_X64)
        char c = _subborrow_u64(0, Alo, Blo, &Rlo);
        _subborrow_u64(c, Ahi, Bhi, &Rhi);
#elif defined(SKLIB_INTERNAL_PLATFORM_GCC_X64)
        static_assert(false, "Not implemented yet");
#else
        Rhi = 0;
        Rlo = 0;
#endif
    }

#else // not SKLIB_INTERNAL_PLATFORM_KNOWN_X64

    // AMDx64-specific functions are always declared so compilation always succeed
    // Linker will succeed only the target is AMDx64
    // Any program that doesn't use these symbols will compile for any platform;
    // Anything that uses them requires x64 to compile
    uint64_t cpu_udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t* R);
    void cpu_umul128_64(uint64_t A, uint64_t B, uint64_t* Mhi, uint64_t* Mlo);
    void cpu_uadd128_64(uint64_t Ahi, uint64_t Alo, uint64_t Bhi, uint64_t Blo, uint64_t& Rhi, uint64_t& Rlo);
    void cpu_usub128_64(uint64_t Ahi, uint64_t Alo, uint64_t Bhi, uint64_t Blo, uint64_t& Rhi, uint64_t& Rlo);

#endif // SKLIB_INTERNAL_PLATFORM_KNOWN_X64

    }; // namespace platform_AMD_x64

    // Divides 128-bit unsigned integer by 64-bit unsigned integer, quotent may exceed 64 bit
    // Returns remainder in R, quotent in Qhi, Qlo
    inline uint64_t udiv128_64(uint64_t Ahi, uint64_t Alo, uint64_t B, uint64_t& Qhi, uint64_t& Qlo, uint64_t& R)
    {
        Qhi = uidivrem(Ahi, B, &Ahi);
        Qlo = sklib::priv::platform_AMD_x64::cpu_udiv128_64(Ahi, Alo, B, &R);
    }

    // "Extended" uint limited support
    // data unit to represent addition or multiplication of two uint64 values
    // NB: not intended to support true 128 bit operations

    template<class T, class T_ex>
    class uNNex
    {
        static_assert(sklib::is_unsigned_integer_v<T>, "Error: uint_extend<> is only defined for unsigned integers");

    private:
        static constexpr T_ex T_carry = (T_ex(1) << sklib::bits_width_v<T>);
        static constexpr T_ex T_mask = T_carry - 1;
        T_ex data = 0;

    public:
        uNNex(T lo) : data(lo) {}
        uNNex(T hi, T lo) : data((T_ex(hi) << sklib::bits_width_v<T>) + lo) {}
        uNNex(const uNNex&) = default;
        uNNex() = default;

        T hi() const { return T((data >> sklib::bits_width_v<T>) & T_mask); }
        T lo() const { return T(data & T_mask); }

        explicit operator bool() { return bool(data); }

        uNNex& add(const uNNex& X)
        {
            data += X.data;
            return *this;
        }

        uNNex& sub(const uNNex& X)
        {
            data -= X.data;
            return *this;
        }

        // special multiplication
        // ignores high portion of the data, if any
        uNNex& mul(T x)
        {
            data = (data & T_mask) * x;
            return *this;
        }

        // special divide - uses high portion of dividend, however,
        // high portion of the quotent MUST be zero, otherwise UB
        uNNex& div(T x, T* rem = nullptr)
        {
            T_ex rem1;
            data = uidivrem<T_ex>(data, x, &rem1);
            if (rem) *rem = T(rem1);
            return *this;
        }
    };

    class u64ex
    {
    private:
        uint64_t data_hi=0, data_lo=0;

    public:
        u64ex(uint64_t lo) : data_lo(lo) {}
        u64ex(uint64_t hi, uint64_t lo) : data_hi(hi), data_lo(lo) {}
        u64ex(const u64ex&) = default;
        u64ex() = default;

        uint64_t hi() const { return data_hi; }
        uint64_t lo() const { return data_lo; }

        explicit operator bool() { return bool(data_hi) || bool(data_lo); }

        u64ex& add(const u64ex& X)
        {
            sklib::priv::platform_AMD_x64::cpu_uadd128_64(data_hi, data_lo, X.data_hi, X.data_lo, data_hi, data_lo);
            return *this;
        }

        u64ex& sub(const u64ex& X)
        {
            sklib::priv::platform_AMD_x64::cpu_usub128_64(data_hi, data_lo, X.data_hi, X.data_lo, data_hi, data_lo);
            return *this;
        }

        // special multiplication
        // ignores high portion of the data, if any
        u64ex& mul(uint64_t x)
        {
            sklib::priv::platform_AMD_x64::cpu_umul128_64(data_lo, x, &data_hi, &data_lo);
            return *this;
        }

        // special divide - uses high portion of dividend, however,
        // high portion of the quotent MUST be zero, otherwise UB
        u64ex& div(uint64_t x, uint64_t* rem = nullptr)
        {
            data_lo = sklib::priv::platform_AMD_x64::cpu_udiv128_64(data_hi, data_lo, x, rem);
            data_hi = 0;
            return *this;
        }

    };

    template<class T> struct uint_extend
    {
        //SKLIB_TYPES_IS_UNSIGNED_INTEGER(T)
        static_assert(sklib::is_unsigned_integer_v<T>, "Error: uint_extend<> is only defined for unsigned integers");
        typedef uNNex<uint8_t, uint16_t> type;
    };
    template<> struct uint_extend<uint16_t> { typedef uNNex<uint16_t, uint32_t> type; };
    template<> struct uint_extend<uint32_t> { typedef uNNex<uint32_t, uint64_t> type; };
    template<> struct uint_extend<uint64_t> { typedef u64ex type; };

    template<class T> using uint_extend_t = uint_extend<T>::type;


}; // namespace priv

