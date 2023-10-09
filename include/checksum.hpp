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

#include "bitwise.hpp"     // this also loads <type_traits>

namespace sklib
{
    namespace opaque
    {
        template<class T> static constexpr T crc_make_polynomial(bool mode_msb, unsigned length, T polynomial)
        { return (mode_msb ? polynomial : ::sklib::supplement::bits_flip_bruteforce<T>(polynomial, length)); }

        // see: https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
        template<class T>
        static constexpr void crc_generate_table(T(&U)[OCTET_ADDRESS_SPAN], unsigned length, T polynomial, bool msb = false)
        {
            const T high_bit = sklib::bits_data_high_1<T>(length);
            const T mask = sklib::bits_data_mask<T>(length);

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
                T vcrc = T(1);
                T poly_lsb = crc_make_polynomial<T>(msb, length, polynomial);

                for (int i = sklib::bits_high_1_v<uint8_t>; i; i >>= 1)
                {
                    auto have_low = vcrc & T(1);
                    vcrc >>= 1;
                    if (have_low) vcrc ^= poly_lsb;

                    for (int j=0, i2=(i<<1); j<OCTET_MASK; j+=i2) U[i+j] = vcrc ^ U[j];
                }
            }
        }

        template<class T>
        static constexpr auto crc_create_table(unsigned L, T P, bool msb)
        {
            sklib::supplement::encapsulated_array_octet_index_type<T> R = { 0 };
            crc_generate_table<T>(R.data, L, P, msb);
            return R;
        }
    };

    namespace supplement
    {
        template<class T>
        class crc_base_type     //sk !! move it to Internal
        {
        private:
            const sklib::supplement::encapsulated_array_octet_index_type<T>& Table;

            template<class T1> static constexpr bool tiny_container_v = (sklib::bits_width_v<T1> <= OCTET_BITS);

        protected:              // initialization order depends on the sequence in which const variables are declared
            const T mask;       // see: https://en.cppreference.com/w/cpp/language/constructor
            const T high_bit;
            const bool mode_add_bare;
            const int msb_shift;
            const T start_crc;

            T vcrc = 0;

        public:
            const bool MSB;
            const unsigned Polynomial_Degree;
            const T Polynomial;

            constexpr crc_base_type(const sklib::supplement::encapsulated_array_octet_index_type<T>& table_in, bool mode_MSB, unsigned Length, T Normal_Polynomial, T Start_Value)
                : Table(table_in)
                , MSB(mode_MSB)
                , mask(sklib::bits_data_mask<T>(Length))
                , high_bit(sklib::bits_data_high_1<T>(Length))
                , Polynomial_Degree(Length)
                , Polynomial(::sklib::opaque::crc_make_polynomial<T>(mode_MSB, Length, (Normal_Polynomial & mask)))   // mask is declared before Polynomial and others
                , msb_shift(Length > OCTET_BITS ? Length - OCTET_BITS : OCTET_BITS - Length)
                , start_crc(Start_Value & mask)
                , vcrc(start_crc)
                , mode_add_bare((mode_MSB && Length == OCTET_BITS) || (!mode_MSB && Length <= OCTET_BITS))
            {}

            constexpr void reset()               { vcrc = start_crc; }
            constexpr const T* get_table() const { return Table.data; }
            constexpr T get() const              { return (vcrc ^ start_crc) & mask; }
            constexpr operator T() const         { return get(); }

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

            // fast but implementation-dependent Update for fundamental types and *packed* POD's
            // deprecated anywhere where portability is critical
            template<class D>
            constexpr T update_packed(const ::sklib::supplement::do_not_deduce<D>& data)
            {
                static_assert(sizeof(uint8_t) == 1, "This cannot happen, size of byte is 1 by definition");
                return update(reinterpret_cast<const uint8_t *>(&data), sizeof(D));
            }

            // hardware-agnostic CRC update for an integer, t
            SKLIB_TEMPLATE_IF_INT(D)
            constexpr T update_integer_lsb(D val, unsigned length = sizeof(D))
            {
                typedef typename std::make_unsigned_t<D> uD;
                auto uval = static_cast<uD>(val);
                for (unsigned i=0; i<length; i++)
                {
                    uint8_t data = (uint8_t)(uval & OCTET_MASK);
                    uval >>= OCTET_BITS;
                    update(&data, 1);
                }
                return get();
            }

            SKLIB_TEMPLATE_IF_INT(D)
            constexpr T update_integer_msb(D val)
            {
                typedef typename std::make_unsigned_t<D> uD;
                auto uval = static_cast<uD>(D);
                for (int i=OCTET_MASK*(int)(sizeof(D)-1); i>=0; i-=OCTET_MASK)
                {
                    uint8_t data = (uint8_t)(uval >> i);
                    update(&data, 1);
                }
                return get();
            }

        protected:
            constexpr void add_bare(uint8_t ch)     // good for 1) MSB 8 bits, and 2) LSB 8 bits or less  // 8 = OCTET_BITS
            {
                vcrc = (Table.data[(vcrc ^ ch) & OCTET_MASK]);
            }

            // "normal" CRC formulas, for any bit count (and size of data)

            template<class T1, std::enable_if_t<!tiny_container_v<T1>, bool> = true>
            constexpr void add_msb_long(uint8_t ch)
            {
                vcrc = (T1(vcrc << OCTET_BITS) ^ Table.data[((vcrc >> msb_shift) ^ ch) & OCTET_MASK]);   // MSB longer than 8 bits
            }

            template<class T1, std::enable_if_t<!tiny_container_v<T1>, bool> = true>
            constexpr void add_msb_short(uint8_t ch)
            {
                vcrc = (T1(vcrc << OCTET_BITS) ^ Table.data[((vcrc << msb_shift) ^ ch) & OCTET_MASK]);   // MSB shorter than 8 bits
            }

            template<class T1, std::enable_if_t<!tiny_container_v<T1>, bool> = true>
            constexpr void add_lsb_long(uint8_t ch)
            {
                vcrc = (T1(vcrc >> OCTET_BITS) ^ Table.data[(vcrc ^ ch) & OCTET_MASK]);   // LSB longer than 8 bits
            }

            // versions for small data type, eliminating portions of normal formula

            template<class T1, std::enable_if_t<tiny_container_v<T1>, bool> = true>
            constexpr void add_msb_long(uint8_t ch)
            {
                vcrc = Table.data[((vcrc >> msb_shift) ^ ch) & OCTET_MASK];   // MSB, formally longer than 8 bits (doesn't make sense)
            }

            template<class T1, std::enable_if_t<tiny_container_v<T1>, bool> = true>
            constexpr void add_msb_short(uint8_t ch)
            {
                vcrc = Table.data[((vcrc << msb_shift) ^ ch) & OCTET_MASK];   // MSB shorter than 8 bits
            }

            template<class T1, std::enable_if_t<tiny_container_v<T1>, bool> = true> constexpr void add_lsb_long(uint8_t ch)
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
                    uint8_t m = sklib::bits_high_1_v<uint8_t>;
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
        sklib::supplement::encapsulated_array_octet_index_type<T> Table;

    public:
        typedef T type;

        constexpr crc_type(unsigned Length, T Normal_Polynomial, bool mode_MSB = false, T Start_Value = ~T(0))
            : ::sklib::supplement::crc_base_type<T>(Table, mode_MSB, Length, Normal_Polynomial, Start_Value)
            , Table(::sklib::opaque::crc_create_table<T>(Length, Normal_Polynomial, mode_MSB))
        {}
    };

    template<class T, unsigned Length, T Normal_Polynomial, bool mode_MSB = false, T Start_Value = sklib::bits_data_mask_v<T, Length>>
    class crc_fixed_type : public ::sklib::supplement::crc_base_type<T>
    {
        static_assert(SKLIB_TYPES_IS_INTEGER(T), "CRC Polynomial representation must be integer of approptiate size");
        static_assert(Length >= 1, "CRC Polynomial must be at least 1 bit long");
        static_assert(sklib::bits_width_less_sign_v<T> >= Length, "Data type for CRC must be large enough to hold the polynomial (sign bit must not be used)");
        static_assert(Normal_Polynomial > 0 && Normal_Polynomial % 2, "CRC Polynomial representation shall be odd positive integer");
        static_assert(Normal_Polynomial <= sklib::bits_data_mask_v<T, Length>, "CRC Polynomial representation must be within the specified length");

    protected:
        static constexpr sklib::supplement::encapsulated_array_octet_index_type<T> Table =
                             sklib::opaque::crc_create_table<T>(Length, Normal_Polynomial, mode_MSB);   // this data block becomes statically linked

    public:
        typedef T type;

        static constexpr bool MSB = mode_MSB;
        static constexpr unsigned Polynomial_Degree = Length;
        static constexpr T Polynomial =
            ::sklib::opaque::crc_make_polynomial<T>(mode_MSB, Length, (Normal_Polynomial & sklib::bits_data_mask<T>(Length)));

        static constexpr const T* get_table() { return Table.data; }

        constexpr crc_fixed_type() : ::sklib::supplement::crc_base_type<T>(Table, mode_MSB, Length, Normal_Polynomial, Start_Value) {}
    };

    // standard CRC types
    // see: https://en.wikipedia.org/wiki/Cyclic_redundancy_check

    // 8-bit: ITU-T I.432.1
    using crc_8_ccitt       = crc_fixed_type<uint8_t, 8, 0x07>;
    using crc_8_ccitt_msb   = crc_fixed_type<uint8_t, 8, 0x07, true>;

    // 16-bit ("Standard 16"): ITU-T X.25; also V.41; XModem; etc
    using crc_16_ccitt      = crc_fixed_type<uint16_t, 16, 0x1021>;
    using crc_16_ccitt_msb  = crc_fixed_type<uint16_t, 16, 0x1021, true>;

    // 16-bit: ANSI; also IBM, Modbus, USB
    using crc_16_ansi       = crc_fixed_type<uint16_t, 16, 0x8005u>;            // X3.28; USB
    using crc_16_ansi_msb   = crc_fixed_type<uint16_t, 16, 0x8005u, true>;

    // 32-bit ("Standard 32"): ISO; ANSI; *Zip; PNG; ZModem; etc
    using crc_32_iso        = crc_fixed_type<uint32_t, 32, 0x04C11DB7ul>;
    using crc_32_iso_msb    = crc_fixed_type<uint32_t, 32, 0x04C11DB7ul, true>;

    // 32-bit: Castagnoli, improved version of CRC-32
    using crc_32C_lsb       = crc_fixed_type<uint32_t, 32, 0x1EDC6F41ul>;
    using crc_32C_msb       = crc_fixed_type<uint32_t, 32, 0x1EDC6F41ul, true>;

    // 64-bit
    using crc_64_ecma       = crc_fixed_type<uint64_t, 64, 0x42F0E1EBA9EA3693ull>;   // ECMA-182
    using crc_64_iso        = crc_fixed_type<uint64_t, 64, 0x000000000000001Bull>;   // ISO 3309

    // other popular CRC's

    using crc_1_parity      = crc_fixed_type<uint8_t, 1, 0x1>;      // parity

    using crc_4_itu         = crc_fixed_type<uint8_t, 4, 0x03>;     // ITU-T G.704
    using crc_5_itu         = crc_fixed_type<uint8_t, 5, 0x15>;
    using crc_6_itu         = crc_fixed_type<uint8_t, 6, 0x03>;
    using crc_7_sd_lsb      = crc_fixed_type<uint8_t, 7, 0x09>;     // SD cards, also ITU-T G.704
    using crc_7_sd_msb      = crc_fixed_type<uint8_t, 7, 0x09, true>;

    using crc_3_gsm         = crc_fixed_type<uint8_t, 3, 0x03>;     // Cellular related standards
    using crc_6A_cdma       = crc_fixed_type<uint8_t, 6, 0x27>;
    using crc_6B_cdma       = crc_fixed_type<uint8_t, 6, 0x07>;
    using crc_6_gsm         = crc_fixed_type<uint8_t, 6, 0x2F>;

    using crc_24_wcdma      = crc_fixed_type<uint32_t, 24, 0x00800063ul>;
    using crc_30_cdma       = crc_fixed_type<uint32_t, 30, 0x2030B9C7ul>;
    using crc_40_gsm        = crc_fixed_type<uint64_t, 40, 0x0004820009ull>;

    using crc_6_darc        = crc_fixed_type<uint8_t, 6, 0x19>;     // FM broadcast data radio channel
    using crc_14_darc       = crc_fixed_type<uint16_t, 14, 0x0805>;

    using crc_5_rfid        = crc_fixed_type<uint8_t, 5, 0x09>;     // Gen 2 RFID
    using crc_5_usb         = crc_fixed_type<uint8_t, 5, 0x05>;     // USB token, LSB

    using crc_7_mvb         = crc_fixed_type<uint8_t, 7, 0x65>;     // IEC 60870-5, train comm network

    using crc_8_dallas_lsb  = crc_fixed_type<uint8_t, 8, 0x31>;     // 1-Wire Bus
    using crc_8_dallas_msb  = crc_fixed_type<uint8_t, 8, 0x31, true>;

    using crc_16A_osafety   = crc_fixed_type<uint16_t, 16, 0x5935u>;        // Open Safety
    using crc_16B_osafety   = crc_fixed_type<uint16_t, 16, 0x755Bu>;
    using crc_16_profi      = crc_fixed_type<uint16_t, 16, 0x1DCFu>;        // ProfiBus
    using crc_17_can        = crc_fixed_type<uint32_t, 17, 0x0001685Bul>;   // CAN bus
    using crc_21_can        = crc_fixed_type<uint32_t, 21, 0x00102899ul>;
    using crc_24_flex       = crc_fixed_type<uint32_t, 24, 0x005D6DCBul>;   // FlexRay

    using crc_24_triplet    = crc_fixed_type<uint32_t, 24, 0x00864CFBul>;   // Radix64 / Base64 / 3 bytes; also RTCM 104v3

    using crc_32Q           = crc_fixed_type<uint32_t, 32, 0x814141ABul>;   // AXIM, aviation

    //sk TODO: review CRC-8-DVB through CRC-15-DNP


};

#endif // SKLIB_INCLUDED_CHECKSUM_HPP
