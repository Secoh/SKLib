// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides functions related to prime numbers compressed storage
// This is internal SKLib file and must NOT be included directly.

namespace primes_compressor
{
    // xxxx:      0000 through 1110, i.e. from delta index 1 to delta 15
    // 1111 xxxx: deltas 16-30
    // 1111 1111 xxxx,: 31-45
    // 1111 1111 1111 + 7 bit: 46 and up, known maximum is 111 (see primes-32 list)

    // bit lengths to encode data in different frequency tiers
    inline constexpr int zone0 = 4; // the most frequent selections
    inline constexpr int zoneA = 4;
    inline constexpr int zoneB = 4;
    inline constexpr int zoneC = 7; // the least frequent selections...
    // ...AND there is no data that need longer encodings

    inline constexpr int cutA = zone0;
    inline constexpr int cutB = cutA + zoneA;
    inline constexpr int cutC = cutB + zoneB;
    inline constexpr int cut_cap = cutC + zoneC;

    inline constexpr auto onesA = sklib::bits_pack<cutA>(sklib::bits_data_mask<int>(cutA));
    inline constexpr auto onesB = sklib::bits_pack<cutB>(sklib::bits_data_mask<int>(cutB));
    inline constexpr auto onesC = sklib::bits_pack<cutC>(sklib::bits_data_mask<int>(cutC));
    inline constexpr auto ones_cap = sklib::bits_pack<cut_cap>(sklib::bits_data_mask<int>(cut_cap));

    inline constexpr auto onesB_overA = sklib::bits_pack<zoneA>(sklib::bits_data_mask<int>(zoneA));
    inline constexpr auto onesC_overB = sklib::bits_pack<zoneB>(sklib::bits_data_mask<int>(zoneB));
    inline constexpr auto ones_overC = sklib::bits_pack<zoneC>(sklib::bits_data_mask<int>(zoneC));

    // codes in zone0 starts with 0; codes in zones A, B, etc, start with argument = tier A, B, etc
    inline constexpr int tierA = sklib::bits_data_mask<int>(zone0);
    inline constexpr int tierB = sklib::bits_data_mask<int>(zoneA) + tierA;
    inline constexpr int tierC = sklib::bits_data_mask<int>(zoneB) + tierB;
    inline constexpr int tier_cap = sklib::bits_data_mask<int>(zoneC) + tierC;
}

enum class primes_decoder_status_type
{
    OK = 0,
    broken_archive, missing_CRC, CRC_mismatch, internal_bitstream_error
};

// clears the output array, reads file, and fills the array
// every 1 second and immediately before successful exit calls heartbeat
// if not nullptr, with payload and the number of records
// does NOT rewind input file
// returned CRC reflects the actual content of PrimesArrayOutput
inline primes_decoder_status_type primes_decode(sklib::bits_file_type& fPackedInput,
                                                std::vector<uint32_t>& PrimesArrayOutput,
                                                uint32_t& CurrentCRC,
                                                void (*heartbeat)(void*, uint32_t) = nullptr,
                                                void* payload = nullptr)
{
    using namespace sklib::primes_compressor;

    sklib::crc_32_iso Checksum;
    sklib::timer_stopwatch_type strobe(1000);

    PrimesArrayOutput.clear();
    PrimesArrayOutput.push_back(2);
    PrimesArrayOutput.push_back(3);
    Checksum.update_integer_lsb<uint32_t>(2);
    Checksum.update_integer_lsb<uint32_t>(3);

    // starting point
    uint32_t nrecords = 2;
    int Idx = 0;

    auto RA = onesA;        // registers to read bits...
    auto RB = onesB_overA;  // yes, A, B and C are identical, but this is for generalization
    auto RC = onesC_overB;
    auto RD = ones_overC;

    auto is_invalid_bitpack = [](int pack, int mask) { return (pack < 0 || pack > mask); };

    while (true)
    {
        auto P = sklib::prime_candidate<uint32_t>(Idx);
        PrimesArrayOutput.push_back(P);
        Checksum.update_integer_lsb<uint32_t>(P);

        nrecords++;
        if (heartbeat && strobe(true)) heartbeat(payload, nrecords);

        if (!fPackedInput.can_read(1)) return primes_decoder_status_type::broken_archive;

        fPackedInput.read(RA);
        if (is_invalid_bitpack(RA.data, onesA.data)) return primes_decoder_status_type::internal_bitstream_error;
        if (RA.data < onesA.data)
        {
            Idx += RA.data + 1;
            continue;
        }

        fPackedInput.read(RB);
        if (is_invalid_bitpack(RB.data, onesB_overA.data)) return primes_decoder_status_type::internal_bitstream_error;
        if (RB.data < onesB_overA.data)
        {
            Idx += RB.data + tierA + 1;
            continue;
        }

        fPackedInput.read(RC);
        if (is_invalid_bitpack(RC.data, onesC_overB.data)) return primes_decoder_status_type::internal_bitstream_error;
        if (RC.data < onesC_overB.data)
        {
            Idx += RC.data + tierB + 1;
            continue;
        }

        fPackedInput.read(RD);
        if (is_invalid_bitpack(RD.data, ones_overC.data)) return primes_decoder_status_type::internal_bitstream_error;

        if (RD.data == ones_overC.data) break;

        Idx += RD.data + tierC + 1;
    }

    static constexpr unsigned crc_width = sklib::bits_width_v<uint32_t>;
    auto R32 = sklib::bits_pack<crc_width, uint32_t>(0);
    if (!fPackedInput.can_read(R32)) return primes_decoder_status_type::missing_CRC;
    fPackedInput.read(R32);

    CurrentCRC = Checksum.get();
    if (CurrentCRC != R32.data) return primes_decoder_status_type::CRC_mismatch;

    if (heartbeat) heartbeat(payload, nrecords);

    return primes_decoder_status_type::OK;
}

inline constexpr bool is_primes_decoder_status_good(primes_decoder_status_type code)
{ return (code == primes_decoder_status_type::OK); }

inline constexpr std::string primes_decoder_status_msg(primes_decoder_status_type code)
{
    switch (code)
    {
    case primes_decoder_status_type::OK:
        return "Success";
    case primes_decoder_status_type::broken_archive:
        return "File error - unexpected EOF (broken archive)";
    case primes_decoder_status_type::missing_CRC:
        return "File error - unexpected EOF (missing CRC)";
    case primes_decoder_status_type::CRC_mismatch:
        return "Warning: input/control CRC mismatch";
    case primes_decoder_status_type::internal_bitstream_error:
        return "Bit stream error: received bits outside mask range (this cannot happen)";
    default:
        return "Undefined error";
    }
}

