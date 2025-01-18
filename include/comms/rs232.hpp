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

// This file defines public interface to RS-232 interface in OS and places it under namespace sklib entirely.

//for now...
#pragma once

#include "../types.hpp"   //sk ?!!

namespace sklib
{
    namespace priv
    {
        namespace rs232_indexes
        {
            struct props_group
            {
                unsigned offset = 0;
                unsigned width  = 0;
                unsigned mask   = 0;
                unsigned count  = 0;

                constexpr props_group(int idx_count)   // this assumes starting index = 0
                {
                    count = idx_count;
                    width = sklib::bits_rank<unsigned>(idx_count - 1);
                    mask = sklib::bits_data_mask<unsigned>(width);
                }

                constexpr props_group(const props_group& prev, int idx_count) : props_group(idx_count)  // normal index propagation
                {
                    offset =  prev.offset + prev.width;
                }

                static constexpr int onoff_count = 2;
                constexpr props_group(const props_group& prev) : props_group(prev, onoff_count)  // this is for on/off flags i.e. 2 choices
                {}
            };

            // Baud rate - variable
            enum   // table size must be 16, verified later
            {
                b9k6 = 0,         // default, then custom, then specific, then count
                baud_rate_custom, // unlike many other line properties, baud rate can potentially be variable, so it CAN be user-defined
                b75,    bk11,   bk3,    bk6,    b1k2,
                b2k4,   b3k6,   b4k8,   b7k2,   b14k4,
                b19k2,  b38k4,  b57k6,  b115k2,
                baud_rate_idx_count
            };

            // Data bits - fixed choice
            enum { dt8=0, dt5, dt6, dt7, data_bits_idx_count };

            // Party - fixed choice: none, odd, even, mark, space
            enum { prno=0, prod, prev, prmk, prsp, parity_idx_count };

            // Stop bits - fixed choice
            // by legacy conventions: 2 stop bits are for data sizes 6, 7, and 8 bits;
            // 1.5 stop bits for data size 5 (i.e. time delay before next data packet is 1.5 clocks)
            enum { st1=0, st2, stop_bits_idx_count };

            // Flow control - fixed choice, may be extended in the future
            // this module is intended for cross-platform compatibility: Windows/Linux
            // lets use simplified subset suitable for both platforms
            // supported modes: none, Xon/Xoff, Rts/Cts
            // Dtr/Dsr while supported in Windows, *not* supported in Linux
            // Dtr and Rts outs may also be controlled
            enum { rts_def=0, rts_1, rts_0, rts_cts, rts_idx_count };
            enum { dtr_1=0, dtr_0, dtr_def, dtr_idx_count };
            enum { xonf_none = 0, xonf_std, xonf_custom, xonf_idx_count };  //xon-xoff

            // Buffer size - variable
            enum
            {
                buffer_size_default = 0,
                buffer_size_custom,
                bf1k, bf2k, bf4k, bf8k, bf16k, bf32k,
                buffer_size_idx_count
            };

            // Timeouts
            enum
            {
                recv_monitor = 0,  // no timeout on receive: read function returns immediately with any characters arrived
                recv_forever,      // no timeout; will lock execution until something is received
                trcv_custom,
                trcv1s, trcv3s, trcv10s, trcv30s,   // 1s, 3s, 10s, and 30s
                timeout_recv_idx_count
            };
            enum
            {
                tsnd_none = 0,   // not used
                tsnd_custom,
                tsnd1s, tsnd3s, tsnd10s, tsnd30s,   // 1s, 3s, 10s, and 30s
                timeout_send_idx_count
            };

            static constexpr props_group baud_rate_gp       {                  baud_rate_idx_count  };
            static constexpr props_group data_bits_gp       { baud_rate_gp,    data_bits_idx_count };
            static constexpr props_group parity_gp          { data_bits_gp,    parity_idx_count };
            static constexpr props_group stop_bits_gp       { parity_gp,       stop_bits_idx_count };

            static constexpr props_group rts_gp             { stop_bits_gp,    rts_idx_count };
            static constexpr props_group dtr_gp             { rts_gp,          dtr_idx_count };
            static constexpr props_group xon_xoff_gp        { dtr_gp,          xonf_idx_count };

            static constexpr props_group buff_recv_gp       { xon_xoff_gp,     buffer_size_idx_count };
            static constexpr props_group buff_send_gp       { buff_recv_gp,    buffer_size_idx_count };

            static constexpr props_group timeout_recv_gp    { buff_send_gp,    timeout_recv_idx_count };
            static constexpr props_group timeout_send_gp    { timeout_recv_gp, timeout_send_idx_count };

            static constexpr props_group available_gp       { timeout_send_gp };
            static_assert(available_gp.offset < sklib::bits_width_v<unsigned>,
                          "SKLIB -- INTERNAL ERROR -- RS232: Line properties don\'t fit target type (unsigned int)");


        };
        // rs232_indexes
    };

// -------------------------- exposed -----------------------------

    namespace priv
    {
        struct rs232_internal_workspace_type;
    };

    namespace aux
    {
        struct rs232_custom_options_type
        {
            int  baud_rate;
            char xon_char;
            char xoff_char;
            int64_t buffer_size_recv;
            int64_t buffer_size_send;
            int64_t recv_timeout_ms;
            int64_t send_timeout_ms;
        };
    };

    class serial_io_type
    {
    public:
        serial_io_type();
        ~serial_io_type() { close(); }

        serial_io_type(unsigned com_port_no, unsigned mode = 0, sklib::aux::rs232_custom_options_type* option = nullptr) : serial_io_type()
        { open(com_port_no, mode, option); }

        bool open(unsigned com_port_no, unsigned mode = 0, sklib::aux::rs232_custom_options_type* option = nullptr);
        void close() { if (is_open()) get_error_and_close_hcom(false); }

        bool is_open() const;
        bool is_error_state() const { return error_state; }
        int get_last_system_code() const { return system_error_code; }

        bool read(uint8_t* data);
        bool write(uint8_t data);

        void set_dtr(bool on);
        void set_rts(bool on);
        bool get_dsr();
        bool get_cts();
        void set_break();
        bool is_break() { return break_state; }

    protected:
        int port_no;
        std::unique_ptr<sklib::priv::rs232_internal_workspace_type> port_data;
        bool error_state;
        bool break_state;
        int system_error_code;    // set to ERROR_SUCCESS=0 by default;

        bool get_error_and_close_hcom(bool before = true);  // collect status, close port, and return false
    };

    class serial_usb_io_type : public serial_io_type
    {
    public:
        static unsigned get_com_port_no(unsigned vid, unsigned pid);

        serial_usb_io_type() {}

        serial_usb_io_type(unsigned vid, unsigned pid, unsigned mode = 0, sklib::aux::rs232_custom_options_type* option = nullptr)
            : serial_io_type(get_com_port_no(vid, pid), mode, option) {}

        bool open(unsigned vid, unsigned pid, unsigned mode = 0, sklib::aux::rs232_custom_options_type* option = nullptr)
        { return serial_io_type::open(get_com_port_no(vid, pid), mode, option); }
    };
};


