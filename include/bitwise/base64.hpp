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

// Base64 implementation on top of bits_stream_base_type class
// This is internal SKLib file and must NOT be included directly.

namespace priv
{
    class base64_property_type
    {
        friend constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> generate_dictionary_inverse_table();

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

    constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> generate_dictionary_inverse_table()
    {
        sklib::aux::encapsulated_array_octet_index_type<uint8_t> R = { 0 };
        for (int k=0; k<=' '; k++) R.data[k] = base64_property_type::Space_code;
        for (int k=' '+1; k<OCTET_ADDRESS_SPAN; k++) R.data[k] = base64_property_type::Bad_code;
        for (size_t k=0; k< base64_property_type::dictionary_size; k++) R.data[base64_property_type::dictionary[k]] = (uint8_t)k;
        R.data[base64_property_type::EOL_char] = base64_property_type::EOL_code;
        return R;
    }

    inline constexpr sklib::aux::encapsulated_array_octet_index_type<uint8_t> b64_dictionary_inverse = generate_dictionary_inverse_table();
};

class base64_type : protected sklib::bits_stream_base_type, public sklib::priv::base64_property_type
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
    sklib::aux::callback_type<base64_type, bool, int&> read_proc;
    sklib::aux::callback_type<base64_type, void, int> write_proc;

    //bool (* const read_proc)(base64_type*, int&);
    //void (* const write_proc)(base64_type*, int);

    sklib::aux::bits_fixed_pack_type<encoding_bit_length, uint8_t> exchg{ 0 };

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

        c = (c < 0 ? EOL_code : sklib::priv::b64_dictionary_inverse.data[c & OCTET_MASK]);

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

        write(sklib::aux::bits_fixed_pack_type<encoding_bit_length, uint8_t>((uint8_t)c));

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
            uint8_t uc = sklib::priv::b64_dictionary_inverse.data[(uint8_t)data];
            if (uc < dictionary_size) write(sklib::aux::bits_fixed_pack_type<encoding_bit_length, uint8_t>(uc));
            if (uc == Bad_code) encoder_errors = true;
            if (decoded_data_accumulator >= 0) write_proc(decoded_data_accumulator);
        }
    }

    static constexpr const uint8_t* get_inverse_table()
    {
        return sklib::priv::b64_dictionary_inverse.data;
    }
};
