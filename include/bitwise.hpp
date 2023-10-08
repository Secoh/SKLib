// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

// conventions:  count of bits  - unsigned
//               count of bytes - size_t

#ifndef SKLIB_INCLUDED_BITWISE_HPP
#define SKLIB_INCLUDED_BITWISE_HPP

#include <utility>
#include <iostream>
#include <fstream>
#include <string>

#include "types.hpp"

namespace sklib
{
    // --------------------------------
    // Helper/reference tables
    // used for: flip, distance, rank, distance, base64

    namespace opaque
    {
        template<class T, size_t N>         //sk ?! can replace with std::array?
        struct encapsulated_array_type
        {
            T data[N];
        };


        SKLIB_TEMPLATE_IF_INT(T) using encapsulated_array_octet_index_type = encapsulated_array_type<T, OCTET_ADDRESS_SPAN>;
//sk            struct encapsulated_array_octet_index_type
//        {
//            T data[OCTET_ADDRESS_SPAN];
//        };
    };

    // ----------------------------------------------------------
    // Flip bits in integer, eg write bits in opposite direction

    namespace supplement
    {
        // word_length is in bits
        SKLIB_TEMPLATE_IF_INT(T) static constexpr T bits_flip_bruteforce(::sklib::supplement::do_not_deduce<T> data, unsigned word_length)
        {
            T R = 0;
            for (unsigned i=0; i<word_length; i++, data >>= 1) R = T((R << 1) | (data & 1));
            return R;
        }

        SKLIB_TEMPLATE_IF_UINT(T) static constexpr T bits_flip_bruteforce(::sklib::supplement::do_not_deduce<T> data)
        {
            return bits_flip_bruteforce<T>(data, sklib::bits_width_v<T>);
        }
    };

    namespace opaque
    {
        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_flip_generate_table()
        {
            encapsulated_array_octet_index_type<uint8_t> R = { 0 };
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = ::sklib::supplement::bits_flip_bruteforce<uint8_t>(uint8_t(k));
            return R;
        }

        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_table_flip = bits_flip_generate_table();
    };

    namespace supplement
    {
        template<size_t N_bytes, class T>
        static constexpr T bits_flip(T data)
        {
            static_assert(sizeof(T) >= N_bytes, "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bytes");
            static_assert(N_bytes > 0, "Data length in bytes must be positive integer");

            T val = (T)::sklib::opaque::bits_table_flip.data[data & OCTET_MASK];

            for (size_t k=1; k<N_bytes; k++)
            {
                data >>= OCTET_BITS;
                val = (val << OCTET_BITS) | ::sklib::opaque::bits_table_flip.data[data & OCTET_MASK];
            }

            return val;
        }

        static constexpr const uint8_t* bits_flip_get_table()
        {
            return ::sklib::opaque::bits_table_flip.data;
        }
    };

    SKLIB_TEMPLATE_IF_UINT(T) static constexpr T bits_flip(::sklib::supplement::do_not_deduce<T> data)
    {
        return ::sklib::supplement::bits_flip<sizeof(T), T>(data);
    }

    // -----------------------------------------
    // Hamming distance between integer and 0
    // (between 2 integers - use XOR)

    namespace supplement
    {
        SKLIB_TEMPLATE_IF_UINT(T) static constexpr unsigned bits_distance_bruteforce(T data)
        {
            unsigned R = 0;
            constexpr unsigned N = sklib::bits_width_v<T>;
            for (unsigned i=0; i<N; i++, data >>= 1) if (data & 1) R++;
            return R;
        }
    };

    namespace opaque
    {
        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_distance_generate_table()
        {
            encapsulated_array_octet_index_type<uint8_t> R = { 0 };
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = ::sklib::supplement::bits_distance_bruteforce(uint8_t(k));
            return R;
        }

        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_table_distance = bits_distance_generate_table();
    };

    namespace supplement
    {
        static constexpr const uint8_t* bits_distance_get_table()
        {
            return ::sklib::opaque::bits_table_distance.data;
        }
    };

    SKLIB_TEMPLATE_IF_UINT(T) static constexpr unsigned bits_distance(T data)
    {
        unsigned R = 0;
        for (size_t k=0; k<sizeof(T); k++, data >>= OCTET_BITS) R += ::sklib::opaque::bits_table_distance.data[data & OCTET_MASK];
        return R;
    }

    SKLIB_TEMPLATE_IF_UINT(T) static constexpr unsigned bits_distance(T data1, T data2)    // for completeness
    {
        return bits_distance(data1 ^ data2);
    }

    // --------------------------------
    // Calculate RANK of an integer
    // return 1-based position of the most significant bit
    // return 0 if input equals 0

    namespace supplement
    {
        static constexpr unsigned bits_rank8_fork(uint8_t v)
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
        static constexpr unsigned bits_rank_bruteforce(T data, unsigned max_bits_count = N)
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

    namespace opaque
    {
        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_rank_generate_table()
        {
            encapsulated_array_octet_index_type<uint8_t> R = { 0 };
            for (size_t k=0; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = ::sklib::supplement::bits_rank_bruteforce(uint8_t(k));
            return R;
        }

        static constexpr encapsulated_array_octet_index_type<uint8_t> bits_table_rank = bits_rank_generate_table();

        static constexpr unsigned rank8(uint8_t v)
        {
            return ::sklib::opaque::bits_table_rank.data[v];
        }
        static constexpr unsigned rank16(uint16_t v)
        {
            return ((v & sklib::bits_high_half_v<uint16_t>) ? rank8(uint8_t(v >> OCTET_BITS)) + OCTET_BITS : rank8(uint8_t(v)));
        }
        static constexpr unsigned rank32(uint32_t v)
        {
            constexpr unsigned N_half = sklib::bits_width_v<uint16_t>;
            return ((v & sklib::bits_high_half_v<uint32_t>) ? rank16(uint16_t(v >> N_half)) + N_half : rank16(uint16_t(v)));
        }
        static constexpr unsigned rank64(uint64_t v)
        {
            constexpr unsigned N_half = sklib::bits_width_v<uint32_t>;
            return ((v & sklib::bits_high_half_v<uint64_t>) ? rank32(uint32_t(v >> N_half)) + N_half : rank32(uint32_t(v)));
        }
    };

    namespace supplement
    {
        static constexpr const uint8_t* bits_rank_get_table()
        {
            return ::sklib::opaque::bits_table_rank.data;
        }
    };

    SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint8_t)  static constexpr unsigned bits_rank(T v) { return ::sklib::opaque::rank8(uint8_t(v)); }
    SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint16_t) static constexpr unsigned bits_rank(T v) { return ::sklib::opaque::rank16(uint16_t(v)); }
    SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint32_t) static constexpr unsigned bits_rank(T v) { return ::sklib::opaque::rank32(uint32_t(v)); }
    SKLIB_TEMPLATE_IF_INT_OF_SIZE(T, uint64_t) static constexpr unsigned bits_rank(T v) { return ::sklib::opaque::rank64(uint64_t(v)); }

    // ---------------------------------------
    // Objects representing series of bits
    // Pack sequence of such objects into sequence of bytes, in MSB mode
    // Unpack byte stream into sequence of objects representing bit packs
    // (bytes are considered MSB; leading bit in the stream corresponds to leading bit in the pack)

// // https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
//#define SKLIB_INTERNAL_TEMPLATE_TT_IS_BIT_PACK template<class TT,                                    \
//    typename std::enable_if_t<std::is_same_v<std::remove_cv_t<decltype(TT::bit_count)>, unsigned> && \
//                              sklib::is_integer_v<decltype(TT::data)>, bool> = true>

    namespace opaque
    {
        struct bits_variable_pack_anchor {};
    };

    namespace supplement
    {
        SKLIB_TEMPLATE_IF_INT(T) class bits_variable_pack_type : public sklib::opaque::bits_variable_pack_anchor
        {
        public:
            unsigned bit_count;
            T data;

            bits_variable_pack_type(T v, unsigned N = sklib::bits_width_v<T>) : data(v), bit_count(N) {}
        };

        template<int N, class T, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
        class bits_fixed_pack_type : public sklib::opaque::bits_variable_pack_anchor
        {
            static_assert(N <= sklib::bits_width_v<T>,
                "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");

        public:
            static constexpr unsigned bit_count = N;
            T data;

            bits_fixed_pack_type(T v) : data(v) {}
        };
    };

    template<int N, class T, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
    constexpr auto bits_pack(T v)
    { return ::sklib::supplement::bits_fixed_pack_type<N, T>(v); }

    SKLIB_TEMPLATE_IF_INT(T)
    constexpr auto bits_pack(T v, unsigned N = sklib::bits_width_v<T>)
    { return ::sklib::supplement::bits_variable_pack_type<T>(v, N); }

    class bits_stream_base_type     // using big-endian model
    {
    protected:
        enum class hook_type { after_reset = 0, after_flush, before_rewind };

    private:
        sklib::supplement::callback_type<bits_stream_base_type, bool, uint8_t&> read_octet;
        sklib::supplement::callback_type<bits_stream_base_type, void, uint8_t> write_octet;
        sklib::supplement::callback_type<bits_stream_base_type, void, hook_type> hook_action;

    public:
        bits_stream_base_type(bool (*read_octet_callback)(bits_stream_base_type*, uint8_t&),         // derived class provides function to read next octet from stream
                              void (*write_octet_callback)(bits_stream_base_type*, uint8_t),         // write into stream
                              void (*hook_callback)(bits_stream_base_type*, hook_type) = nullptr)    // stream-related events
            : read_octet(read_octet_callback, this)
            , write_octet(write_octet_callback, this)
            , hook_action(hook_callback, this)
        {}

        bits_stream_base_type(void* external_descriptor,
                              bool (*read_octet_callback)(void*, uint8_t&),         // version for payload at void pointer
                              void (*write_octet_callback)(void*, uint8_t),
                              void (*hook_callback)(void*, hook_type) = nullptr)
            : read_octet(read_octet_callback, external_descriptor)
            , write_octet(write_octet_callback, external_descriptor)
            , hook_action(hook_callback, external_descriptor)
        {}

        bits_stream_base_type(bool (*read_octet_callback)(uint8_t&),                // version for global C functions
                              void (*write_octet_callback)(uint8_t),
                              void (*hook_callback)(hook_type) = nullptr)
            : read_octet(read_octet_callback)
            , write_octet(write_octet_callback)
            , hook_action(hook_callback)
        {}

        void reset()
        {
            accumulator_sender = 0;
            clear_bits_sender = OCTET_BITS;
            accumulator_receiver = 0;
            available_bits_receiver = 0;
            if (hook_action) hook_action(hook_type::after_reset);
        }

        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bits_stream_base_type& write(const TT& input)
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
                    if (write_octet) write_octet(accumulator_sender);
                    accumulator_sender = 0;
                    clear_bits_sender = OCTET_BITS;
                }
            }

            return *this;
        }

        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bits_stream_base_type& operator<< (const TT& input) { return write(input); }

        void write_flush()
        {
            if (clear_bits_sender < OCTET_BITS && write_octet) write_octet(accumulator_sender << clear_bits_sender);
            clear_bits_sender = OCTET_BITS;
            accumulator_sender = 0;
            if (hook_action) hook_action(hook_type::after_flush);
        }

        void read_rewind()
        {
            if (hook_action) hook_action(hook_type::before_rewind);
            uint8_t accumulator_receiver = 0;
            uint8_t available_bits_receiver = 0;
        }

        // true if internal storage has enough data for the next read
        bool can_read_without_input_stream(unsigned bit_count) const
        {
            return (bit_count <= available_bits_receiver);
        }
        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bool can_read_without_input_stream(const TT& request) const
        { return can_read_without_input_stream(request.bit_count); }

        // true if subsequent read() can return valid data
        // even when input stream has ended
        bool read_has_residual_data() const
        { return can_read_without_input_stream(1); }

        // true if there is some data in the stream (beware: trailing bits may be truncated)
        bool can_read(unsigned bit_count)
        {
            if (!bit_count || available_bits_receiver) return true;
            if (!read_octet || !read_octet(accumulator_receiver)) return false;
            available_bits_receiver = OCTET_BITS;
            return true;
        }

        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bool can_read(const TT& request) { return can_read(request.bit_count); }

        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bits_stream_base_type& read(TT& request)    // size is input, data is output
        {
            request.data = 0;
            auto data_size = request.bit_count;
            while (data_size)
            {
                if (!available_bits_receiver)
                {
                    if (!read_octet || !read_octet(accumulator_receiver)) accumulator_receiver = 0;
                    available_bits_receiver = OCTET_BITS;
                }

                auto load_size = std::min(data_size, available_bits_receiver);
                available_bits_receiver -= load_size;

                // this arrangement guarantees that bits higher than bit_count will be 0
                auto receiver_split = uint16_t(accumulator_receiver << load_size);
                request.data = (request.data << load_size) + (receiver_split >> OCTET_BITS);
                accumulator_receiver = uint8_t(receiver_split);

                data_size -= load_size;
            }

            return *this;
        }

        SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
        bits_stream_base_type& operator>> (TT& request) { return read(request); }

    protected:
        static constexpr uint8_t byte_low_mask[OCTET_BITS + 1] = { 0,
            sklib::bits_data_mask_v<uint8_t, 1>,
            sklib::bits_data_mask_v<uint8_t, 2>,
            sklib::bits_data_mask_v<uint8_t, 3>,
            sklib::bits_data_mask_v<uint8_t, 4>,
            sklib::bits_data_mask_v<uint8_t, 5>,
            sklib::bits_data_mask_v<uint8_t, 6>,
            sklib::bits_data_mask_v<uint8_t, 7>,
            sklib::bits_data_mask_v<uint8_t, 8> };

        uint8_t  accumulator_sender = 0;
        unsigned clear_bits_sender = OCTET_BITS;
        uint8_t  accumulator_receiver = 0;
        unsigned available_bits_receiver = 0;
    };

    // -------------------------------------------------------------------
    // Application of bit stream abstratction - file based per-bit I/O

//TODO: must be "open" member function

    class bits_file_type : public bits_stream_base_type
    {
    private:
        std::fstream fs;
        std::ios_base::openmode fs_mode;

        bool is_readable() const  { return (fs_mode & std::ios_base::in); }
        bool is_writeable() const { return (fs_mode & std::ios_base::out); }

    public:
        template<class T, std::enable_if_t<sklib::is_any_string<T>, bool> = true>
        explicit bits_file_type(const T& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
            : bits_stream_base_type(read_byte_proc, write_byte_proc, stream_action)
            , fs_mode(mode)
            , fs(filename, mode | std::ios_base::binary)
        {
            if (is_readable() && is_writeable() && !fs.is_open())  // extend RW mode: if file doesn't exist, create
            {
                fs.open(filename, std::ios_base::out);
                fs.close();
                fs.open(filename, mode | std::ios_base::binary);
            }
        }

        std::fstream& file_stream() { return fs; }

    private:
        bool redirect_read_byte(uint8_t& data)
        {
            if (!is_readable() || fs.eof()) return false;

            char c = 0;
            fs.read(&c, 1);
            if (fs.eof()) return false;

            data = c;
            return true;
        }
        static bool read_byte_proc(bits_stream_base_type* root, uint8_t& data)
        {
            return static_cast<bits_file_type*>(root)->redirect_read_byte(data);
        }

        void redirect_write_byte(uint8_t data)
        {
            char c = data;
            if (is_writeable()) fs.write(&c, 1);
        }
        static void write_byte_proc(bits_stream_base_type* root, uint8_t data)
        {
            static_cast<bits_file_type*>(root)->redirect_write_byte(data);
        }

        void action_after_reset()
        {
            if (is_readable()) fs.seekg(0);
            if (is_writeable()) fs.seekp(0);
        }
        void action_after_flush()
        {
            if (is_writeable()) fs.flush();
        }
        void action_before_rewind()
        {
            if (is_readable()) fs.seekg(0);
        }
        static void stream_action(bits_stream_base_type* root, hook_type what)
        {
            auto self = static_cast<bits_file_type*>(root);

            switch (what)
            {
            case hook_type::after_reset:   self->action_after_reset();   break;
            case hook_type::after_flush:   self->action_after_flush();   break;
            case hook_type::before_rewind: self->action_before_rewind(); //break;
            }
        }
    };

    // ---------------------------------------------------
    // base64 I/O on top of bits_stream_base_type class

    namespace opaque
    {
        class base64_property_type
        {
            friend constexpr encapsulated_array_octet_index_type<uint8_t> generate_dictionary_inverse_table();

        public:
            static constexpr int encoding_bit_length = 6;
            static constexpr size_t dictionary_size = (1 << encoding_bit_length);
            static constexpr uint8_t dictionary_address_mask = sklib::bits_data_mask_v<uint8_t, encoding_bit_length>; // same as (dictionary_size-1)
            static constexpr char dictionary[dictionary_size+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            static constexpr char EOL_char = '=';

        protected:
            // special Inverse/Decode table entries (see: Tables) to handle non-dictionary characters on input
            // b64_dictionary_inverse[input_ch] => output data, or soecial code as follows:
            static constexpr uint8_t EOL_code   = 0xF9;     // input was EOL (EOF, EOT, end of data stream, etc)
            static constexpr uint8_t Space_code = 0xF0;     // Space or Blank ASCII character
            static constexpr uint8_t Bad_code   = 0xFF;     // invalid input
        };

        constexpr encapsulated_array_octet_index_type<uint8_t> generate_dictionary_inverse_table()
        {
            encapsulated_array_octet_index_type<uint8_t> R = { 0 };
            for (int k=0; k<=' '; k++) R.data[k] = base64_property_type::Space_code;
            for (int k=' '+1; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = base64_property_type::Bad_code;
            for (size_t k=0; k< base64_property_type::dictionary_size; k++) R.data[base64_property_type::dictionary[k]] = (uint8_t)k;
            R.data[base64_property_type::EOL_char] = base64_property_type::EOL_code;
            return R;
        }

        static constexpr encapsulated_array_octet_index_type<uint8_t> b64_dictionary_inverse = generate_dictionary_inverse_table();
    };

    class base64_type : protected bits_stream_base_type, public ::sklib::opaque::base64_property_type
    {                      // make underlying members hidden from caller but allow further class derivation, include global constants
    protected:
        // octet (or character) I/O is communicated as integers, just like as ANSI C does
        // lets note that we are using special "character" for EOF = -1, inherited from C language
        // internally, lets have designation for "idle state" - the "idle" shall be never returned to caller!
        static constexpr int Idle_char = EOF-1;
        static_assert(EOF < 0, "SKLIB ** INTERNAL ERROR ** EOF must be negative");
        static_assert(Idle_char < 0, "SKLIB ** INTERNAL ERROR ** EOF-1 must be negative");

    public:
        static constexpr size_t raw_data_block_size = 3;
        static constexpr size_t encoded_block_size = 4;

        static constexpr size_t encoded_length(size_t raw_data_input_length)
        {
            size_t blocks_count = raw_data_input_length / raw_data_block_size;
            size_t tail_length = raw_data_input_length % raw_data_block_size;     // { 0, 1, 2 } => { 0, 1, 2 }
            return blocks_count * encoded_block_size + tail_length;
        }

        static constexpr size_t decoded_length(size_t encoded_input_length)
        {
            size_t blocks_count = encoded_input_length / encoded_block_size;
            size_t tail_length = encoded_input_length % encoded_block_size;
            if (tail_length) tail_length--;                                     // { 0, 1, 2, 3 } => { 0, 0, 1, 2 }
            return blocks_count * raw_data_block_size + tail_length;
        }

    protected:
        bool encoder_errors = false;

        int pending_data_to_encode = Idle_char;
        static bool read_encoded_proc(bits_stream_base_type* root, uint8_t& data)
        {
            int d = static_cast<base64_type*>(root)->pending_data_to_encode;
            data = (d < 0 ? 0 : (uint8_t)d);
            return (d >= 0);
        }

        int decoded_data_accumulator = Idle_char;
        static void write_decoded_proc(bits_stream_base_type* root, uint8_t data)
        {
            static_cast<base64_type*>(root)->decoded_data_accumulator = data;
        }

    private:
        sklib::supplement::callback_type<base64_type, bool, int&> read_proc;
        sklib::supplement::callback_type<base64_type, void, int> write_proc;

        //bool (* const read_proc)(base64_type*, int&);
        //void (* const write_proc)(base64_type*, int);

        ::sklib::supplement::bits_fixed_pack_type<encoding_bit_length, uint8_t> exchg{ 0 };

    public:
        // This class mimics C functions to read from, and write to, a stream
        // while encoding or decoding Base64 format. Four modes are supported:
        // - read_encode, underlying input is raw data, caller receives bas64 encoding;
        // - read_decode, original input is in base_64, caller receives decoded data;
        // - write_encode, caller sends raw data and it gets converted to base64 in the output;
        // - write_decode, caller sends base64 symbols, and raw data is written to output stream.
        // Encoder uses callback functions to read and write bytes (0..255 range),
        // as well as to communicate EOF condition by negative data value.
        //
        base64_type(bool (*read_callback)(base64_type*, int&),
                    void (*write_callback)(base64_type*, int))
            : read_proc(read_callback, this)
            , write_proc(write_callback, this)
            , bits_stream_base_type(read_encoded_proc, write_decoded_proc)
        {}

        base64_type(void *external_descriptor,
                    bool (*read_callback)(void*, int&),
                    void (*write_callback)(void*, int))
            : read_proc(read_callback, external_descriptor)
            , write_proc(write_callback, external_descriptor)
            , bits_stream_base_type(read_encoded_proc, write_decoded_proc)
        {}

        base64_type(bool (*read_callback)(int&),
                    void (*write_callback)(int))
            : read_proc(read_callback)
            , write_proc(write_callback)
            , bits_stream_base_type(read_encoded_proc, write_decoded_proc)
        {}

        // returns TRUE if any encoding/decoding error was seen since last call
        // clears internal error counter
        //
        bool have_errors()
        {
            bool R = encoder_errors;
            encoder_errors = false;
            return R;
        }

        // resets the state of the class (and the encoder) to idle state
        // same as just after constructor completion
        //
        void reset()
        {
            bits_stream_base_type::reset();
            encoder_errors = false;
            pending_data_to_encode = Idle_char;
            decoded_data_accumulator = Idle_char;
        }

        // reads from incoming base64 stream and returns decoded characters in "data"
        // returns TRUE if new data is received or if the stream is in EOF state
        // EOF is signaled as ANSI C symbol EOF (normally equal to -1)
        // incoming (encoded) stream is received via read_callback function, see constructor
        // "data present" and EOF interpretations shall be the same as for this function
        //
        bool read_decode(int& data)
        {
            if (decoded_data_accumulator == EOF)
            {
                data = EOF;
                return true;
            }

            decoded_data_accumulator = Idle_char;

            int c;
            if (!read_proc(c)) return false;

            c = (c < 0 ? EOL_code : ::sklib::opaque::b64_dictionary_inverse.data[c & OCTET_MASK]);

            if (c == EOL_code)  // EOF
            {
                write_flush();
                data = EOF;
                decoded_data_accumulator = EOF;
                return true;
            }
            if (c == Space_code)
            {
                return false;
            }
            if (c == Bad_code)
            {
                c = 0;
                encoder_errors = true;
            }

            write(::sklib::supplement::bits_fixed_pack_type<encoding_bit_length, uint8_t>((uint8_t)c));

            if (decoded_data_accumulator >= 0)
            {
                data = decoded_data_accumulator;
                return true;
            }

            return false;
        }

        // sends one byte of data to the stream, or EOF to signal end of transmission
        // encoded data is delivired via write_callback function, see constructor
        // more than one character can be sent out per one byte of data
        // P.S. end of transmission stgnal in base64 encoding is special in-stream character
        // new transmissio can be started after that
        //
        void write_encode(int data)
        {
            pending_data_to_encode = data;

            if (data < 0)
            {
                if (can_read(exchg))
                {
                    read(exchg);
                    write_proc(dictionary[exchg.data & dictionary_address_mask]);
                }

                read_rewind();
                write_proc(EOL_char);
            }
            else
            {
                read(exchg);
                write_proc(dictionary[exchg.data & dictionary_address_mask]);

                if (can_read_without_input_stream(exchg))
                {
                    read(exchg);
                    write_proc(dictionary[exchg.data & dictionary_address_mask]);
                }
            }
        }

        // reads from incoming raw stream and returns encoded symbols in "data"
        // returns TRUE if new data is received or if the stream is in EOF state
        // EOF is signaled as ANSI C symbol EOF (normally equal to -1)
        // incoming (raw) stream is received via read_callback function, see constructor
        // "data present" and EOF interpretations shall be the same as for this function
        //
        bool read_encode(int& data)
        {
            data = EOL_char;

            if (pending_data_to_encode == EOF) return true;

            if (can_read_without_input_stream(exchg))
            {
                read(exchg);
                data = dictionary[exchg.data & dictionary_address_mask];
                return true;
            }

            if (!read_proc(pending_data_to_encode)) return false;

            bool eof_seen = (pending_data_to_encode < 0);

            if (!eof_seen || read_has_residual_data())
            {
                read(exchg);
                data = dictionary[exchg.data & dictionary_address_mask];
            }

            if (eof_seen)
            {
                read_rewind();
                pending_data_to_encode = EOF;
            }

            return true;
        }

        // sends one encoded base64 symbol to the stream for decoding, or "=" to signal EOF
        // (also, just EOF can be sent in lieu of base64 terminator char)
        // decoded data is delivired via write_callback function, see constructor
        // less than one character can be sent out per one input symbol
        // P.S. new transmissio can be started after previous EOF condition
        //
        void write_decode(int data)
        {
            decoded_data_accumulator = Idle_char;

            if (data < 0 || data == EOL_char)
            {
                write_flush();
                write_proc(EOF);
            }
            else
            {
                uint8_t uc = ::sklib::opaque::b64_dictionary_inverse.data[(uint8_t)data];
                if (uc < dictionary_size) write(::sklib::supplement::bits_fixed_pack_type<encoding_bit_length, uint8_t>(uc));
                if (uc == Bad_code) encoder_errors = true;
                if (decoded_data_accumulator >= 0) write_proc(decoded_data_accumulator);
            }
        }

        static constexpr const uint8_t* get_inverse_table()
        {
            return ::sklib::opaque::b64_dictionary_inverse.data;
        }
    };

};

#endif // SKLIB_INCLUDED_BITWISE_HPP
