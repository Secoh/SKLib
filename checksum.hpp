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

#ifndef SKLIB_INCLUDED_CHECKSUM_HPP
#define SKLIB_INCLUDED_CHECKSUM_HPP


#include "./bitwise.hpp"

namespace sklib
{
#include "./static/checksum-tables.hpp"

    template<class T, size_t N, T V>
    class crc_poly
    {
        static_assert(std::is_integral_v<T>&& std::is_unsigned_v<T>, "CRC Polynom representation must be unsigned integer of approptiate size");
        static_assert(N >= 1, "CRC Polynom must be at least 1 bit long");
        static_assert(sizeof(T)* sklib::bitwise::BYTE_BITS >= N, "CRC Polynom representation must be long ");

    };

    void crc_generate_table_msb(uint8_t(&U)[::sklib::OCTET_ADDRESS_SPAN])
    {
        for (size_t k = 0; k < ::sklib::OCTET_ADDRESS_SPAN; k++) U[k] = ::sklib::supplement::bits_flip_bruteforce<uint8_t>(uint8_t(k));
    }

};


/* CRC16/CRC32 Transformation Tables can be either linked by linker,
   or declared in the header at compile-time as constexpr arrays.

   Linking guarantees that the tables are not present in the executable
   file more than once. (Constexpr declaration relies on compiler's
   ability to reduce bloating which is NOT guaranteed.)

   The benefit of using standalone header(s) is reducing the number of
   projects/libraries that you, the developer, need to remember.

   The default for SkLib/checksum is using standalone header.
   To switch to linking mode, include SkLib/checksum/checksum.vcxproj
   into your compilation, AND define the following before including
   the SkLib/checksum.hpp header:

   #define SKLIB_CHECKSUM_LINK_CRC_TABLES
*/

/*
namespace sklib_internal
{
    constexpr size_t checksum_crc_table_size = 256;
};

#ifdef SKLIB_CHECKSUM_LINK_CRC_TABLES
namespace sklib_internal
{
    extern const uint16_t checksum_crc16_table[checksum_crc_table_size];
};
#else
#define SKLIB_CHECKSUM_LINK_CRC_TABLES do not  / * reuse to trigger sharing to the compilation unit * /
#include "checksum/crc_tables.hpp"
#undef SKLIB_CHECKSUM_LINK_CRC_TABLES
#endif
*/
namespace sklib
{
    template<class T>   // shall be unsigned integer
    class crc
    {
    private:
        T check;
        T mask;
        T data;
        T poly;
        bool shift_left;
        bool msb_input;

    public:
        crc(T polynomial, int size, bool msb = true, bool msb_source = msb) : check(0), mask(0), shift_left(msb), msb_input(msb_source)
        {
            if (size > 0)
            {
                auto hc = T(T(1) << (size-1));
                check = (msb ? hc : T(1));
                mask = (hc | (hc-1));
            }

            data = mask;
            poly = (polynomial & mask);
        }

        void reset(T init)
        {
            data = init;
        }

        T add_bit_bruteforce(bool bit)
        {
            if (data & check) bit = !bit;
            data = (shift_left ? ((data<<1) & mask) : (data>>1));
            if (bit) data ^= poly;
            return data;
        }

        T get_data() const
        {
            return data;
        }
    };

    class crc16_ccitt
    {
    public:
        crc16_ccitt(uint16_t initial_value = start_value_default) { reset(initial_value); }
        void reset(uint16_t initial_value = start_value_default)  { start_value = accumulator = initial_value; }

        uint16_t get() const                      { return (accumulator ^ start_value); }
        operator uint16_t() const                 { return get(); }

        void add(const uint8_t* buf, size_t len)  { while (len--) add(*buf++); }
        void add(uint8_t ch)                      { accumulator = (uint16_t(accumulator >> 8) ^
                                     ::sklib::internal::tables::crc16_transformation_table[(accumulator ^ ch) & 0xFF]); }

    private:
        uint16_t accumulator, start_value;

        static constexpr uint16_t start_value_default = 0xFFFFu;
    };


};



#endif // SKLIB_INCLUDED_CHECKSUM_HPP
