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

// Windows-specific audio interface
// To prevent namespace pollution, Windows.h header is NOT included into the main SKLib headers structure
// Requires linking to compilation unit: w32_audio_code

#ifndef SKLIB_INCLUDED_W32_AUDIO_HPP
#define SKLIB_INCLUDED_W32_AUDIO_HPP

#include <string>
#include <vector>

namespace sklib
{
    class wave_io
    {
    public:
        struct device_caps_type
        {
            std::wstring name;  // Device name
            uint32_t caps;      // copy of dwFormats
            uint32_t opts;      // optional functionality for output device, equals 0 for input
            bool stereo;        // true if 2 channels are supported
        };

        wave_io();
        ~wave_io();

        bool open_out();
        bool open_in();
        void close_out();
        void close_in();

        // we do not include Window.h header - lets borrow the constants by linking to compiled implementation code
        const static uint32_t WAVE_FORMAT_1M08;     // 11.025 kHz, mono, 8-bit
            WAVE_FORMAT_1M16	11.025 kHz, mono, 16 - bit
            WAVE_FORMAT_1S08	11.025 kHz, stereo, 8 - bit
            WAVE_FORMAT_1S16	11.025 kHz, stereo, 16 - bit
            WAVE_FORMAT_2M08	22.05 kHz, mono, 8 - bit
            WAVE_FORMAT_2M16	22.05 kHz, mono, 16 - bit
            WAVE_FORMAT_2S08	22.05 kHz, stereo, 8 - bit
            WAVE_FORMAT_2S16	22.05 kHz, stereo, 16 - bit
            WAVE_FORMAT_4M08	44.1 kHz, mono, 8 - bit
            WAVE_FORMAT_4M16	44.1 kHz, mono, 16 - bit
            WAVE_FORMAT_4S08	44.1 kHz, stereo, 8 - bit
            WAVE_FORMAT_4S16	44.1 kHz, stereo, 16 - bit
            WAVE_FORMAT_96M08	96 kHz, mono, 8 - bit
            WAVE_FORMAT_96M16	96 kHz, mono, 16 - bit
            WAVE_FORMAT_96S08	96 kHz, stereo, 8 - bit
            WAVE_FORMAT_96S16
    protected:
        std::vector<device_caps_type> hdw_output, hdw_input;
    };

    class wave_out_sys
    {
    public:



        wave_out_sys();
        wave_out_sys(int sample_rate, int bits, bool stereo);
        ~wave_out_sys();
    };
};

#endif // SKLIB_INCLUDED_W32_AUDIO_HPP

