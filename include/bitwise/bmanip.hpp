// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_BITWISE_BMANIP_HPP
#define SKLIB_INCLUDED_BITWISE_BMANIP_HPP

//sk  #include <type_traits>

#include "../types.hpp"

namespace sklib {

// Provides functions: bit set/clear/test, bits flip, hamming distance, integer rank
// This is internal SKLib file and must NOT be included directly.

// -----------------------------------------
// Bit set, clear, test

namespace priv
{
    template<class T, typename = void>
    struct select_bits_manip
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Integer bit manipulation is defined only for integer types");
    };

    // signed and unsigned are different for clear() because it has more efficient version for unsigned types
    template<class T> struct sel_bits_manip_common
    {
        static constexpr bool test(T data, unsigned pos)   { return (data & bits_data_cap<T>(pos)); }
        static constexpr void set(T& data, unsigned pos)   { data |= bits_data_cap<T>(pos); }
    };
    template<class T>
    struct select_bits_manip<T, std::enable_if_t<is_signed_native_integer_v<T>>> : public sel_bits_manip_common<T>
    {
        static constexpr void clear(T& data, unsigned pos) { data &= (bits_mask_v<T> & bits_data_cap<T>(pos)); }
    };
    template<class T>
    struct select_bits_manip<T, std::enable_if_t<is_unsigned_native_integer_v<T>>> : public sel_bits_manip_common<T>
    {
        static constexpr void clear(T& data, unsigned pos) { data &= ~bits_data_cap<T>(pos); }
    };

    // for completeness, make up the table of bit positions
    template<class T>
    inline constexpr auto bits_table_test = []()
    {
        aux::encapsulated_array_width_index_type<T> R{};
        for (unsigned k=0; k<R.size; k++) R.data[k] = bits_data_cap<T>(k);
        return R;
    }();
};

template<class T> constexpr bool bit_test (T  data, unsigned pos) { return priv::select_bits_manip<T>::test(data, pos); }
template<class T> constexpr void bit_set  (T& data, unsigned pos) { priv::select_bits_manip<T>::set(data, pos); }
template<class T> constexpr void bit_clear(T& data, unsigned pos) { priv::select_bits_manip<T>::clear(data, pos); }

// counterparts with fixed bit position
template<class T, unsigned pos> constexpr bool bit_test (T  data) { return bit_test<T>(data, pos); }
template<class T, unsigned pos> constexpr void bit_set  (T& data) { bit_set<T>(data, pos); }
template<class T, unsigned pos> constexpr void bit_clear(T& data) { bit_clear<T>(data,  pos); }

namespace aux
{
    template<class T>
    constexpr const T* bits_test_get_table()
    {
        return priv::bits_table_test<T>.data;
    }
};

// ----------------------------------------------------------
// Flip bits in integer, eg write bits in opposite direction

namespace aux
{
    // word_length is in bits
    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_INT(T, T) bits_flip_bruteforce(do_not_deduce<T> data, unsigned word_length)
    {
        T R = 0;
        for (unsigned i=0; i<word_length; i++, data >>= 1) R = T((R << 1) | (data & 1));
        return R;
    }

    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_UINT(T, T) bits_flip_bruteforce(do_not_deduce<T> data)
    {
        return bits_flip_bruteforce<T>(data, bits_width_v<T>);
    }
};

namespace priv
{
    inline constexpr aux::encapsulated_array_octet_index_type<uint8_t> bits_table_flip = []()
    {
        aux::encapsulated_array_octet_index_type<uint8_t> R{};
        for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = aux::bits_flip_bruteforce<uint8_t>(k);
        return R;
    }();
};

namespace aux
{
    template<size_t N_bytes, class T>
    constexpr T bits_flip_span(T data)
    {
        static_assert(is_unsigned_native_integer_v<T>, "SKLIB ERROR ** Byte-wise flip supports only unsigned fundamental integers");
        static_assert(sizeof(T) >= N_bytes, "SKLIB ERROR ** Size of data type must be enough to hold specified number of bytes");
        static_assert(N_bytes > 0, "SKLIB ERROR ** Data length in bytes must be positive integer");

        T val = (T)priv::bits_table_flip.data[data & OCTET_MASK];

        for (size_t k=1; k<N_bytes; k++)
        {
            data >>= OCTET_BITS;
            val = (val << OCTET_BITS) | priv::bits_table_flip.data[data & OCTET_MASK];
        }

        return val;
    }

    constexpr const uint8_t* bits_flip_get_table()
    {
        return priv::bits_table_flip.data;
    }
};

namespace priv
{
    template<class T, typename = void>
    struct select_bits_flip
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Bitwise flip is defined only for unsigned integer types");
    };

    template<class T>
    struct select_bits_flip<T, std::enable_if_t<is_unsigned_native_integer_v<T>>>
    {
        static constexpr T flip(T data) { return aux::bits_flip_span<sizeof(T), T>(data); }
    };
};

template<class T>
constexpr T bits_flip(const aux::do_not_deduce<T>& data)
{
    return priv::select_bits_flip<T>::flip(data);
}

// -----------------------------------------------
// Hamming distance between unsigned integer and 0
// (between 2 integers - use XOR)

namespace aux
{
    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_UINT(unsigned, T) bits_distance_bruteforce(T data)
    {
        unsigned R = 0;
        constexpr unsigned N = bits_width_v<T>;
        for (unsigned i=0; i<N; i++, data >>= 1) if (data & 1) R++;
        return R;
    }
};

namespace priv
{
    inline constexpr aux::encapsulated_array_octet_index_type<uint8_t> bits_table_distance = []()
    {
        aux::encapsulated_array_octet_index_type<uint8_t> R{};
        for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = aux::bits_distance_bruteforce(uint8_t(k));
        return R;
    }();
};

namespace aux
{
    constexpr const uint8_t* bits_distance_get_table()
    {
        return priv::bits_table_distance.data;
    }
};

namespace priv
{
    template<class T, typename = void>
    struct select_bits_distance
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Hamming distance is defined only for unsigned integer types");
    };

    template<class T>
    struct select_bits_flip<T, std::enable_if_t<is_unsigned_native_integer_v<T>>>
    {
        static constexpr T flip(T data) { return aux::bits_flip_span<sizeof(T), T>(data); }
    };
};

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_UINT(unsigned, T) bits_distance(T data)
{
    unsigned R = 0;
    for (size_t k=0; k<sizeof(T); k++, data >>= OCTET_BITS) R += priv::bits_table_distance.data[data & OCTET_MASK];
    return R;
}

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_UINT(unsigned, T) bits_distance(T data1, T data2)    // for completeness
{
    return bits_distance(data1 ^ data2);
}

// --------------------------------
// Calculate RANK of an integer
// return 1-based position of the most significant bit
// return 0 if input equals 0

namespace aux
{
    constexpr unsigned bits_rank8_fork(uint8_t v)
    {
        if (!v) return 0;

        if (v & 0xF0)
        {
            if (v & 0xC0) return ((v & 0x80) ? 8 : 7);
            else          return ((v & 0x20) ? 6 : 5);
        }
        else
        {
            if (v & 0x0C) return ((v & 0x08) ? 4 : 3);
            else          return ((v & 0x02) ? 2 : 1);
        }
    }

    template<class T, unsigned N = bits_width_v<T>>
    constexpr unsigned bits_rank_bruteforce(T data, unsigned max_bits_count = N)
    {
        auto udata = std::make_unsigned_t<T>(data);
        for (unsigned k=0; k<max_bits_count; k++)
        {
            if (!udata) return k;
            udata >>= 1;
        }
        return max_bits_count;
    }
};

namespace priv
{
    constexpr aux::encapsulated_array_octet_index_type<uint8_t> bits_rank_generate_table()
    {
        aux::encapsulated_array_octet_index_type<uint8_t> R = { 0 };
        for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = aux::bits_rank_bruteforce(uint8_t(k));
        return R;
    }

    inline constexpr aux::encapsulated_array_octet_index_type<uint8_t> bits_table_rank = bits_rank_generate_table();

    constexpr unsigned bits_rank8(uint8_t v)
    {
        return priv::bits_table_rank.data[v];
    }
    constexpr unsigned bits_rank16(uint16_t v)
    {
        return ((v & bits_high_half_v<uint16_t>) ? bits_rank8(uint8_t(v >> OCTET_BITS)) + OCTET_BITS : bits_rank8(uint8_t(v)));
    }
    constexpr unsigned bits_rank32(uint32_t v)
    {
        constexpr unsigned N_half = bits_width_v<uint16_t>;
        return ((v & bits_high_half_v<uint32_t>) ? bits_rank16(uint16_t(v >> N_half)) + N_half : bits_rank16(uint16_t(v)));
    }
    constexpr unsigned rank64(uint64_t v)
    {
        constexpr unsigned N_half = bits_width_v<uint32_t>;
        return ((v & bits_high_half_v<uint64_t>) ? bits_rank32(uint32_t(v >> N_half)) + N_half : bits_rank32(uint32_t(v)));
    }
};

namespace aux
{
    constexpr const uint8_t* bits_rank_get_table()
    {
        return priv::bits_table_rank.data;
    }
};

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint8_t, uint8_t)
bits_rank(T v) { return priv::bits_rank8(uint8_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint8_t, uint16_t)
bits_rank(T v) { return priv::bits_rank16(uint16_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint16_t, uint32_t)
bits_rank(T v) { return priv::bits_rank32(uint32_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint32_t, uint64_t)
bits_rank(T v) { return priv::bits_rank64(uint64_t(v)); }

//sk
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint8_t)  constexpr unsigned bits_rank(T v) { return priv::bits_rank8(uint8_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint16_t) constexpr unsigned bits_rank(T v) { return priv::bits_rank16(uint16_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint32_t) constexpr unsigned bits_rank(T v) { return priv::bits_rank32(uint32_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint64_t) constexpr unsigned bits_rank(T v) { return priv::bits_rank64(uint64_t(v)); }

}; // namespace sklib

#endif // SKLIB_INCLUDED_BITWISE_BMANIP_HPP

