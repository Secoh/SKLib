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

#include <memory>
#include <string>
#include <vector>

namespace sklib
{
    struct wave_device_caps_type
    {
        std::wstring name;   // Device name
        uint32_t caps = 0;   // copy of dwFormats
        uint32_t opts = 0;   // optional functionality for output device, unused (=0) for input device
        bool stereo = false; // true if 2 channels are supported

        // caps (see above) is bit field - check the bit mask from list below to verify the mode is supported
        // mimics WAVE_FORMAT_* constants Windows.h for dwFormats capability variable
        static const uint32_t Caps_Mono_11kHz_8bit;
        static const uint32_t Caps_Mono_11kHz_16bit;
        static const uint32_t Caps_Mono_22kHz_8bit;
        static const uint32_t Caps_Mono_22kHz_16bit;
        static const uint32_t Caps_Mono_44kHz_8bit;
        static const uint32_t Caps_Mono_44kHz_16bit;
        static const uint32_t Caps_Mono_96kHz_8bit;
        static const uint32_t Caps_Mono_96kHz_16bit;
        static const uint32_t Caps_Stereo_11kHz_8bit;
        static const uint32_t Caps_Stereo_11kHz_16bit;
        static const uint32_t Caps_Stereo_22kHz_8bit;
        static const uint32_t Caps_Stereo_22kHz_16bit;
        static const uint32_t Caps_Stereo_44kHz_8bit;
        static const uint32_t Caps_Stereo_44kHz_16bit;
        static const uint32_t Caps_Stereo_96kHz_8bit;
        static const uint32_t Caps_Stereo_96kHz_16bit;
    };

#pragma pack(push, 1)
    struct wave_sample_stereo_8bit_type  { uint8_t L=0, R=0; };
    struct wave_sample_stereo_16bit_type { int16_t L=0, R=0; };
#pragma pop
    static_assert(sizeof(wave_sample_stereo_8bit_type) == 2, "SKLIB ** INTERNAL ERROR ** Stereo 8bit sample must occupy 2 bytes. Check Pragma Pack statement.");
    static_assert(sizeof(wave_sample_stereo_16bit_type) == 4, "SKLIB ** INTERNAL ERROR ** Stereo 16bit sample must occupy 4 bytes. Check Pragma Pack statement.");

#include "../include/w32-audio/waveout.hpp"

    class wave_in_type
    {
        std::vector<wave_device_caps_type> device_list;
    };

    class wave_endpoint_control_type
    {
    };
};

#endif // SKLIB_INCLUDED_W32_AUDIO_HPP

