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
// See file: "dll.hpp" for details how to use the split-header arrangement.

#ifndef SKLIB_INCLUDED_DLL_IMPLEMENTATION
#define SKLIB_INCLUDED_DLL_IMPLEMENTATION


#ifndef SKLIB_INCLUDED_DLL_HPP
#include "../include/dll.hpp"
#endif


#if defined(_MSC_VER)
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#elif defined(__GNUC__)
#include <errno.h>
#include <dlfcn.h>

#endif

struct sklib::priv::dll_internal_workspace_type
{
    HMODULE hdll = NULL;
};

// constructor and destructor may visit the default destructor for the "internal" type
sklib::priv::dll_interface_helper::dll_interface_helper() {}
sklib::priv::dll_interface_helper::~dll_interface_helper() {}

bool sklib::priv::dll_interface_helper::open_dll_impl(const SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE* dll_name, uint32_t caller_mode, bool request_mode_check)
{
    if (!handle) handle = std::make_unique<dll_internal_workspace_type>();
    if (is_dll_open()) close_dll();

    handle->hdll = ::LoadLibrary(dll_name);
    dll_info.sys_dll_error = ::GetLastError();

    if (!is_dll_open())
    {
        dll_info.dll_load_status = dll_info.dll_status_code.LOAD_FAILURE;
        return false;
    }

    dll_info.dll_load_status = dll_info.dll_status_code.OK;   // and use inductive extension

    if (request_mode_check)
    {
        dll_generic_entry beacon = nullptr;
        get_address(SKLIB_MACRO_EXPAND_TO_STRING(SKLIB_INTERNAL_DLL_BEACON_FUNCTION), false, beacon, dll_info.sys_dll_error);
        if (!beacon) dll_info.dll_load_status |= dll_info.dll_status_code.BAREFOOT_LIBRARY;
        if (caller_mode != get_remote_calling_mode(beacon)) dll_info.dll_load_status |= dll_info.dll_status_code.CALLING_MODE_MISMATCH;
    }

    if (dll_info.is_dll_usable()) return true;

    close_dll(false);
    return false;
}

void sklib::priv::dll_interface_helper::close_dll(bool update_load_status)
{
    if (!is_dll_open()) return;

    ::FreeLibrary(handle->hdll);
    handle->hdll = nullptr;

    dll_info.sys_dll_error = ::GetLastError();  // just in case
    if (update_load_status) dll_info.dll_load_status = dll_info.dll_status_code.NOT_LOADED;
}

bool sklib::priv::dll_interface_helper::is_dll_open()
{
    return handle && handle->hdll;
}

void sklib::priv::dll_interface_helper::get_address(const char* symbol, bool required,
                                                        sklib::priv::dll_generic_entry& address, unsigned& sys_load_error)
{
    if (is_dll_open())
    {
        static_assert(sizeof(dll_generic_entry) == sizeof(address), "SKLIB ** INTERNAL ERROR ** DLL Function Address Mismatch (Load)");
        address = reinterpret_cast<dll_generic_entry>(::GetProcAddress(handle->hdll, symbol));
        sys_load_error = ::GetLastError();

        if (!address) dll_info.dll_load_status |= (required ? dll_info.dll_status_code.MISSING_REQUIRED_FUNCTION
                                                            : dll_info.dll_status_code.MISSING_OPTIONAL_FUNCTION);
    }
    else
    {
        address = nullptr;
        sys_load_error = dll_info.sys_dll_error;
        dll_info.dll_load_status |= dll_info.dll_status_code.LOAD_FAILURE;
    }
}

#endif // SKLIB_INCLUDED_DLL_IMPLEMENTATION

