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

#pragma once

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#endif

#include "./utility.hpp"     // this also loads <type_traits>

namespace sklib
{
    namespace bitwise
    {
        static constexpr uint8_t BYTE_BITS  = 8;
        static constexpr uint8_t INT16_BITS = 2 * BYTE_BITS;
        static constexpr uint8_t INT32_BITS = 4 * BYTE_BITS;
        static constexpr uint8_t INT64_BITS = 8 * BYTE_BITS;

        static constexpr uint8_t BYTE_MASK  = (1 << BYTE_BITS) - 1;
        static constexpr uint8_t BYTE_MAX   = BYTE_MASK;
        static constexpr size_t  BYTE_ADDRESS_SPAN = BYTE_MAX + 1;

#include "./bitwise/tables.hpp"     // load internal reference tables in ::sklib::bitwise::internal::tables - flip, rank

        // -------------------
        // Bits manipulation

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>
        constexpr T low_half_bits() { return T((T(1) << ((BYTE_BITS/2)*sizeof(T))) - 1); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>
        static constexpr T high_half_bits() { return T(low_half_bits<T>() << ((BYTE_BITS/2)*sizeof(T))); }

        // ----------------------------------------------------------
        // FLIP bits in integer, eg write bits in opposite direction

        template<size_t N, class T>
        inline T flip(T data)
        {
            static_assert(sizeof(T) >= N, "Size of data type must be enough to hold specified number of bytes");
            static_assert(N > 0, "Data length in bytes must be positive integer");

            T val = (T)::sklib::bitwise::internal::tables::flip[data & BYTE_MASK];

            for (size_t k = 1; k < N; k++)
            {
                data >>= BYTE_BITS;
                val = (val << BYTE_BITS) | ::sklib::bitwise::internal::tables::flip[data & BYTE_MASK];
            }
            return val;
        }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>
        inline T flip(::sklib::internal::do_not_deduce<T> data) { return flip<sizeof(T), T>(data); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>
        inline T flip_bruteforce(::sklib::internal::do_not_deduce<T> data)
        {
            T R = 0;
            const size_t N = BYTE_BITS * sizeof(T);
            for (size_t i = 0; i < N; i++, data >>= 1) R = T((R << 1) | (data & 1));
            return R;
        }

        inline void generate_table_flip(uint8_t(&U)[BYTE_ADDRESS_SPAN])
        {
            for (size_t k = 0; k < BYTE_ADDRESS_SPAN; k++) U[k] = flip_bruteforce<uint8_t>(uint8_t(k));
        }

        // --------------------------------
        // Calculate RANK of an integer
        // return 1-based position of the most significant bit
        // return 0 if input equals 0

        inline uint8_t rank8(uint8_t v)    { return ::sklib::bitwise::internal::tables::rank[v]; }

        inline uint8_t rank16(uint16_t v)  { return ((v & high_half_bits<uint16_t>()) ? rank8(uint8_t(v>>BYTE_BITS))+BYTE_BITS : rank8(uint8_t(v))); }
        inline uint8_t rank32(uint32_t v)  { return ((v & high_half_bits<uint32_t>()) ? rank16(uint16_t(v>>INT16_BITS))+INT16_BITS : rank16(uint16_t(v))); }
        inline uint8_t rank64(uint64_t v)  { return ((v & high_half_bits<uint64_t>()) ? rank32(uint32_t(v>>INT32_BITS))+INT32_BITS : rank32(uint32_t(v))); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, uint8_t), bool> = true>
        inline uint8_t rank(T v) { return rank8(uint8_t(v)); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, uint16_t), bool> = true>
        inline uint8_t rank(T v) { return rank16(uint16_t(v)); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, uint32_t), bool> = true>
        inline uint8_t rank(T v) { return rank32(uint32_t(v)); }

        template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, uint64_t), bool> = true>
        inline uint8_t rank(T v) { return rank64(uint64_t(v)); }

        inline uint8_t rank8_fork(uint8_t v)
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

        template<class T, size_t N = BYTE_BITS * sizeof(T)>
        inline size_t rank_bruteforce(T data, size_t max_bits_count = N)
        {
            auto udata = std::make_unsigned_t<T>(data);
            for (size_t k=0; k<max_bits_count; k++)
            {
                if (!udata) return k;
                udata >>= 1;
            }
            return max_bits_count;
        }

        inline void generate_table_rank(uint8_t(&U)[BYTE_ADDRESS_SPAN])
        {
            for (size_t k = 0; k < BYTE_ADDRESS_SPAN; k++) U[k] = rank<uint8_t>(uint8_t(k));
        }

        // ---------------------------------------
        // Objects representing series of bits
        // Pack sequence of such objects into sequence of bytes, in MSB mode
        // Unpack byte stream into sequence of objects representing bit packs
        // (bytes are considered MSB; leading bit in the stream corresponds to leading bit in the pack)

        template<size_t N>
        struct bit_cpack_t
        {
            static constexpr size_t size = N;
            uint64_t data;

            bit_cpack_t(uint64_t v) : data(v) {}
            operator uint64_t() { return data; }
        };

        struct bit_pack_t
        {
            uint8_t size;
            uint64_t data;
            operator uint64_t() { return data; }
        };

        class bit_stream_base_t     // using big-endian model
        {
        protected:
            virtual void push_byte(uint8_t /*data*/) {}
            virtual bool pop_byte(uint8_t& /*data*/) { return false; }

            virtual void hook_after_reset()     {}
            virtual void hook_after_flush()     {}
            virtual void hook_before_rewind()   {}

        public:
            void reset()
            {
                accumulator_sender = 0;
                clear_bits_sender = BYTE_BITS;
                accumulator_receiver = 0;
                available_bits_receiver = 0;
                hook_after_reset();
            }

            bit_stream_base_t& operator<< (const bit_pack_t& input)
            {
                auto data_size = input.size;
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
                        clear_bits_sender = BYTE_BITS;
                    }
                }

                return *this;
            }

            template<size_t N>
            bit_stream_base_t& operator<< (const bit_cpack_t<N>& input)
            {
                return operator<< (bit_pack_t{ N, input.data });
            }

            void flush()
            {
                if (clear_bits_sender < BYTE_BITS) push_byte(accumulator_sender << clear_bits_sender);
                clear_bits_sender = BYTE_BITS;
                accumulator_sender = 0;
                hook_after_flush();
            }

            void rewind()
            {
                hook_before_rewind();
                uint8_t accumulator_receiver = 0;
                uint8_t available_bits_receiver = 0;
            }

            bool check(const bit_pack_t& readout)
            {
                if (!readout.size || available_bits_receiver) return true;
                if (!pop_byte(accumulator_receiver)) return false;
                available_bits_receiver = BYTE_BITS;
                return true;
            }

            bit_stream_base_t& operator>> (bit_pack_t& readout)   // size is input, data is output
            {
                auto data_size = readout.size;
                while (data_size)
                {
                    if (!available_bits_receiver)
                    {
                        if (!pop_byte(accumulator_receiver)) accumulator_receiver = 0;
                        available_bits_receiver = BYTE_BITS;
                    }

                    auto load_size = std::min(data_size, available_bits_receiver);
                    available_bits_receiver -= load_size;

                    auto receiver_split = uint16_t(accumulator_receiver << load_size);
                    readout.data = (readout.data << load_size) + (receiver_split >> BYTE_BITS);
                    accumulator_receiver = uint8_t(receiver_split);

                    data_size -= load_size;
                }

                return *this;
            }

            template<size_t N>
            bit_stream_base_t& operator>> (bit_cpack_t<N>& input)
            {
                bit_pack_t R{ N, 0 };
                operator<< (R);
                input.data = R.data;
                return *this;
            }

        private:
            static constexpr uint8_t byte_low_mask[BYTE_BITS + 1] = { 0, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

            uint8_t accumulator_sender = 0;
            uint8_t clear_bits_sender = BYTE_BITS;
            uint8_t accumulator_receiver = 0;
            uint8_t available_bits_receiver = 0;
        };

        class bit_file_t : public bit_stream_base_t
        {
        private:
            std::fstream fs;
            std::ios_base::openmode fs_mode;

            bool can_read() const { return (fs_mode & std::ios_base::in); }
            bool can_write() const { return (fs_mode & std::ios_base::out); }

        public:
            bit_file_t();

            template<class T, std::enable_if_t<SKLIB_TYPES_IS_ANYSTRING(T), bool> = true>
            explicit bit_file_t(const T& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
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
};

