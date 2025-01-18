// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides functions: bits flip, hamming distance, integer rank, bit set/clear/test
// This is internal SKLib file and must NOT be included directly.

// ----------------------------------------------------------
// Flip bits in integer, eg write bits in opposite direction

namespace aux
{
    // word_length is in bits
    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_INT(T, T) bits_flip_bruteforce(sklib::aux::do_not_deduce<T> data, unsigned word_length)
    {
        T R = 0;
        for (unsigned i=0; i<word_length; i++, data >>= 1) R = T((R << 1) | (data & 1));
        return R;
    }

    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_UINT(T, T) bits_flip_bruteforce(sklib::aux::do_not_deduce<T> data)
    {
        return bits_flip_bruteforce<T>(data, sklib::bits_width_v<T>);
    }
};

namespace priv
{
    constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_flip_generate_table()
    {
        sklib::aux::encapsulated_array_octet_index_type<uint8_t> R = { 0 };
        for (size_t k=0; k<sklib::OCTET_ADDRESS_SPAN; k++) R.data[k] = sklib::aux::bits_flip_bruteforce<uint8_t>(uint8_t(k));
        return R;
    }

    inline constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_table_flip = bits_flip_generate_table();
};

namespace aux
{
    template<size_t N_bytes, class T>   //sk modify, if T is integer
    constexpr T bits_flip(T data)
    {
        static_assert(sizeof(T) >= N_bytes, "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bytes");
        static_assert(N_bytes > 0, "Data length in bytes must be positive integer");

        T val = (T)sklib::priv::bits_table_flip.data[data & OCTET_MASK];

        for (size_t k=1; k<N_bytes; k++)
        {
            data >>= sklib::OCTET_BITS;
            val = (val << sklib::OCTET_BITS) | sklib::priv::bits_table_flip.data[data & OCTET_MASK];
        }

        return val;
    }

    constexpr const uint8_t* bits_flip_get_table()
    {
        return sklib::priv::bits_table_flip.data;
    }
};

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_UINT(T, T) bits_flip(sklib::aux::do_not_deduce<T> data)
{
    return sklib::aux::bits_flip<sizeof(T), T>(data);
}

// -----------------------------------------
// Hamming distance between integer and 0
// (between 2 integers - use XOR)

namespace aux
{
    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_UINT(unsigned, T) bits_distance_bruteforce(T data)
    {
        unsigned R = 0;
        constexpr unsigned N = sklib::bits_width_v<T>;
        for (unsigned i=0; i<N; i++, data >>= 1) if (data & 1) R++;
        return R;
    }
};

namespace priv
{
    constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_distance_generate_table()
    {
        sklib::aux::encapsulated_array_octet_index_type<uint8_t> R = { 0 };
        for (size_t k=0; k<sklib::OCTET_ADDRESS_SPAN; k++) R.data[k] = sklib::aux::bits_distance_bruteforce(uint8_t(k));
        return R;
    }

    inline constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_table_distance = bits_distance_generate_table();
};

namespace aux
{
    constexpr const uint8_t* bits_distance_get_table()
    {
        return sklib::priv::bits_table_distance.data;
    }
};

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_UINT(unsigned, T) bits_distance(T data)
{
    unsigned R = 0;
    for (size_t k=0; k<sizeof(T); k++, data >>= sklib::OCTET_BITS) R += sklib::priv::bits_table_distance.data[data & OCTET_MASK];
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

    template<class T, unsigned N = sklib::bits_width_v<T>>
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
    constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_rank_generate_table()
    {
        sklib::aux::encapsulated_array_octet_index_type<uint8_t> R = { 0 };
        for (size_t k=0; k<sklib::OCTET_ADDRESS_SPAN; k++) R.data[k] = sklib::aux::bits_rank_bruteforce(uint8_t(k));
        return R;
    }

    inline constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> bits_table_rank = bits_rank_generate_table();

    constexpr unsigned bits_rank8(uint8_t v)
    {
        return sklib::priv::bits_table_rank.data[v];
    }
    constexpr unsigned bits_rank16(uint16_t v)
    {
        return ((v & sklib::bits_high_half_v<uint16_t>) ? bits_rank8(uint8_t(v >> sklib::OCTET_BITS)) + sklib::OCTET_BITS : bits_rank8(uint8_t(v)));
    }
    constexpr unsigned bits_rank32(uint32_t v)
    {
        constexpr unsigned N_half = sklib::bits_width_v<uint16_t>;
        return ((v & sklib::bits_high_half_v<uint32_t>) ? bits_rank16(uint16_t(v >> N_half)) + N_half : bits_rank16(uint16_t(v)));
    }
    constexpr unsigned rank64(uint64_t v)
    {
        constexpr unsigned N_half = sklib::bits_width_v<uint32_t>;
        return ((v & sklib::bits_high_half_v<uint64_t>) ? bits_rank32(uint32_t(v >> N_half)) + N_half : bits_rank32(uint32_t(v)));
    }
};

namespace aux
{
    constexpr const uint8_t* bits_rank_get_table()
    {
        return sklib::priv::bits_table_rank.data;
    }
};

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint8_t, uint8_t)
bits_rank(T v) { return sklib::priv::bits_rank8(uint8_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint8_t, uint16_t)
bits_rank(T v) { return sklib::priv::bits_rank16(uint16_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint16_t, uint32_t)
bits_rank(T v) { return sklib::priv::bits_rank32(uint32_t(v)); }

template<class T>
constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(unsigned, T, uint32_t, uint64_t)
bits_rank(T v) { return sklib::priv::bits_rank64(uint64_t(v)); }

//sk
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint8_t)  constexpr unsigned bits_rank(T v) { return sklib::priv::bits_rank8(uint8_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint16_t) constexpr unsigned bits_rank(T v) { return sklib::priv::bits_rank16(uint16_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint32_t) constexpr unsigned bits_rank(T v) { return sklib::priv::bits_rank32(uint32_t(v)); }
//SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint64_t) constexpr unsigned bits_rank(T v) { return sklib::priv::bits_rank64(uint64_t(v)); }

// -----------------------------------------
// Bit set, clear, test

namespace priv
{
    template<class T>
    constexpr auto bits_test_generate_table()
    {
        sklib::aux::encapsulated_array_width_index_type<T> R{};
        for (unsigned k=0; k<sklib::bits_width_v<T>; k++) R.data[k] = sklib::bits_data_cap<T>(k);
        return R;
    }

    template<class T> inline constexpr auto bits_table_test = bits_test_generate_table<T>();
};

//sk: verify that it calculates correctly
//constexpr auto internal_test = sklib::priv::bits_table_test<unsigned>;

namespace aux
{
    SKLIB_TEMPLATE_IF_UINT(T) constexpr const T* bits_test_get_table()
    {
        return sklib::priv::bits_table_test<T>.data;
    }
};

SKLIB_TEMPLATE_IF_UINT(T) constexpr bool bit_test(T data, unsigned pos)
{
    return (data & (T(1) << pos));
}

SKLIB_TEMPLATE_IF_UINT(T) constexpr void bit_set(T& data, unsigned pos)
{
    data |= (T(1) << pos);
}

SKLIB_TEMPLATE_IF_UINT(T) constexpr void bit_clear(T& data, unsigned pos)
{
    data &= ~(T(1) << pos);
}

// counterparts with fixed bit position
template<class T, unsigned pos>
constexpr bool bit_test(T data) { return (data & (T(1) << pos)); }

template<class T, unsigned pos>
constexpr void bit_set(T& data) { data |= (T(1) << pos); }

template<class T, unsigned pos>
constexpr void bit_clear(T& data) { data &= ~(T(1) << pos); }

