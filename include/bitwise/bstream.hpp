// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides bits stream - continuous packed bits array with I/O in the manner of a file
// This is internal SKLib file and must NOT be included directly.

// ---------------------------------------
// Objects representing series of bits
// Pack sequence of such objects into sequence of bytes, in MSB mode
// Unpack byte stream into sequence of objects representing bit packs
// (bytes are considered MSB; leading bit in the stream corresponds to leading bit in the pack)

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
{
    return sklib::supplement::bits_fixed_pack_type<N, T>(v);
}

SKLIB_TEMPLATE_IF_INT(T)
constexpr auto bits_pack(T v, unsigned N = sklib::bits_width_v<T>)
{
    return sklib::supplement::bits_variable_pack_type<T>(v, N);
}

// --------------------------------------------------
// Bit Stream control class
// uses big-endian model

class bits_stream_base_type
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
        clear_bits_sender = sklib::OCTET_BITS;
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
                clear_bits_sender = sklib::OCTET_BITS;
            }
        }

        return *this;
    }

    SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
    bits_stream_base_type& operator<< (const TT& input)
    {
        return write(input);
    }

    void write_flush()
    {
        if (clear_bits_sender < sklib::OCTET_BITS && write_octet) write_octet(accumulator_sender << clear_bits_sender);
        clear_bits_sender = sklib::OCTET_BITS;
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
    {
        return can_read_without_input_stream(request.bit_count);
    }

    // true if subsequent read() can return valid data
    // even when input stream has ended
    bool read_has_residual_data() const
    {
        return can_read_without_input_stream(1);
    }

    // true if there is some data in the stream (beware: trailing bits may be truncated)
    bool can_read(unsigned bit_count)
    {
        if (!bit_count || available_bits_receiver) return true;
        if (!read_octet || !read_octet(accumulator_receiver)) return false;
        available_bits_receiver = sklib::OCTET_BITS;
        return true;
    }

    SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
    bool can_read(const TT& request)
    {
        return can_read(request.bit_count);
    }

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
                available_bits_receiver = sklib::OCTET_BITS;
            }

            auto load_size = std::min(data_size, available_bits_receiver);
            available_bits_receiver -= load_size;

            // this arrangement guarantees that bits higher than bit_count will be 0
            auto receiver_split = uint16_t(accumulator_receiver << load_size);
            request.data = (request.data << load_size) + (receiver_split >> sklib::OCTET_BITS);
            accumulator_receiver = uint8_t(receiver_split);

            data_size -= load_size;
        }

        return *this;
    }

    SKLIB_TEMPLATE_IF_DERIVED(TT, sklib::opaque::bits_variable_pack_anchor)
    bits_stream_base_type& operator>> (TT& request)
    {
        return read(request);
    }

protected:
    static constexpr uint8_t byte_low_mask[sklib::OCTET_BITS + 1] = { 0,
        sklib::bits_data_mask_v<uint8_t, 1>,
        sklib::bits_data_mask_v<uint8_t, 2>,
        sklib::bits_data_mask_v<uint8_t, 3>,
        sklib::bits_data_mask_v<uint8_t, 4>,
        sklib::bits_data_mask_v<uint8_t, 5>,
        sklib::bits_data_mask_v<uint8_t, 6>,
        sklib::bits_data_mask_v<uint8_t, 7>,
        sklib::bits_data_mask_v<uint8_t, 8> };

    uint8_t  accumulator_sender = 0;
    unsigned clear_bits_sender = sklib::OCTET_BITS;
    uint8_t  accumulator_receiver = 0;
    unsigned available_bits_receiver = 0;
};

