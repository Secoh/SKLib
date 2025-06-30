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

// This is internal SKLib file and must NOT be included directly.

class wave_out_type
{
public:
    enum class mode_type { GREEDY=0,    // allocate memory for each new block, copy from caller buffer, then free memory after playback
                           IMPORT,      // do nothing with memory; caller supplies buffer and must keep it alive until playback is done
                           BLOCKS,      // caller must send blocks of the same size - caller's data is copied to internal buffers
                           EXPORT };    // caller receives address of the next block to fill; blocks are of the same size

    static const int MIN_QUEUE_SIZE = 2;

    class handle_type
    {
        friend wave_out_type;

    protected:
        struct internal_internal_resource_type;
        std::unique_ptr<internal_internal_resource_type> Resource;

        // call the respective function(s) on itself - hide actual system calls behind the linker
        void system_waveOutReset_this() const;
        void system_waveOutUnprepareHeader_this(int block_handle) const;
        void system_waveOutClose_this() const;



        int AddFragment_Impl(const void* data, int sample_count);

        int playmode, elem_size, blocks_count, block_size;
        int64_t block_active, block_vacant;     // active and vacant are through counters; "write" moves vacant, "read" moves active
        bool update_busy, update_refuse;

    public:
        int AddFragment(const uint8_t* data, int sample_count = 0)      { return AddFragment_Impl(data, sample_count); }
        int AddFragment(const int16_t* data, int sample_count = 0)      { return AddFragment_Impl(data, sample_count); }
        int AddFragment(const sklib::wave_sample_stereo_8bit_type* data, int sample_count = 0)   { return AddFragment_Impl(data, sample_count); }
        int AddFragment(const sklib::wave_sample_stereo_16bit_type* data, int sample_count = 0)  { return AddFragment_Impl(data, sample_count); }

        bool isClearToSend() const
        {

        }

        bool isBufferIdle(int block_handle) const
        {

        }

        bool isPlaybackDone() const
        {

        }

        void Reset();
        void Close();

        ~handle_type()
        {

        }
    };

    std::vector<sklib::wave_device_caps_type> device_list;
    void RefreshList();

    handle_type OpenDevice(unsigned device_no, int sample_rate, int bits, bool stereo, int queue_count = MIN_QUEUE_SIZE+1, mode_type mode = mode_type::GREEDY);

    handle_type OpenDevice(unsigned device_no, uint32_t format = sklib::wave_device_caps_type::Caps_Mono_11kHz_8bit,
                           int queue_count = MIN_QUEUE_SIZE+1, mode_type mode = mode_type::GREEDY)
    {

    }

    wave_out_type()
    {
        RefreshList();
    }


};

