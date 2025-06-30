// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// This file contains all the calls to external library(ies).
// Any system/standard header specific to the functions used is also included exclusively here.
// See file: "w32-audio.hpp" for details how to use the split-header arrangement.

#ifndef SKLIB_INCLUDED_W32_AUDIO_IMPLEMENTATION
#define SKLIB_INCLUDED_W32_AUDIO_IMPLEMENTATION

// code with actual usage of library function
// keep it away from the main compilation unit to avoid namespace pollution

// all definitions are in "exported" header
#include "../include/w32-audio.hpp"

#include "Windows.h"

namespace sklib
{
    const uint32_t wave_device_caps_type::Caps_Mono_11kHz_8bit = WAVE_FORMAT_1M08;
    const uint32_t wave_device_caps_type::Caps_Mono_11kHz_16bit = WAVE_FORMAT_1M16;
    const uint32_t wave_device_caps_type::Caps_Mono_22kHz_8bit = WAVE_FORMAT_2M08;
    const uint32_t wave_device_caps_type::Caps_Mono_22kHz_16bit = WAVE_FORMAT_2M16;
    const uint32_t wave_device_caps_type::Caps_Mono_44kHz_8bit = WAVE_FORMAT_4M08;
    const uint32_t wave_device_caps_type::Caps_Mono_44kHz_16bit = WAVE_FORMAT_4M16;
    const uint32_t wave_device_caps_type::Caps_Mono_96kHz_8bit = WAVE_FORMAT_96M08;
    const uint32_t wave_device_caps_type::Caps_Mono_96kHz_16bit = WAVE_FORMAT_96M16;
    const uint32_t wave_device_caps_type::Caps_Stereo_11kHz_8bit = WAVE_FORMAT_1S08;
    const uint32_t wave_device_caps_type::Caps_Stereo_11kHz_16bit = WAVE_FORMAT_1S16;
    const uint32_t wave_device_caps_type::Caps_Stereo_22kHz_8bit = WAVE_FORMAT_2S08;
    const uint32_t wave_device_caps_type::Caps_Stereo_22kHz_16bit = WAVE_FORMAT_2S16;
    const uint32_t wave_device_caps_type::Caps_Stereo_44kHz_8bit = WAVE_FORMAT_4S08;
    const uint32_t wave_device_caps_type::Caps_Stereo_44kHz_16bit = WAVE_FORMAT_4S16;
    const uint32_t wave_device_caps_type::Caps_Stereo_96kHz_8bit = WAVE_FORMAT_96S08;
    const uint32_t wave_device_caps_type::Caps_Stereo_96kHz_16bit = WAVE_FORMAT_96S16;
};

struct sklib::wave_out_type::handle_type::internal_internal_resource_type
{
    HWAVEOUT m_hWaveOut;
    WAVEHDR* m_hWaveOutHdrArr;
    WAVEFORMATEX pFmt;
};

void sklib::wave_out_type::RefreshList()
{
    device_list.clear();
    int N = waveOutGetNumDevs();

    for (int i=0; i<N; i++)
    {
        WAVEOUTCAPS woutcaps{ 0 };
        device_list.push_back((waveOutGetDevCaps(i, &woutcaps, sizeof(woutcaps)) == MMSYSERR_NOERROR)
                               ? wave_device_caps_type{ woutcaps.szPname, woutcaps.dwFormats, woutcaps.dwSupport, (woutcaps.wChannels > 1) }
                               : wave_device_caps_type());
    }
}

sklib::wave_out_type::handle_type sklib::wave_out_type::OpenDevice(unsigned device_no, int sample_rate, int bits, bool stereo,
                                                                   int queue_count, mode_type mode)
{
    handle_type H;
    H.Resource = 
    CloseDevice();
    if (ordno < 0 || ordno >= num_devices ||
        sample_rate <= 0 || blkdepth < CAUDOUT_MIN_BLOCK_COUNT) return false;
    if (mode != CAUDOUT_GREEDY && mode != CAUDOUT_QSYNC && mode != CAUDOUT_BLOCKS) return false;

    elem_size = 0;
    if (bits == CAUDIN_BITS_8) elem_size = 1;
    if (bits == CAUDIN_BITS_16) elem_size = 2;
    if (stereo) elem_size *= 2;
    if (!elem_size) return false;

    // get memory
    m_hWaveOutHdrArr = (WAVEHDR*)malloc(sizeof(WAVEHDR) * blkdepth);
    if (NULL == m_hWaveOutHdrArr)
    {
        CloseDevice();	// safe to call it from here
        return false;
    }

    // open

    pFmt.cbSize = 0;
    pFmt.wFormatTag = WAVE_FORMAT_PCM;
    pFmt.nChannels = (stereo ? 2 : 1);
    pFmt.nSamplesPerSec = sample_rate;
    pFmt.wBitsPerSample = bits;
    pFmt.nBlockAlign = pFmt.nChannels * bits / 8;		// always whole
    pFmt.nAvgBytesPerSec = pFmt.nChannels * pFmt.nSamplesPerSec * pFmt.wBitsPerSample / 8;
    if (waveOutOpen(&m_hWaveOut, ordno, &pFmt, (DWORD_PTR)CAUDOUT_CALLBACK, (DWORD_PTR)this,
        CALLBACK_FUNCTION + WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR)
    {
        m_hWaveOut = NULL;
        CloseDevice();
        return false;	// Keep It Simple
    }

    // just in case...
    waveOutSetVolume(m_hWaveOut, 0xFFFFFFFFUL);		// all that is, play "as-is"
    waveOutSetPlaybackRate(m_hWaveOut, 0x00010000);
    waveOutSetPitch(m_hWaveOut, 0x00010000);

    // buffer

    for (int i = 0; i < blkdepth; i++)
    {
        m_hWaveOutHdrArr[i].lpData = NULL;		// these two are populated by PutFragment()
        m_hWaveOutHdrArr[i].dwBufferLength = 0;

        m_hWaveOutHdrArr[i].dwBytesRecorded = 0;
        m_hWaveOutHdrArr[i].dwUser = i;
        m_hWaveOutHdrArr[i].dwFlags = 0;
        m_hWaveOutHdrArr[i].dwLoops = 1;
    }

    // params... only update those not set by default

    playmode = mode;
    blocks_count = blkdepth;

    selected_device = ordno;
    update_refuse = false;

    // this is it!

//	CString S;
//	S.Format("ba=%d; bv=%d; bc=%d; pm=%d; es=%d; seld=%d", (int)block_active, (int)block_vacant, blocks_count, playmode, elem_size, selected_device);
//	AfxMessageBox(S);

    return true;

}


#endif // SKLIB_INCLUDED_W32_AUDIO_IMPLEMENTATION

