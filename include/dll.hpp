// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// This file defines public interface to dynamic library functions and places them under sklib namespace entirely.
//
// The actual working code is moved to separate CODE file in order to eliminate namespace pollution coming from global
// System API headers (such as Windows.h).
// SKLib uses headers-only approach where possible. Instead of using precompiled libraries, the user shall dedicate
// compilation unit for CODE file, so there can be no name pollution at compilation time. Specifically, user needs to:
// - include this file: "dll.hpp" anywhere where you USE dynamic link libraries;
// - dedicate a single .cpp file for "library implementation", and place the single line in it:
//   #include <SKLib/source/dll-code.hpp>
// This way, any user code "doesn't know" about system-specific libraries, and the system functionality is accessed
// at the link time.


// DLL options (preprocessor variables defined before #include "dll.hpp"):
//
//    SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT     DLL will have instance-specific "global" variables, like TLS
//    SKLIB_DLL_ENABLE_THROW                     If enabled, call to missing function will throw runtime error
//    SKLIB_DLL_DISABLE_RUNTIME_CHECK            Normally, DLL has self-check for calling mode compatibility
//    SKLIB_DLL_EXPORT                           If set, dll.hpp header is for exporting symbols; otherwise, loading

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include "utility.hpp"

#ifdef SKLIB_DLL_ENABLE_THROW
  #include <stdexcept>
#endif

// Convention:
// any members other than defined by the user, are prefixed by "sklib_internal" to avoid collision with user DLL space
// declaration:    class interface_name : public base_interface_class
//                 { ...declarations... dll_func(); dll_func(); ... };
// instantination: interface_name instance; *OR* interface_name instance("dll_file_name");
// open dll:       instance("dll_file_name");
// close dll:      instance(nullptr); or use "", empty name, or destroying the instance
// test:           is open and usable: explicit (bool)instance
// access to data segment: *instance;
// access to members:      instance->member
// access to global status: instance() returns reference to internal struct with DLL status and OS last error
// call function:  func(params...)
// test function:  explicit (bool)func returns true if function is loaded

// Calling Mode:
    // SKLib DLL interface may use different conventions of calling, function headedrs, list of parameters, etc.
    // They are established at compilation time by defining certain C macros before inclusion of dll.h header.
    // Care shall be taken that the DLL interface declaration is exactly the same for DLL import and DLL export,
    // preferrably by reusing the same DLL declaration file (see example for recommended arrangement).
    //
    // To ensure that the caller and the callee are compatible, a barefoot DLL function is employed ("beacon").
    // It always has the same calling converntion, takes no parameters, and transmits the bit fiels, where
    // each bit corresponds to the respective dll.h option. If no beacon is found, return 0 is assumed.
    //
    // Nov 2022: The only supported option is SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT


#ifdef SKLIB_INCLUDED_DLL_HPP
  #error Header dll.hpp must NOT be included more than once in any compilation unit. (Prevents certain mistakes.)
#endif
#define SKLIB_INCLUDED_DLL_HPP

#ifndef __cplusplus
  #error SKLIB DLL ** Target platform is C++ (tested C++14 and up)
#endif
#if defined(_MSC_VER)
  #define SKLIB_INTERNAL_DLL_VISIBILITY __declspec(dllexport)
  #define SKLIB_INTERNAL_DLL_CDECL __cdecl
#elif defined(__GNUC__)
  #define SKLIB_INTERNAL_DLL_VISIBILITY __attribute__ ((visibility(default)))
  #define SKLIB_INTERNAL_DLL_CDECL
#else
  #error Sorry, only GNU C++ and MS Visual Studio are supported
#endif

#ifdef SKLIB_DLL_DISABLE_RUNTIME_CHECK
    #define SKLIB_INTERNAL_DLL_DO_RUNTIME_CHECK false
#else
    #define SKLIB_INTERNAL_DLL_DO_RUNTIME_CHECK true
#endif

#define SKLIB_INTERNAL_DLL_BEACON_FUNCTION sklib_dll_beacon

#if defined(_MSC_VER) && defined (_UNICODE)
    #define SKLIB_INTERNAL_DLL_FILENAME_STRING_TYPE std::wstring
    #define SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE wchar_t
#else
    #define SKLIB_INTERNAL_DLL_FILENAME_STRING_TYPE std::string
    #define SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE char
#endif

namespace sklib {
namespace priv {

    // type of function call compatible with SKLib DLL
    typedef int(SKLIB_INTERNAL_DLL_CDECL* dll_generic_entry)(int);

    // OS-specific data related to DLL I/O; moved to separate compilation unit to prevent namespace pollution
    struct dll_internal_workspace_type;

    // DLL class infrastructure
    class sklib_internal_dll_interface_type;

    // DLL declaration order test infrastructure
    struct dll_test_base {};

    // DLL entry (function) descriptor, except the specifc calling details
    class dll_function_info
    {
        friend sklib::priv::sklib_internal_dll_interface_type;

    public:
        const std::string name;
        const bool is_required = false;
        unsigned int sys_load_error = 0;    // system error, if any, while loading the symbol from DLL

        dll_function_info(const std::string& fn, bool reqd) : name(fn), is_required(reqd) {}

    protected:
        sklib::priv::dll_generic_entry address = nullptr;
    };

    // common DLL interface functionality
    // elements open to dll_interface class, shielded by sklib_internal private variable
    struct dll_interface_helper
    {
        dll_interface_helper();  // default constructor/destructor cannot coexist with smart pointer to incomplete type
        ~dll_interface_helper();

        std::unique_ptr<sklib::priv::dll_internal_workspace_type> handle;
        std::vector<sklib::priv::dll_function_info*> func_list;

        // this object reflects DLL status; it is exposed to caller code by operator() of the DLL class
        struct
        {
            struct dll_status_code_type
            {
                static constexpr unsigned MASK_FATAL                = 0x0FFF;

                static constexpr unsigned OK                        = 0;
                static constexpr unsigned NOT_LOADED                = 0x0001;
                static constexpr unsigned LOAD_FAILURE              = 0x0002;
                static constexpr unsigned CALLING_MODE_MISMATCH     = 0x0004;
                static constexpr unsigned MISSING_REQUIRED_FUNCTION = 0x0100;
                static constexpr unsigned BAREFOOT_LIBRARY          = 0x1000;  // only may be set if mode check is requested
                static constexpr unsigned MISSING_OPTIONAL_FUNCTION = 0x2000;
            }
            dll_status_code;

            unsigned dll_load_status = dll_status_code_type::NOT_LOADED;
            unsigned sys_dll_error = 0;

            bool is_dll_usable() const { return !(dll_load_status & dll_status_code.MASK_FATAL); }
        }
        dll_info;

        // calling mode support

        struct calling_mode_type
        {
            typedef uint32_t(SKLIB_INTERNAL_DLL_CDECL* ptr_beacon_func)();

            static constexpr uint32_t NOTHING      = 0;
            static constexpr uint32_t DATA_SEGMENT = 0x0001;
        };

        static constexpr uint32_t get_calling_mode()
        {
            return 0
#ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT
                + calling_mode_type::DATA_SEGMENT
#endif
                ;  // future: add more ifdefs and mode designators as needed
        }

        static uint32_t get_remote_calling_mode(sklib::priv::dll_generic_entry address)
        {
            static_assert(sizeof(calling_mode_type::ptr_beacon_func) == sizeof(address), "SKLIB ** INTERNAL ERROR ** DLL Function Address Mismatch (Beacon)");
            return (address ? reinterpret_cast<calling_mode_type::ptr_beacon_func>(address)() : calling_mode_type::NOTHING);
        }

        // calls using specific OS API are moved to separate compilation unit to avoid namespace pollution
        // following functions are specifically for DLL I/O

        bool open_dll(const SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE* dll_name)
        { return this->open_dll_impl(dll_name, get_calling_mode(), SKLIB_INTERNAL_DLL_DO_RUNTIME_CHECK); }

        bool open_dll_impl(const SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE* dll_name, uint32_t caller_mode, bool request_mode_check);
        bool is_dll_open();
        void close_dll(bool update_load_status = true);  // <-- only closes DLL handle, don't clear function poinetrs, leaves alone Handle variable
        void get_address(const char* symbol, bool required, sklib::priv::dll_generic_entry& address, unsigned& sys_load_error);
    };

    // the base class for DLL interface must not contain any members other than prefixed by "sklib_internal", or overloaded operators
    class sklib_internal_dll_interface_type
    {
    protected:
        sklib::priv::dll_interface_helper sklib_internal;
        struct sklib_internal_dll_test_impl : protected sklib::priv::dll_test_base {};

    public:
        sklib_internal_dll_interface_type(const SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE* dll_name) { sklib_internal.open_dll(dll_name); }
        sklib_internal_dll_interface_type()  = default;
        ~sklib_internal_dll_interface_type() { sklib_internal.close_dll(); };

        // by convention, brackets are for loading library
        // call with nullptr or empty string is for unloading

        const auto& operator() (const SKLIB_INTERNAL_DLL_FILENAME_CHAR_TYPE* dll_name)
        {
            sklib_internal.close_dll();

            for (auto& elem : sklib_internal.func_list)
            {
                elem->sys_load_error = 0;
                elem->address = nullptr;
            }

            if (dll_name && *dll_name && sklib_internal.open_dll(dll_name))
            {
                for (auto& elem : sklib_internal.func_list)
                {
                    sklib_internal.get_address(elem->name.c_str(), elem->is_required, elem->address, elem->sys_load_error);
                }
            }

            return sklib_internal.dll_info;
        }

        // flavor of the above for std::w/string
        const auto& operator() (const SKLIB_INTERNAL_DLL_FILENAME_STRING_TYPE& dll_name) { return operator()(dll_name.c_str()); }

        // call with no parameter does nothing (returns status)
        const auto& operator() () const { return sklib_internal.dll_info; }

        // direct conversion to bool returns flag whether DLL is loaded and usable
        explicit operator bool() const { return sklib_internal.dll_info.is_dll_usable(); }

        // when the data segment is defined in the top-level (user) declaration, the members of the base class get replaced
        // otherwise, the dummy placeholders are used
        struct sklib_internal_dll_instance_data_segment_type {};
    };

    // representation of the DLL call
    // when sptype is irrelevant, void is used

    template<class, class> class dll_function_entry;

    template<class sptype, class rtype, class... targs>
    class dll_function_entry<sptype, typename rtype(targs...)> : public sklib::priv::dll_function_info
    {
    public:
        typedef rtype ret_type;

#ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT
        static_assert(std::is_pointer_v<sptype>, "SKLIB ** INTERNAL ERROR ** DLL Data Segment must be a pointer");
        typedef rtype(*ptr_func_type)(sptype, targs...);

    private:
        sptype data_segment = nullptr;

    public:
        dll_function_entry(sptype dsptr, sklib::priv::dll_interface_helper* root, const char* name, bool reqd = true)
            : dll_function_entry(root, name, reqd)
        {
            data_segment = dsptr;
        }
#else
        typedef rtype(*ptr_func_type)(targs...);
#endif

        dll_function_entry(sklib::priv::dll_interface_helper* root, const char* name, bool reqd = true) : dll_function_info(name, reqd)
        {
            root->func_list.push_back(this);
            if (root->is_dll_open()) root->get_address(name, reqd, address, sys_load_error);
        }

        rtype operator() (targs... args) const
        {
            if (!address)
            {
#ifdef SKLIB_DLL_ENABLE_THROW
                throw std::runtime_error("SKLIB DLL ** Attempt to call missing function");
#else
                return rtype();
#endif
            }

            static_assert(sizeof(ptr_func_type) == sizeof(address), "SKLIB ** INTERNAL ERROR ** DLL Function Address Mismatch (Call)");

#ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT
            return reinterpret_cast<ptr_func_type>(address)(data_segment, args...);
#else
            return reinterpret_cast<ptr_func_type>(address)(args...);
#endif
        }

        explicit operator bool() const { return address; }
        operator ptr_func_type() const { return address; }
    };

}; // priv
}; // sklib

//

#ifdef SKLIB_DLL_EXPORT

  // Calling mode runtime check

  #ifndef SKLIB_DLL_DISABLE_RUNTIME_CHECK
    #define SKLIB_INTERNAL_DLL_EXPORT_BEACON extern "C" SKLIB_INTERNAL_DLL_VISIBILITY    \
        constexpr uint32_t SKLIB_INTERNAL_DLL_CDECL SKLIB_INTERNAL_DLL_BEACON_FUNCTION() \
        { return sklib::priv::dll_interface_helper::get_calling_mode(); }
  #else
    #define SKLIB_INTERNAL_DLL_EXPORT_BEACON
    #ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT   // future: check for other calling modes that affect DLL exporting
      #ifdef _MSC_VER
        #pragma message("Warning: SKLIB DLL ** Skipping runtime compatibility check is only safe in barefoot mode")
      #else
        #warning(SKLIB DLL ** Skipping runtime compatibility check is only safe in barefoot mode)
      #endif
    #endif
  #endif

  // Data segment handling

  #ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT
    #define SKLIB_DLL_DATA_SEGMENT struct sklib_internal_dll_check {}; struct sklib_internal_dll_instance_data_segment_type
    #define SKLIB_INTERNAL_DLL_TRANSLATE_PARAMS(...) (sklib_internal_dll_instance_data_segment_type* INSTANCE_DATA, __VA_ARGS__)
    #define SKLIB_INTERNAL_DLL_CHECK_DATA_SEGMENT static_assert(sklib::is_complete<sklib_internal_dll_check>::value, \
        "SKLIB DLL ** Declaration of DATA SEGMENT must go BEFORE DLL functions declarations")
  #else
    #define SKLIB_INTERNAL_DLL_TRANSLATE_PARAMS(...) (__VA_ARGS__)
    #define SKLIB_DLL_DATA_SEGMENT static_assert(false, "SKLIB DLL ** May Not Use Data Segment in Data-Less Environment"); \
        struct sklib_internal_dll_instance_data_segment_type
    #define SKLIB_INTERNAL_DLL_CHECK_DATA_SEGMENT
  #endif

  // Calling entry

  #define SKLIB_DLL_FUNC(rtype,fname,...) SKLIB_INTERNAL_DLL_CHECK_DATA_SEGMENT; \
      extern "C" SKLIB_INTERNAL_DLL_VISIBILITY rtype SKLIB_INTERNAL_DLL_CDECL fname SKLIB_INTERNAL_DLL_TRANSLATE_PARAMS(__VA_ARGS__)

  // DLL structure

  #define SKLIB_DLL_INTERFACE(ClassName) SKLIB_INTERNAL_DLL_EXPORT_BEACON; struct sklib_internal_dll_check; extern "C++"
  #define SKLIB_DLL_FUNC_OPT(rtype,fname,...) SKLIB_DLL_FUNC(rtype,fname,__VA_ARGS__)
  #define SKLIB_DLL_CLASS_IMPLEMENT

#else // Not SKLIB_DLL_EXPORT

  #define SKLIB_DLL_INTERFACE(ClassName) struct ClassName : public sklib::priv::sklib_internal_dll_interface_type

  #ifdef SKLIB_DLL_ENABLE_INSTANCE_DATA_SEGMENT
    #define SKLIB_DLL_DATA_SEGMENT static_assert(std::is_base_of_v<sklib::priv::dll_test_base, sklib_internal_dll_test_impl>, \
        "SKLIB DLL ** Declaration of DATA SEGMENT must go BEFORE class implementation"); struct sklib_internal_dll_instance_data_segment_type
    #define SKLIB_DLL_CLASS_IMPLEMENT struct sklib_internal_dll_test_impl {}; \
        using sklib_internal_dll_interface_type::sklib_internal_dll_interface_type; \
        sklib_internal_dll_instance_data_segment_type sklib_internal_dll_instance_data_segment; \
        sklib_internal_dll_instance_data_segment_type* operator->() { return &sklib_internal_dll_instance_data_segment; } \
        sklib_internal_dll_instance_data_segment_type& operator*()  { return sklib_internal_dll_instance_data_segment; }

    #define SKLIB_INTERNAL_DLL_FUNC(rtype,reqd,fname,...) \
        static_assert(!std::is_base_of_v<sklib::priv::dll_test_base, sklib_internal_dll_test_impl>, \
            "SKLIB DLL ** Class implementation must be done BEFORE declaration of DLL ENTRY"); \
        sklib::priv::dll_function_entry<sklib_internal_dll_instance_data_segment_type*, rtype(__VA_ARGS__)> \
        fname{&sklib_internal_dll_instance_data_segment, &this->sklib_internal, #fname, reqd};
  #else
    #define SKLIB_DLL_DATA_SEGMENT \
        static_assert(false, "SKLIB DLL ** May Not Use Data Segment in Data-Less Environment"); \
        struct sklib_internal_dll_instance_data_segment_type
    #define SKLIB_DLL_CLASS_IMPLEMENT \
        using sklib_internal_dll_interface_type::sklib_internal_dll_interface_type;
    #define SKLIB_INTERNAL_DLL_FUNC(rtype,reqd,fname,...) \
        sklib::priv::dll_function_entry<void, rtype(__VA_ARGS__)> fname{&this->sklib_internal, #fname, reqd};
  #endif

  #define SKLIB_DLL_FUNC(rtype,fname,...) SKLIB_INTERNAL_DLL_FUNC(rtype,true,fname,__VA_ARGS__)
  #define SKLIB_DLL_FUNC_OPT(rtype,fname,...) SKLIB_INTERNAL_DLL_FUNC(rtype,false,fname,__VA_ARGS__)

#endif // SKLIB_DLL_EXPORT


