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

//sk?
#pragma once

#include "../rs232.hpp"

#include <Windows.h>

namespace sklib
{
    namespace priv
    {
        using namespace sklib::priv::rs232_indexes;   // lets keep it local to the short block

        struct rs232_internal_workspace_type
        {
            HANDLE hCom;
        };

#define SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ(type,name,group) type name[group.count] = { 0 }

#define SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(arr,val) { \
 size_t SZ = sizeof((arr)) / sizeof(*(arr));  \
 for (size_t i=0; i<SZ; i++) arr[i] = val;    }


        // how it works compared to traditional indexed arrays
        // instead of filling the array sequentially and relying on the coincidence between the indexes and enums (see, for example, bidx, b9k6, etc),
        // lets explicitly assign the numerical value of the property to the array element at the specific index, defined by its index enum
        // despite that couple checks and assertions must be made, this method positively constructs pairs index-data, compared to the traditional C initialization

        struct rs232_constants_helper_type
        {
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, baud_rate, baud_rate_gp  );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, data_bits, data_bits_gp  );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, parity,    parity_gp     );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, stop_bits, stop_bits_gp  );

            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, rts_control, rts_gp );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, cts_sense,   rts_gp );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, dtr_control, dtr_gp);
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, xon_xoff_control, xon_xoff_gp);
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int, xon_xoff_special, xon_xoff_gp);

            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int64_t, buffer_size_recv, buff_recv_gp    );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int64_t, buffer_size_send, buff_send_gp    );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int64_t, timeout_recv,     timeout_recv_gp );
            SKLIB_INTERNAL_RS232_HELPER_GROUP_TABLEZ( int64_t, timeout_send,     timeout_send_gp );

            static constexpr int KB = 1024;
            static constexpr int ms1s = 1000;

            // hack: lets take some existing but irrelevalt numbers and assign them new action
            static constexpr int DTR_UNUSED = DTR_CONTROL_HANDSHAKE;
            static constexpr int RTS_UNUSED = RTS_CONTROL_TOGGLE;

            // convention for values corresponding to indexes:
            // negative - invalid value, used to indicate "custom" when available, or that it needs to load OS default, otherwise
            // 0 - no action and/or use OS default
            // positive - use this value

            constexpr rs232_constants_helper_type(int novalue)
            {
                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(baud_rate, novalue);
                baud_rate[b75]    = 75;
                baud_rate[bk11]   = CBR_110;
                baud_rate[bk3]    = CBR_300;
                baud_rate[bk6]    = CBR_600;
                baud_rate[b1k2]   = CBR_1200;
                baud_rate[b2k4]   = CBR_2400;
                baud_rate[b3k6]   = 3600;
                baud_rate[b4k8]   = CBR_4800;
                baud_rate[b7k2]   = 7200;
                baud_rate[b9k6]   = CBR_9600;
                baud_rate[b14k4]  = CBR_14400;
                baud_rate[b19k2]  = CBR_19200;
                baud_rate[b38k4]  = CBR_38400;
                baud_rate[b57k6]  = CBR_57600;
                baud_rate[b115k2] = CBR_115200;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(data_bits, novalue);
                data_bits[dt5] = 5;
                data_bits[dt6] = 6;
                data_bits[dt7] = 7;
                data_bits[dt8] = 8;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(parity, novalue);
                parity[prno] = NOPARITY;
                parity[prod] = ODDPARITY;
                parity[prev] = EVENPARITY;
                parity[prmk] = MARKPARITY;
                parity[prsp] = SPACEPARITY;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(stop_bits, novalue);
                stop_bits[st1] = ONESTOPBIT;
                stop_bits[st2] = TWOSTOPBITS;   // caveat: this value becomes ONE5STOPBITS if 5 data bits is set (condition dt5 && st2 is tested separately)

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(rts_control, novalue);
                rts_control[rts_1]   = RTS_CONTROL_ENABLE;
                rts_control[rts_0]   = RTS_CONTROL_DISABLE;
                rts_control[rts_cts] = RTS_CONTROL_HANDSHAKE;
                rts_control[rts_def] = RTS_UNUSED;
                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(cts_sense, FALSE);
                cts_sense[rts_cts] = TRUE;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(dtr_control, novalue);
                dtr_control[dtr_1] = DTR_CONTROL_ENABLE;
                dtr_control[dtr_0] = DTR_CONTROL_DISABLE;
                dtr_control[dtr_def] = DTR_UNUSED;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(xon_xoff_control, FALSE);
                xon_xoff_control[xonf_std] = TRUE;
                xon_xoff_control[xonf_custom] = TRUE;
                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(xon_xoff_special, FALSE);
                xon_xoff_special[xonf_custom] = TRUE;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(buffer_size_recv, novalue);
                buffer_size_recv[buffer_size_default] = 0;
                buffer_size_recv[bf1k]  = KB;
                buffer_size_recv[bf2k]  = 2 * KB;
                buffer_size_recv[bf4k]  = 4 * KB;
                buffer_size_recv[bf8k]  = 8 * KB;
                buffer_size_recv[bf16k] = 16 * KB;
                buffer_size_recv[bf32k] = 32 * KB;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(buffer_size_send, novalue);
                buffer_size_send[buffer_size_default] = 0;
                buffer_size_send[bf1k] = KB;
                buffer_size_send[bf2k] = 2 * KB;
                buffer_size_send[bf4k] = 4 * KB;
                buffer_size_send[bf8k] = 8 * KB;
                buffer_size_send[bf16k] = 16 * KB;
                buffer_size_send[bf32k] = 32 * KB;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(timeout_recv, novalue);
                timeout_recv[recv_monitor] = 0;
                timeout_recv[trcv1s]  = ms1s;
                timeout_recv[trcv3s]  = 3 * ms1s;
                timeout_recv[trcv10s] = 10 * ms1s;
                timeout_recv[trcv30s] = 30 * ms1s;
                timeout_recv[recv_forever] = (DWORD)MAXDWORD;

                SKLIB_INTERNAL_RS232_HELPER_FILL_ARRAY(timeout_send, novalue);
                timeout_send[tsnd_none] = 0;
                timeout_send[tsnd1s] = ms1s;
                timeout_send[tsnd3s] = 3 * ms1s;
                timeout_send[tsnd10s] = 10 * ms1s;
                timeout_send[tsnd30s] = 30 * ms1s;
            }
        };

        struct rs232_props
        {
            static constexpr int novalue = -1;
            static constexpr rs232_constants_helper_type TAB{ novalue };

            // tests...
            static_assert(TAB.baud_rate[baud_rate_custom] < 0, "SKLIB -- INTERNAL ERROR -- RS232: Custom baud rate is invalid");
            static_assert(TAB.buffer_size_recv[buffer_size_custom] < 0, "SKLIB -- INTERNAL ERROR -- RS232: Custom buffer size is invalid");
            static_assert(TAB.buffer_size_send[buffer_size_custom] < 0, "SKLIB -- INTERNAL ERROR -- RS232: Custom buffer size is invalid");
            static_assert(TAB.timeout_recv[trcv_custom] < 0, "SKLIB -- INTERNAL ERROR -- RS232: Custom RX timeout is invalid");
            static_assert(TAB.timeout_send[tsnd_custom] < 0, "SKLIB -- INTERNAL ERROR -- RS232: Custom TX timeout is invalid");

            int baud_rate = novalue;    // negative "no value" will convert to some default behavior
            int data_bits = novalue;
            int parity    = novalue;
            int stop_bits = novalue;
            int rts_control = novalue;
            int cts_sense = FALSE;
            int dtr_control = novalue;
            int xon_xoff_control = novalue;
            int xon_xoff_special = FALSE;
            char xon_sym = 0;
            char xoff_sym = 0;
            int64_t buffer_size_recv = novalue;
            int64_t buffer_size_send = novalue;
            int64_t timeout_ms_recv = novalue;
            int64_t timeout_ms_send = novalue;

            unsigned index_norm(const sklib::priv::rs232_indexes::props_group& group, unsigned mode, unsigned grp_count)
            {
                if (mode < 0) mode = 0;
                unsigned idx = (mode >> group.offset) & group.mask;
                return ((idx < grp_count) ? idx : 0);
            }

            template<class T, unsigned N>
            auto index_2_value(const sklib::priv::rs232_indexes::props_group& group, unsigned mode, const T (&arr)[N], const T* option = nullptr)
            {
                auto R = arr[index_norm(group, mode, N)];
                return (R >= 0 ? R : (option ? *option : arr[0]));
            }

            // Parses input and fills data structures for opening a serial port
            // If "custom" option is called but custom_options_ptr is nullprt, this option assumes default value
            // Indexes for default values, if any, are replaced with actual default values

            constexpr rs232_props(unsigned mode, sklib::aux::rs232_custom_options_type* option)
            {
#define PMEMBER(pack,field) ((pack) ? &((pack)->field) : nullptr)

                using namespace sklib::priv::rs232_indexes;   // keep local

                baud_rate = index_2_value(baud_rate_gp, mode, TAB.baud_rate, PMEMBER(option, baud_rate));
                data_bits = index_2_value(data_bits_gp, mode, TAB.data_bits);
                parity    = index_2_value(parity_gp, mode, TAB.parity);
                stop_bits = index_2_value(stop_bits_gp, mode, TAB.stop_bits);

                rts_control = index_2_value(rts_gp, mode, TAB.rts_control);
                cts_sense   = index_2_value(rts_gp, mode, TAB.cts_sense);
                dtr_control = index_2_value(dtr_gp, mode, TAB.dtr_control);
                xon_xoff_control = index_2_value(xon_xoff_gp, mode, TAB.xon_xoff_control);
                xon_xoff_special = index_2_value(xon_xoff_gp, mode, TAB.xon_xoff_special);

                buffer_size_recv = index_2_value(buff_recv_gp, mode, TAB.buffer_size_recv, PMEMBER(option, buffer_size_recv));
                buffer_size_send = index_2_value(buff_send_gp, mode, TAB.buffer_size_send, PMEMBER(option, buffer_size_send));
                timeout_ms_recv = index_2_value(timeout_recv_gp, mode, TAB.timeout_recv, PMEMBER(option, recv_timeout_ms));
                timeout_ms_send = index_2_value(timeout_send_gp, mode, TAB.timeout_send, PMEMBER(option, send_timeout_ms));

                if (data_bits == TAB.data_bits[dt5] && stop_bits != ONESTOPBIT) stop_bits = ONE5STOPBITS;

                xon_sym = sklib::ascii::XON;
                xoff_sym = sklib::ascii::XOFF;
                if (xon_xoff_special && option)
                {
                    xon_sym = option->xon_char;
                    xoff_sym = option->xoff_char;
                }
            }
        };
    };
};

sklib::serial_io_type::serial_io_type() : error_state(false), break_state(false), system_error_code(ERROR_SUCCESS), port_no(0)
{
    typedef sklib::priv::rs232_internal_workspace_type workspace_type;
    port_data = std::unique_ptr<workspace_type>(new workspace_type);

    port_data->hCom = INVALID_HANDLE_VALUE;
}

bool sklib::serial_io_type::open(unsigned com_port_no, unsigned mode, sklib::aux::rs232_custom_options_type* option)
{
    using namespace sklib::priv::rs232_indexes;   // keep local

    if (is_open()) close();

    error_state = false;
    system_error_code = ERROR_NOT_SUPPORTED;
    port_data->hCom = INVALID_HANDLE_VALUE;
    port_no = com_port_no;
    if (com_port_no <= 0) return false;

    std::string port_name = "\\\\.\\COM" + std::to_string(com_port_no);
    port_data->hCom = CreateFileA(port_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (port_data->hCom == INVALID_HANDLE_VALUE)
    {
        system_error_code = GetLastError();
        return false;
    }

    sklib::priv::rs232_props Props{ mode, option };

    DCB sdcb;
    if (!GetCommState(port_data->hCom, &sdcb)) return get_error_and_close_hcom();

    sdcb.BaudRate = Props.baud_rate;
    sdcb.ByteSize = Props.data_bits;
    sdcb.Parity   = Props.parity;
    sdcb.StopBits = Props.stop_bits;

    if (Props.rts_control != Props.TAB.RTS_UNUSED)
    {
        sdcb.fRtsControl  = Props.rts_control;
        sdcb.fOutxCtsFlow = Props.cts_sense;
    }
    if (Props.dtr_control != Props.TAB.DTR_UNUSED)
    {
        sdcb.fDtrControl  = Props.dtr_control;
        sdcb.fOutxDsrFlow = FALSE;
    }

    sdcb.fOutX =
    sdcb.fInX  = Props.xon_xoff_control;
    sdcb.XonChar  = Props.xon_sym;
    sdcb.XoffChar = Props.xoff_sym;

    if (!SetCommState(port_data->hCom, &sdcb)) return get_error_and_close_hcom();

    COMMTIMEOUTS stimeouts;
    ZeroMemory(&stimeouts, sizeof(stimeouts));
    stimeouts.ReadIntervalTimeout         = (Props.timeout_ms_recv ? 0 : (DWORD)MAXWORD);
    stimeouts.ReadTotalTimeoutMultiplier  = 0;
    stimeouts.ReadTotalTimeoutConstant    = (DWORD)Props.timeout_ms_recv;
    stimeouts.WriteTotalTimeoutMultiplier = 0;
    stimeouts.WriteTotalTimeoutConstant   = (DWORD)Props.timeout_ms_send;

    if (!SetCommTimeouts(port_data->hCom, &stimeouts)) return get_error_and_close_hcom();

    COMMPROP scomprop;
    if (!GetCommProperties(port_data->hCom, &scomprop)) return get_error_and_close_hcom();

    DWORD rdbuf = (DWORD)Props.buffer_size_recv;
    DWORD wrbuf = (DWORD)Props.buffer_size_send;
    if (!SetupComm(port_data->hCom, (rdbuf ? rdbuf : scomprop.dwMaxRxQueue), (wrbuf ? wrbuf : scomprop.dwMaxTxQueue))) return get_error_and_close_hcom();

    return true;
}

bool sklib::serial_io_type::is_open() const
{
    return (port_data->hCom == INVALID_HANDLE_VALUE);
}

bool sklib::serial_io_type::get_error_and_close_hcom(bool before)
{
    if (before) system_error_code = GetLastError();
    CloseHandle(port_data->hCom);
    if (!before) system_error_code = GetLastError();

    port_data->hCom = INVALID_HANDLE_VALUE;
    error_state = false;
    return false;
}

bool sklib::serial_io_type::read(uint8_t* data)
{
    system_error_code = ERROR_NOT_SUPPORTED;
    if (!is_open()) return false;

    DWORD rd_count = 0;
    error_state = !ReadFile(port_data->hCom, data, 1, &rd_count, NULL);

    system_error_code = GetLastError();
    return (!error_state && rd_count);
}

bool sklib::serial_io_type::write(uint8_t data)
{
    system_error_code = ERROR_NOT_SUPPORTED;
    if (!is_open()) return false;

    DWORD wr_count = 0;
    error_state = (!WriteFile(port_data->hCom, &data, 1, &wr_count, NULL) || !wr_count);

    system_error_code = GetLastError();
    return !error_state;
}

unsigned sklib::serial_usb_io_type::get_com_port_no(unsigned vid, unsigned pid)
{

}

