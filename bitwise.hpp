// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

// define macro SKLIB_BITWISE_DISABLE_TABLES to help perform certain self tests

// conventions:  count of bits  - unsigned
//               count of bytes - size_t

#ifndef SKLIB_INCLUDED_BITWISE_HPP
#define SKLIB_INCLUDED_BITWISE_HPP

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#endif

#include "./utility.hpp"     // this also loads <type_traits>

namespace sklib
{
    static_assert(sizeof(char) == 1, "SKLIB ** INTERNAL ERROR ** sizeof(char) must be equal to 1");

    // -------------------------------------------------
    // Integer type traits
    // (constants for a given type T if T is integer)

    static constexpr unsigned OCTET_BITS = 8;

    namespace supplement
    {
        //template<class T is integer>
        SKLIB_INTERNAL_FEATURE_IF_INT_T unsigned bits_data_width() { return OCTET_BITS * (unsigned)sizeof(T); }
        SKLIB_INTERNAL_FEATURE_IF_INT_T   T  bits_data_high_1()    { return (T(1) << (bits_data_width<T>()-1)); }
        SKLIB_INTERNAL_FEATURE_IF_INT_T   T  bits_data_mask()      { return (bits_data_high_1<T>() | T(bits_data_high_1<T>()-1)); }

        SKLIB_INTERNAL_FEATURE_IF_UINT_T  T  bits_data_low_half()  { return ((T(1) << (bits_data_width<T>()/2)) - 1); }
        SKLIB_INTERNAL_FEATURE_IF_UINT_T  T  bits_data_high_half() { return (bits_data_low_half<T>() << (bits_data_width<T>()/2)); }
    };

    static constexpr uint8_t OCTET_MASK         = ::sklib::supplement::bits_data_mask<uint8_t>();           //sk? to supplement
    static constexpr size_t  OCTET_ADDRESS_SPAN = OCTET_MASK + 1;

    // --------------------------------
    // Helper/reference tables
    // used for: flip, distance, rank

#include "./bitwise/tables.hpp"

    // ----------------------------------------------------------
    // Flip bits in integer, eg write bits in opposite direction

    namespace supplement
    {
        template<size_t N, class T>
        inline T bits_flip(T data)
        {
            static_assert(sizeof(T) >= N, "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bytes");
            static_assert(N > 0, "Data length in bytes must be positive integer");

            T val = (T)::sklib::internal::tables::flip[data & OCTET_MASK];

            for (size_t k=1; k<N; k++)
            {
                data >>= OCTET_BITS;
                val = (val << OCTET_BITS) | ::sklib::internal::tables::flip[data & OCTET_MASK];
            }

            return val;
        }

        SKLIB_INTERNAL_TEMPLATE_IF_UINT_T T bits_flip_bruteforce(::sklib::internal::do_not_deduce<T> data)
        {
            T R = 0;
            static const unsigned N = ::sklib::supplement::bits_data_width<T>();
            for (unsigned i=0; i<N; i++, data >>= 1) R = T((R << 1) | (data & 1));
            return R;
        }

        void bits_flip_generate_table(uint8_t(&U)[OCTET_ADDRESS_SPAN])
        {
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) U[k] = bits_flip_bruteforce<uint8_t>(uint8_t(k));
        }
    };

    //template<class T is integer>
    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T inline T bits_flip(::sklib::internal::do_not_deduce<T> data)
    {
        return ::sklib::supplement::bits_flip<sizeof(T), T>(data);
    }

    // -----------------------------------------
    // Hamming distance between integer and 0
    // (between 2 integers - use XOR)

    //template<class T>
    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T inline unsigned bits_distance(T data)
    {
        unsigned R = 0;
        for (size_t k=0; k<sizeof(T); k++, data >>= OCTET_BITS) R += ::sklib::internal::tables::distance[data & OCTET_MASK];
        return R;
    }

    //template<class T is integer>
    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T inline unsigned bits_distance(T data1, T data2)    // for completeness
    {
        return bits_distance(data1 ^ data2);
    }

    namespace supplement
    {
        SKLIB_INTERNAL_TEMPLATE_IF_UINT_T unsigned bits_distance_bruteforce(T data)
        {
            unsigned R = 0;
            static const unsigned N = ::sklib::supplement::bits_data_width<T>();
            for (unsigned i=0; i<N; i++, data >>= 1) if (data & 1) R++;
            return R;
        }

        void bits_distance_generate_table(char(&U)[OCTET_ADDRESS_SPAN])
        {
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) U[k] = bits_distance_bruteforce(uint8_t(k));
        }
    };

    // --------------------------------
    // Calculate RANK of an integer
    // return 1-based position of the most significant bit
    // return 0 if input equals 0

    namespace internal
    {
        inline unsigned rank8(uint8_t v)
        {
            return ::sklib::internal::tables::rank[v];
        }
        inline unsigned rank16(uint16_t v)
        {
            return ((v & ::sklib::supplement::bits_data_high_half<uint16_t>()) ? rank8(uint8_t(v >> OCTET_BITS)) + OCTET_BITS : rank8(uint8_t(v)));
        }
        inline unsigned rank32(uint32_t v)
        {
            static const unsigned N_half = ::sklib::supplement::bits_data_width<uint16_t>();
            return ((v & ::sklib::supplement::bits_data_high_half<uint32_t>()) ? rank16(uint16_t(v >> N_half)) + N_half : rank16(uint16_t(v)));
        }
        inline unsigned rank64(uint64_t v)
        {
            static const unsigned N_half = ::sklib::supplement::bits_data_width<uint32_t>();
            return ((v & ::sklib::supplement::bits_data_high_half<uint64_t>()) ? rank32(uint32_t(v >> N_half)) + N_half : rank32(uint32_t(v)));
        }
    };

    //template<class T is integer given size>
    SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(uint8_t)  inline unsigned bits_rank(T v) { return ::sklib::internal::rank8(uint8_t(v)); }
    SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(uint16_t) inline unsigned bits_rank(T v) { return ::sklib::internal::rank16(uint16_t(v)); }
    SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(uint32_t) inline unsigned bits_rank(T v) { return ::sklib::internal::rank32(uint32_t(v)); }
    SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(uint64_t) inline unsigned bits_rank(T v) { return ::sklib::internal::rank64(uint64_t(v)); }

    namespace supplement
    {
        inline unsigned bits_rank8_fork(uint8_t v)
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

        template<class T, unsigned N = bits_data_width<T>()>
        unsigned bits_rank_bruteforce(T data, unsigned max_bits_count = N)
        {
            auto udata = std::make_unsigned_t<T>(data);
            for (unsigned k=0; k<max_bits_count; k++)
            {
                if (!udata) return k;
                udata >>= 1;
            }
            return max_bits_count;
        }

        void bits_rank_generate_table(char(&U)[OCTET_ADDRESS_SPAN])
        {
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) U[k] = bits_rank_bruteforce(uint8_t(k));
        }
    };

    // ---------------------------------------
    // Objects representing series of bits
    // Pack sequence of such objects into sequence of bytes, in MSB mode
    // Unpack byte stream into sequence of objects representing bit packs
    // (bytes are considered MSB; leading bit in the stream corresponds to leading bit in the pack)


// special purpose
#define SKLIB_INTERNAL_TEMPLATE_W_SIZE_I_N_IF_INT_T template<int N, class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>

// https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
#define SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK template<class TT,                                    \
    typename std::enable_if_t<std::is_same_v<std::remove_cv_t<decltype(TT::bit_count)>, unsigned> && \
                              SKLIB_TYPES_IS_INTEGER(decltype(TT::data)), bool> = true>

    namespace supplement
    {
        //template<class T is integer>
        SKLIB_INTERNAL_TEMPLATE_IF_INT_T class bits_variable_pack_type
        {
        public:
            unsigned bit_count;
            T data;

            bits_variable_pack_type(T v, unsigned N = ::sklib::supplement::bits_data_width<T>()) : data(v), bit_count(N) {}
        };

        SKLIB_INTERNAL_TEMPLATE_W_SIZE_I_N_IF_INT_T class bits_fixed_pack_type
        {
            static_assert(N <= ::sklib::supplement::bits_data_width<T>(), "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");

        public:
            static constexpr unsigned bit_count = N;
            T data;

            bits_fixed_pack_type(T v) : data(v) {}
        };
    };

    SKLIB_INTERNAL_TEMPLATE_W_SIZE_I_N_IF_INT_T
    constexpr auto bits_pack(T v)
    { return ::sklib::supplement::bits_fixed_pack_type<N, T>(v); }

    SKLIB_INTERNAL_TEMPLATE_IF_INT_T
    constexpr auto bits_pack(T v, unsigned N = ::sklib::supplement::bits_data_width<T>())
    { return ::sklib::supplement::bits_variable_pack_type<T>(v, N); }

    class bits_stream_base_type     // using big-endian model
    {
    protected:
        virtual void push_byte(uint8_t /*data*/) {}
        virtual bool pop_byte(uint8_t& /*data*/) { return false; }

        virtual void hook_after_reset() {}
        virtual void hook_after_flush() {}
        virtual void hook_before_rewind() {}

    public:
        void reset()
        {
            accumulator_sender = 0;
            clear_bits_sender = OCTET_BITS;
            accumulator_receiver = 0;
            available_bits_receiver = 0;
            hook_after_reset();
        }

        SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK bits_stream_base_type& write(const TT& input)
        {
            auto data_size = input.bit_count;
            while (data_size)
            {
                auto load_size = std::min(data_size, clear_bits_sender);
                data_size -= load_size;

                accumulator_sender = uint8_t((accumulator_sender << load_size) + (uint8_t(input.data >> data_size) & byte_low_mask[load_size]));

                clear_bits_sender -= load_size;
                if (!clear_bits_sender)
                {
                    push_byte(accumulator_sender);
                    accumulator_sender = 0;
                    clear_bits_sender = OCTET_BITS;
                }
            }

            return *this;
        }

        SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK bits_stream_base_type& operator<< (const TT& input) { return write(input); }

        void write_flush()
        {
            if (clear_bits_sender < OCTET_BITS) push_byte(accumulator_sender << clear_bits_sender);
            clear_bits_sender = OCTET_BITS;
            accumulator_sender = 0;
            hook_after_flush();
        }

        void read_rewind()
        {
            hook_before_rewind();
            uint8_t accumulator_receiver = 0;
            uint8_t available_bits_receiver = 0;
        }

        bool can_read(unsigned bit_count)
        {
            if (!bit_count || available_bits_receiver) return true;
            if (!pop_byte(accumulator_receiver)) return false;
            available_bits_receiver = OCTET_BITS;
            return true;
        }

        SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK bool can_read(const TT& request) { return can_read(request.bit_count); }

        SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK bits_stream_base_type& read(TT& request)    // size is input, data is output
        {
            request.data = 0;
            auto data_size = request.bit_count;
            while (data_size)
            {
                if (!available_bits_receiver)
                {
                    if (!pop_byte(accumulator_receiver)) accumulator_receiver = 0;
                    available_bits_receiver = OCTET_BITS;
                }

                auto load_size = std::min(data_size, available_bits_receiver);
                available_bits_receiver -= load_size;

                auto receiver_split = uint16_t(accumulator_receiver << load_size);
                request.data = (request.data << load_size) + (receiver_split >> OCTET_BITS);
                accumulator_receiver = uint8_t(receiver_split);

                data_size -= load_size;
            }

            return *this;
        }

        SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK bits_stream_base_type& operator>> (TT& request) { return read(request); }

    private:
        static constexpr uint8_t byte_low_mask[OCTET_BITS + 1] = { 0, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

        uint8_t  accumulator_sender = 0;
        unsigned clear_bits_sender = OCTET_BITS;
        uint8_t  accumulator_receiver = 0;
        unsigned available_bits_receiver = 0;
    };

    class bits_file_type : public bits_stream_base_type
    {
    private:
        std::fstream fs;
        std::ios_base::openmode fs_mode;

        bool can_read() const { return (fs_mode & std::ios_base::in); }
        bool can_write() const { return (fs_mode & std::ios_base::out); }

    public:
        bits_file_type();

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_ANYSTRING(T), bool> = true>
        explicit bits_file_type(const T& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
            : fs_mode(mode), fs(filename, mode | std::ios_base::binary)
        {
            if (can_read() && can_write() && !fs.is_open())  // extend RW mode: if file doesn't exist, create
            {
                fs.open(filename, std::ios_base::out);
                fs.close();
                fs.open(filename, mode | std::ios_base::binary);
            }
        }

        std::fstream& file_stream() { return fs; }

    protected:
        void hook_after_reset()
        {
            if (can_read()) fs.seekg(0);
            if (can_write()) fs.seekp(0);
        }

        void hook_after_flush()
        {
            if (can_write()) fs.flush();
        }

        void hook_before_rewind()
        {
            if (can_read()) fs.seekg(0);
        }

        void push_byte(uint8_t data)
        {
            char c = data;
            if (can_write()) fs.write(&c, 1);
        }

        bool pop_byte(uint8_t& data)
        {
            if (!can_read() || fs.eof()) return false;

            char c = 0;
            fs.read(&c, 1);
            if (fs.eof()) return false;

            data = c;
            return true;
        }
    };
};

#endif // SKLIB_INCLUDED_BITWISE_HPP
