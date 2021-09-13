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

// This file defines public interface to socket layer and places it under namespace sklib entirely.
// 
// The actual working code is moved to separate file: "comms-code.hpp" in order to eliminate namespace pollution
// coming from WinSock headers / other global standard libraries headers.
// SKLib uses headers-only approach where possible. Instead of using precompiled libraries, the suggested use
// is as follows (see "test-comms-sockets" project for example): 
// - use this file: "comms.hpp" anywhere where you USE sockets;
// - dedicate a single .cpp file for implementation, and place the single line in it: #include <SKLib/comms-code.hpp>
// This way, all code that can clobber usable names is contained in its own compilation unit.

// For more information on WinSock itself, refer to MSDN, and also:
//      FAQ: https://tangentsoft.net/wskfaq/
//      I/O example: https://tangentsoft.net/wskfaq/examples/basics/select-server.cpp

#ifndef SKLIB_INCLUDED_COMMS_HPP
#define SKLIB_INCLUDED_COMMS_HPP

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include<memory>
#include<string>
#endif

namespace sklib
{
    namespace internal
    {
        struct stream_tcpip_opaque_workspace_type;
    };

    class stream_tcpip_type
    {
    private:
        bool error_state = false;
        int system_error_code;    // set to ERROR_SUCCESS=0 by default;

    public:
        static constexpr char loopback_ip_address[] = "127.0.0.1";

        stream_tcpip_type(bool server, uint16_t port_no, const std::string& net_address = loopback_ip_address);
        ~stream_tcpip_type();

        bool is_error_state() const      { return error_state; }
        int get_last_system_code() const { return system_error_code; }

        bool is_server() const;
        bool is_connected() const;

        bool can_read();
        bool can_write();

        bool read(uint8_t* data);
        bool write(uint8_t data);

    private:
        static const int MAXSOCKS_PIPE = 3;   // some small number

        std::unique_ptr<::sklib::internal::stream_tcpip_opaque_workspace_type> stream_data;

        // close all sockets, shutdown service, and set error state
        bool net_cleanup_with_error();

        // returns true if and only if the subsequent action can commence without blocking
        // false in return may indicate error state (caller must verify)
        enum class net_query_type { probe_can_accept = 0, probe_can_read, probe_can_write };
        bool net_can_proceed(net_query_type what);

        // returns true if no error
        bool net_update_server_status();

        // if server, drop active socket and return to listening; if client set error state
        bool net_close_connection();
    };
};

#endif // SKLIB_INCLUDED_COMMS_HPP
