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
    namespace internal
    {
        template<class T>
        static constexpr T crc_make_polynomial(bool mode_msb, unsigned length, T polynomial)
        {
            return (mode_msb ? polynomial : ::sklib::supplement::bits_flip_bruteforce<T>(polynomial, length));
        }

        // see: https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
        template<class T>
        static constexpr void crc_generate_table(T(&U)[OCTET_ADDRESS_SPAN], unsigned length, T polynomial, bool msb = false)
        {
            const T high_bit = ::sklib::supplement::bits_short_data_high_1<T>(length);
            const T mask = ::sklib::supplement::bits_short_data_mask<T>(length);

            polynomial &= mask;
            U[0] = 0;

            if (msb)
            {
                T vcrc = high_bit;

                for (int i = 1; i < OCTET_ADDRESS_SPAN; i <<= 1)
                {
                    auto have_high = vcrc & high_bit;
                    vcrc = (vcrc << 1) & mask;
                    if (have_high) vcrc ^= polynomial;

                    for (int j=0; j<i; j++) U[i+j] = vcrc ^ U[j];
                }
            }
            else // LSB
            {
                T vcrc = 1;
                T poly_lsb = crc_make_polynomial<T>(msb, length, polynomial);

                for (int i = ::sklib::supplement::bits_data_high_1<uint8_t>(); i; i >>= 1)
                {
                    auto have_low = vcrc & 1;
                    vcrc >>= 1;
                    if (have_low) vcrc ^= poly_lsb;

                    for (int j=0, i2=(i<<1); j<OCTET_MASK; j+=i2) U[i+j] = vcrc ^ U[j];
                }
            }
        }

        template<class T>
        static constexpr auto crc_create_table(unsigned L, T P, bool msb)
        {
            encapsulated_array_octet_index_type<T> R = { 0 };
            crc_generate_table<T>(R.data, L, P, msb);
            return R;
        }
    };

    namespace supplement
    {
        template<class T>
        class crc_base_type
        {
        private:
            const ::sklib::internal::encapsulated_array_octet_index_type<T>& Table;

            template<class T1>
            static constexpr bool tiny_container()
            {
                return (::sklib::supplement::bits_data_width<T1>() <= OCTET_BITS);
            }

        public:
            const bool MSB;
            const unsigned Polynomial_Degree;
            const T Polynomial;

        protected:
            const bool mode_add_bare;
            const int msb_shift;
            const T start_crc;
            const T mask;
            const T high_bit;

            T vcrc = 0;

        public:
            constexpr crc_base_type(const ::sklib::internal::encapsulated_array_octet_index_type<T>& table_in, bool mode_MSB, unsigned Length, T Normal_Polynomial, T Start_Value)
                : Table(table_in)
                , MSB(mode_MSB)
                , Polynomial_Degree(Length)
                , Polynomial(::sklib::internal::crc_make_polynomial<T>(MSB, Length, Normal_Polynomial))
                , msb_shift(Length > OCTET_BITS ? Length - OCTET_BITS : OCTET_BITS - Length)
                , start_crc(Start_Value)
                , mask(::sklib::supplement::bits_short_data_mask<T>(Length))
                , high_bit(::sklib::supplement::bits_short_data_high_1<T>(Length))
                , vcrc(Start_Value)
                , mode_add_bare((MSB&& Length == OCTET_BITS) || (!MSB && Length <= OCTET_BITS))
            {}

            constexpr void reset() { vcrc = start_crc; }

            constexpr const T* get_table() const { return Table.data; }

            constexpr T get() const { return (vcrc ^ start_crc) & mask; }

            constexpr operator T() const { return get(); }

            constexpr T update(const char* cstr)
            {
                size_t N = 0;
                while (cstr[N]) N++;

                add(cstr, N);
                return get();
            }

            constexpr T update_bruteforce(const char* cstr)
            {
                while (*cstr) add_octet_bruteforce((uint8_t)*cstr++);
                return get();
            }

            constexpr T update(const uint8_t* data, size_t count)
            {
                add(data, count);
                return get();
            }

            constexpr T update_bruteforce(const uint8_t* data, size_t count)
            {
                while (count--) add_octet_bruteforce(*data++);
                return get();
            }

        protected:
            constexpr void add_bare(uint8_t ch)     // good for 1) MSB 8 bits, and 2) LSB 8 bits or less  // 8 = OCTET_BITS
            {
                vcrc = (Table.data[(vcrc ^ ch) & OCTET_MASK]);
            }

            // "normal" CRC formulas, for any bit count (and size of data)

            template<class T1, std::enable_if_t<!tiny_container<T1>(), bool> = true>
            constexpr void add_msb_long(uint8_t ch)
            {
                vcrc = (T1(vcrc << OCTET_BITS) ^ Table.data[((vcrc >> msb_shift) ^ ch) & OCTET_MASK]);   // MSB longer than 8 bits
            }

            template<class T1, std::enable_if_t<!tiny_container<T1>(), bool> = true>
            constexpr void add_msb_short(uint8_t ch)
            {
                vcrc = (T1(vcrc << OCTET_BITS) ^ Table.data[((vcrc << msb_shift) ^ ch) & OCTET_MASK]);   // MSB shorter than 8 bits
            }

            template<class T1, std::enable_if_t<!tiny_container<T1>(), bool> = true>
            constexpr void add_lsb_long(uint8_t ch)
            {
                vcrc = (T1(vcrc >> OCTET_BITS) ^ Table.data[(vcrc ^ ch) & OCTET_MASK]);   // LSB longer than 8 bits
            }

            // versions for small data type, eliminating portions of normal formula

            template<class T1, std::enable_if_t<tiny_container<T1>(), bool> = true>
            constexpr void add_msb_long(uint8_t ch)
            {
                vcrc = Table.data[((vcrc >> msb_shift) ^ ch) & OCTET_MASK];   // MSB, formally longer than 8 bits (doesn't make sense)
            }

            template<class T1, std::enable_if_t<tiny_container<T1>(), bool> = true>
            constexpr void add_msb_short(uint8_t ch)
            {
                vcrc = Table.data[((vcrc << msb_shift) ^ ch) & OCTET_MASK];   // MSB shorter than 8 bits
            }

            template<class T1, std::enable_if_t<tiny_container<T1>(), bool> = true> constexpr void add_lsb_long(uint8_t ch)
            {
                add_bare(ch);   // LSB, formally longer than 8 bits - same as Bare
            }

            template<class T8>
            constexpr void add(const T8* buf, size_t len)
            {
                if (mode_add_bare)
                {
                    while (len--) add_bare((uint8_t)*buf++);
                }
                else if (!MSB) // LSB and Length > OCTET_BITS
                {
                    while (len--) add_lsb_long<T>((uint8_t)*buf++);
                }
                else if (Polynomial_Degree > OCTET_BITS) // MSB and polynomial is longer than byte
                {
                    while (len--) add_msb_long<T>((uint8_t)*buf++);
                }
                else  // MSB and polynomial is shorter than byte
                {
                    while (len--) add_msb_short<T>((uint8_t)*buf++);
                }
            }

            constexpr void add_bit_msb_bruteforce(bool bit)
            {
                if (vcrc & high_bit) bit = !bit;
                vcrc = ((vcrc << 1) & mask);
                if (bit) vcrc ^= Polynomial;
            }

            constexpr void add_bit_lsb_bruteforce(bool bit)
            {
                if (vcrc & T(1)) bit = !bit;
                vcrc >>= 1;
                if (bit) vcrc ^= Polynomial;
            }

            constexpr void add_octet_bruteforce(uint8_t ch)
            {
                if (MSB)
                {
                    uint8_t m = ::sklib::supplement::bits_data_high_1<uint8_t>();
                    for (int i = 0; i < OCTET_BITS; i++, m >>= 1) add_bit_msb_bruteforce(ch & m);
                }
                else // LSB
                {
                    uint8_t m = 1;
                    for (int i = 0; i < OCTET_BITS; i++, m <<= 1) add_bit_lsb_bruteforce(ch & m);
                }
            }
        };
    };


    template<class T>
    class crc_type : public ::sklib::supplement::crc_base_type<T>
    {
        static_assert(SKLIB_TYPES_IS_INTEGER(T), "CRC Polynomial representation must be integer of approptiate size");

    protected:
        ::sklib::internal::encapsulated_array_octet_index_type<T> Table;

    public:
        constexpr crc_type(unsigned Length, T Normal_Polynomial, bool mode_MSB = false, T Start_Value = ::sklib::supplement::bits_short_data_mask<T>(Length))
            : ::sklib::supplement::crc_base_type<T>(Table, mode_MSB, Length, Normal_Polynomial, Start_Value)
            , Table(::sklib::internal::crc_create_table<T>(Length, Normal_Polynomial, mode_MSB))
        {}
    };

    template<class T, unsigned Length, T Normal_Polynomial, bool mode_MSB = false, T Start_Value = ::sklib::supplement::bits_short_data_mask<T>(Length)>
    class crc_fixed_type : public ::sklib::supplement::crc_base_type<T>
    {
        static_assert(SKLIB_TYPES_IS_INTEGER(T), "CRC Polynomial representation must be integer of approptiate size");
        static_assert(Length >= 1, "CRC Polynomial must be at least 1 bit long");
        static_assert(::sklib::supplement::bits_data_width_less_sign<T>() >= Length, "Data type for CRC must be large enough to hold the polynomial (sign bit must not be used)");
        static_assert(Normal_Polynomial > 0 && Normal_Polynomial % 2, "CRC Polynomial representation shall be odd positive integer");
        static_assert(Normal_Polynomial <= ::sklib::supplement::bits_short_data_mask<T, Length>(), "CRC Polynomial representation must be within the specified length");

    protected:
        static constexpr ::sklib::internal::encapsulated_array_octet_index_type<T> Table =
                             ::sklib::internal::crc_create_table<T>(Length, Normal_Polynomial, mode_MSB);   // this data block becomes statically linked

    public:
        static constexpr bool MSB = mode_MSB;
        static constexpr unsigned Polynomial_Degree = Length;
        static constexpr T Polynomial = ::sklib::internal::crc_make_polynomial<T>(mode_MSB, Length, Normal_Polynomial);

        static constexpr const T* get_table() { return Table.data; }

        constexpr crc_fixed_type() : ::sklib::supplement::crc_base_type<T>(Table, mode_MSB, Length, Normal_Polynomial, Start_Value) {}
    };

    // standard CRC types
    // see: https://en.wikipedia.org/wiki/Cyclic_redundancy_check

    using crc_1_parity      = crc_fixed_type<uint8_t, 1, 0x1>;           // parity
    using crc_3_gsm         = crc_fixed_type<uint8_t, 3, 0x3>;           // cellular
    using crc_4_itu         = crc_fixed_type<uint8_t, 4, 0x3>;           // G.704

    using crc_8_dallas_lsb  = crc_fixed_type<uint8_t, 8, 0x31>;          // 1-Wire Bus
    using crc_8_dallas_msb  = crc_fixed_type<uint8_t, 8, 0x31, true>;

    using crc_21_can_lsb    = crc_fixed_type<uint32_t, 21, 0x102899>;


    // ITU-T I.432.1
    using crc_8_ccitt       = crc_fixed_type<uint8_t, 8, 0x07>;
    using crc_8_ccitt_msb   = crc_fixed_type<uint8_t, 8, 0x07, true>;

    // X.25; V.41; XModem; etc - "Standard 16"
    using crc_16_ccitt      = crc_fixed_type<uint16_t, 16, 0x1021>;
    using crc_16_ccitt_msb  = crc_fixed_type<uint16_t, 16, 0x1021, true>;

    // ISO; ANSI; *Zip; PNG; ZModem; etc - "Standard 32"
    using crc_32_iso        = crc_fixed_type<uint32_t, 32, 0x04C11DB7ul>;
    using crc_32_iso_msb    = crc_fixed_type<uint32_t, 32, 0x04C11DB7ul, true>;

    using crc_32C_lsb       = crc_fixed_type<uint32_t, 32, 0x1EDC6F41ul>;         // Castagnoli, improved version of CRC-32
    using crc_32C_msb       = crc_fixed_type<uint32_t, 32, 0x1EDC6F41ul, true>;

};



//sk lets work more on description below

/* CRC16/CRC32 Transformation Tables can be either linked by linker,
   or declared in the header at compile-time as constexpr arrays.

   Linking guarantees that the tables are not present in the executable
   file more than once. (Constexpr declaration relies on compiler's
   ability to reduce bloating which is NOT guaranteed.)

   The benefit of using standalone header(s) is reducing the number of
   projects/libraries that you, the developer, need to remember.

   The default for SkLib/checksum is using standalone header.
*/


#endif // SKLIB_INCLUDED_CHECKSUM_HPP
