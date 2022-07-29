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
// See file: "comms.hpp" for details how to use the split-header arrangement.

#ifndef SKLIB_INCLUDED_SOCKET_IMPLEMENTATION
#define SKLIB_INCLUDED_SOCKET_IMPLEMENTATION

// code with actual usage of library function
// keep it away from the main compilation unit to avoid namespace pollution

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

// linker request
#pragma comment(lib, "ws2_32.lib")

// all definitions are in "exported" header
#include "../include/comms.hpp"

namespace sklib
{
    namespace internal
    {
        struct stream_tcpip_opaque_workspace_type
        {
            WSADATA wsaData;
            SOCKET ListenSocket = INVALID_SOCKET;
            SOCKET NetSocket    = INVALID_SOCKET;
        };
    };
};

bool sklib::stream_tcpip_type::is_server() const
{
    return (INVALID_SOCKET != stream_data->ListenSocket);
}

bool sklib::stream_tcpip_type::is_connected() const
{
    return (INVALID_SOCKET != stream_data->NetSocket);
}

sklib::stream_tcpip_type::stream_tcpip_type(bool server, uint16_t port_no, const std::string& net_address)
{
    system_error_code = ERROR_SUCCESS;

    typedef ::sklib::internal::stream_tcpip_opaque_workspace_type workspace_type;
    stream_data = std::unique_ptr<workspace_type>(new workspace_type);

    // 2021: the supported Winsock version is 2.2.x since 1996
    if (WSAStartup(MAKEWORD(2, 2), &(stream_data->wsaData)))
    {
        net_cleanup_with_error();
        return;
    }

    // TCP/IP stack is almost guaranteed on modern OS environment
    SOCKET& StartupSocket = (server ? stream_data->ListenSocket : stream_data->NetSocket);
    StartupSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (StartupSocket == INVALID_SOCKET)
    {
        net_cleanup_with_error();
        return;
    }

    // Configure TCP/IP v4 service
    sockaddr_in service;
    service.sin_family = AF_INET;
    inet_pton(AF_INET, net_address.c_str(), (PVOID)&service.sin_addr.s_addr);
    service.sin_port = htons(port_no);

    // Start listening (server) or connect (client)
    if (server)
    {
        if (SOCKET_ERROR == bind(stream_data->ListenSocket, (SOCKADDR*)&service, sizeof(service)))
        {
            net_cleanup_with_error();
            return;
        }

        if (SOCKET_ERROR == listen(stream_data->ListenSocket, MAXSOCKS_PIPE))
        {
            net_cleanup_with_error();
            return;
        }
    }
    else
    {
        if (SOCKET_ERROR == connect(stream_data->NetSocket, (SOCKADDR*)&service, sizeof(service)))
        {
            net_cleanup_with_error();
            return;
        }
    }
}

sklib::stream_tcpip_type::~stream_tcpip_type()
{
    if (!error_state) net_cleanup_with_error();
}

bool sklib::stream_tcpip_type::can_read()
{
    net_update_server_status();
    if (error_state || !is_connected()) return false;
    return net_can_proceed(net_query_type::probe_can_read);
}

bool sklib::stream_tcpip_type::can_write()
{
    net_update_server_status();
    if (error_state || !is_connected()) return false;
    return net_can_proceed(net_query_type::probe_can_write);
}

bool sklib::stream_tcpip_type::read(uint8_t* data)
{
    if (!can_read()) return false;

    uint8_t d = 0;
    if (recv(stream_data->NetSocket, (char*)&d, sizeof(char), 0) != sizeof(char)) return net_close_connection();

    *data = d;
    return true;
}

bool sklib::stream_tcpip_type::write(uint8_t data)
{
    if (!can_write()) return false;
    if (send(stream_data->NetSocket, (char*)&data, sizeof(char), 0) != sizeof(char)) return net_close_connection();
    return true;
}

bool sklib::stream_tcpip_type::net_cleanup_with_error()
{
    error_state = true;
    system_error_code = WSAGetLastError();

    if (is_connected()) closesocket(stream_data->NetSocket);
    if (is_server()) closesocket(stream_data->ListenSocket);

    WSACleanup();

    stream_data->NetSocket = INVALID_SOCKET;
    stream_data->ListenSocket = INVALID_SOCKET;

    return false;
}

// returns true if and only if the subsequent action can commence without blocking
// false in return may indicate error state (caller must verify)
bool sklib::stream_tcpip_type::net_can_proceed(net_query_type what)
{
    if (what != net_query_type::probe_can_accept && what != net_query_type::probe_can_read && what != net_query_type::probe_can_write) net_cleanup_with_error();
    if (error_state) return net_cleanup_with_error();

    SOCKET& sock = ((what == net_query_type::probe_can_accept) ? stream_data->ListenSocket : stream_data->NetSocket);

    static constexpr timeval poll_mode{ 0, 0 };
    fd_set IOFD, ExFD;
    FD_ZERO(&IOFD);     // ReadFD(s) for probin accept() and rcvd(); WriteFD(s) for send()
    FD_ZERO(&ExFD);     // for verifying error state
    FD_SET(sock, &IOFD);
    FD_SET(sock, &ExFD);

    int select_code = SOCKET_ERROR;
    switch (what)
    {
    case net_query_type::probe_can_accept:
    case net_query_type::probe_can_read:    select_code = select(0, &IOFD, nullptr, &ExFD, &poll_mode);
        break;
    case net_query_type::probe_can_write:   select_code = select(0, nullptr, &IOFD, &ExFD, &poll_mode);
        break;
    }

    return ((SOCKET_ERROR == select_code) ? net_cleanup_with_error() : (select_code > 0));
}

bool sklib::stream_tcpip_type::net_update_server_status()
{
    if (error_state || !is_server() || is_connected()) return error_state;

    if (!net_can_proceed(net_query_type::probe_can_accept)) return error_state;  // and if there is error, error state is set

    stream_data->NetSocket = accept(stream_data->ListenSocket, nullptr, nullptr);
    if (!is_connected()) return net_cleanup_with_error();

    return true;
}

bool sklib::stream_tcpip_type::net_close_connection()  // if server, drop active socket and return to listening.
{                                                      // if client set error state
    if (error_state || !is_connected()) return false;

    if (is_server())
    {
        system_error_code = WSAGetLastError();
        closesocket(stream_data->NetSocket);
        stream_data->NetSocket = INVALID_SOCKET;
    }
    else
    {
        net_cleanup_with_error();
    }
    return false;
}

#endif // SKLIB_INCLUDED_COMMS_IMPLEMENTATION
