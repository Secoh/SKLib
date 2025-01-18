// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#ifndef SKLIB_INCLUDED_CMDPAR_HPP
#define SKLIB_INCLUDED_CMDPAR_HPP

#include <type_traits>

#include "string.hpp"
#include "utility.hpp"

// ------------------------------------------------------------------------------
// Option vs Parameter vs Argument
//   Parameter is required, option is optional
//   Definitions of classes and members in the list are Parameters
//   When something in context is optional, it is Option
//   Argument is string portion of command line to be interpreted by parser

// Shortcuts for Parameter Set declaration
//
// 1) Declaring set of parameters; input string type is optional
//
//      SKLIB_DECLARE_CMD_PARAMS(my_param_class_name[,character_type])
//      {
//          param
//          param
//          param
//      };
//      ...
//      int main(int argn, char *argc[])
//      {
//          my_param_class_name ParamsBlock [ constructor options ];
//          ParamsBlock(argn, argc);
//
// 2) Using parameters declarations
//
//    a) Required named parameters: int, int64, double, key (single character), string; example:
//          SKLIB_PARAM_INT(option_name [ , default_value ] );
//
//    b) Optional named parameter, except Switch - same as required
//          SKLIB_OPTION_INT(option_name, [ default_value ] );
//
//    c) Switch, optional - special parameter that shows only presence (no value)
//          SKLIB_OPTION_SWITCH(option_name);
//
//    d) Help option declaration
//          SKLIB_OPTION_HELP(option_name);
//
//    e) Required plain parameters - same as named, given without prefixed name
//          SKLIB_PLAIN_INT(option_name [ , default_value ] );
//
//    f) Optional plain parameters - similar as named
//          SKLIB_PLAIN_OPT_INT(option_name [ , default_value ] );
//

#ifdef SKLIB_CMDPAR_PARAM_PREFIX
#define SKLIB_INTERNAL_CMDPAR_USE_PARAM_PREFIX SKLIB_CMDPAR_PARAM_PREFIX
#else
#define SKLIB_INTERNAL_CMDPAR_USE_PARAM_PREFIX
#endif

// === Class Names/Template parameters in the main CMDPAR table namespace
// Must be made up such a way so accidental collision with command line parameter is unlikely

#define SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE               \
sklib_internal_2N5W3M28EFBZ8HSDY3O0HECRSWGXCB49_cmdpar_table_base

#define SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE_CONDUIT           \
sklib_internal_3YDD0M3079BQY31YBFPZXOH22H844Y3A_cmdpar_table_anchor

#define SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER                \
sklib_internal_R8CN6GNPALNTILL5YILVJJKURH7WSTTT_cmdpar_embedded_member

#define SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE                   \
sklib_internal_WF86I91MC30BCXJYBORWAWC1OVDDW386_cmdpar_letter_type

#define SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_ALTERNATIVE_PREFIX_SETTER     \
sklib_internal_H9V8GG33G51W7JJ93VO2SI2HQG288040_alternative_prefix_setter

// === Parameters collection is class template, declared by the header:
//

#define SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_TWO(name,type) \
    struct name : public sklib::priv::SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE<type>
#define SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_ONE(name) SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_TWO(name,char)
#define SKLIB_DECLARE_CMD_PARAMS(...) SKLIB_MACRO_SELECT_ONE_TWO(SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_ONE, \
                                                                 SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_TWO, __VA_ARGS__)

// === Parameters, Options declarations

#define SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,suffix,name,...) \
    sklib::priv::cmdpar_param_##suffix<SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE, std::decay_t<decltype(*name)>> \
    SKLIB_MACRO_SELF(SKLIB_INTERNAL_CMDPAR_USE_PARAM_PREFIX)##x \
    { sklib::priv::cmdpar_table_to_params_list_header<SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE>(this), name, __VA_ARGS__ };

#define SKLIB_OPTION_SWITCH(x)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,switch,#x)

#define SKLIB_PARAM_INT(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,#x,true,__VA_ARGS__)
#define SKLIB_PARAM_UINT(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,#x,true,__VA_ARGS__)
#define SKLIB_PARAM_I64(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,#x,true,__VA_ARGS__)
#define SKLIB_PARAM_DOUBLE(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,#x,true,__VA_ARGS__)
#define SKLIB_PARAM_KEY(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,#x,true,__VA_ARGS__)
#define SKLIB_PARAM_STRING(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,#x,true,__VA_ARGS__)

#define SKLIB_OPTION_INT(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,#x,false,__VA_ARGS__)
#define SKLIB_OPTION_UINT(x,...)    SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,#x,false,__VA_ARGS__)
#define SKLIB_OPTION_I64(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,#x,false,__VA_ARGS__)
#define SKLIB_OPTION_DOUBLE(x,...)  SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,#x,false,__VA_ARGS__)
#define SKLIB_OPTION_KEY(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,#x,false,__VA_ARGS__)
#define SKLIB_OPTION_STRING(x,...)  SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,#x,false,__VA_ARGS__)

#define SKLIB_OPTION_HELP(x)        SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,help,#x)

#define SKLIB_PLAIN_INT(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,"",true,__VA_ARGS__)
#define SKLIB_PLAIN_UINT(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,"",true,__VA_ARGS__)
#define SKLIB_PLAIN_I64(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,"",true,__VA_ARGS__)
#define SKLIB_PLAIN_DOUBLE(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,"",true,__VA_ARGS__)
#define SKLIB_PLAIN_KEY(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,"",true,__VA_ARGS__)
#define SKLIB_PLAIN_STRING(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,"",true,__VA_ARGS__)

#define SKLIB_PLAIN_INT_OPTIONAL(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,"",false,__VA_ARGS__)
#define SKLIB_PLAIN_UINT_OPTIONAL(x,...)     SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,"",false,__VA_ARGS__)
#define SKLIB_PLAIN_I64_OPTIONAL(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,"",false,__VA_ARGS__)
#define SKLIB_PLAIN_DOUBLE_OPTIONAL(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,"",false,__VA_ARGS__)
#define SKLIB_PLAIN_KEY_OPTIONAL(x,...)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,"",false,__VA_ARGS__)
#define SKLIB_PLAIN_STRING_OPTIONAL(x,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,"",false,__VA_ARGS__)

#define SKLIB_PARAM_INT_ALT_NAME(x,m,...)    SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,m,true,__VA_ARGS__)
#define SKLIB_PARAM_UINT_ALT_NAME(x,m,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,m,true,__VA_ARGS__)
#define SKLIB_PARAM_I64_ALT_NAME(x,m,...)    SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,m,true,__VA_ARGS__)
#define SKLIB_PARAM_DOUBLE_ALT_NAME(x,m,...) SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,m,true,__VA_ARGS__)
#define SKLIB_PARAM_KEY_ALT_NAME(x,m,...)    SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,m,true,__VA_ARGS__)
#define SKLIB_PARAM_STRING_ALT_NAME(x,m,...) SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,m,true,__VA_ARGS__)

#define SKLIB_OPTION_SWITCH_ALT_NAME(x,m)    SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,switch,m)

#define SKLIB_OPTION_INT_ALT_NAME(x,m,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int,m,false,__VA_ARGS__)
#define SKLIB_OPTION_UINT_ALT_NAME(x,m,...)  SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,uint,m,false,__VA_ARGS__)
#define SKLIB_OPTION_I64_ALT_NAME(x,m,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,int64,m,false,__VA_ARGS__)
#define SKLIB_OPTION_DOUBLE_ALT_NAME(x,m,...) SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,double,m,false,__VA_ARGS__)
#define SKLIB_OPTION_KEY_ALT_NAME(x,m,...)   SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,key,m,false,__VA_ARGS__)
#define SKLIB_OPTION_STRING_ALT_NAME(x,m,...) SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,str,m,false,__VA_ARGS__)

#define SKLIB_OPTION_HELP_ALT_NAME(x,m)      SKLIB_INTERNAL_CMDPAR_DECLARE_NAMED_TYPED_OPTION(x,help,m)

#define SKLIB_CMD_PARAMS_ALT_PREFIX(c) \
    sklib::priv::cmdpar_parser_prefix<SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE> \
    SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_ALTERNATIVE_PREFIX_SETTER \
    { sklib::priv::cmdpar_table_to_params_list_header<SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE>(this), c };

#define SKLIB_CMD_PARAMS_INITIALIZER(name) name() = default; \
    name(int argn, const SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE* const* argc, int arg_start = 1, bool do_reset = true) \
    { this->operator()(argn, argc, arg_start, do_reset); }

// === Parameters Collection and Parser classes

namespace sklib
{
    struct cmdpar_parser_flags
    {
        static constexpr uint16_t nothing      = 0;       // parser wasn't run
        static constexpr uint16_t good         = 0x0001;  // all checks satisfied, no errors, and not help request
        static constexpr uint16_t empty        = 0x0002;  // set if the input is empty; still may be valid state
        static constexpr uint16_t error        = 0x0004;  // error state; check clarification bit for specific problem
        static constexpr uint16_t error_unknown_name   = 0x0008;  // unrecognized parameter(s); also, unrecognized help request
        static constexpr uint16_t error_missing_named  = 0x0010;  // some (or all) required named parameters are not present
        static constexpr uint16_t error_missing_plain  = 0x0020;  // some (or all) required plain parameters are not present
        static constexpr uint16_t error_overflow_named = 0x0040;  // too many (repeating) named parameters (hint only in case of error)
        static constexpr uint16_t error_overflow_plain = 0x0080;  // too many plain parameters
        static constexpr uint16_t error_malformed      = 0x0100;  // some (or all) parameters are malformed (represented incorrectly) - check individual options
        static constexpr uint16_t help_request = 0x0200;  // help is requested; check specific bit(s) to see the request(s)
        static constexpr uint16_t help_banner  = 0x0400;  // general help is requested; -help without parameter
        static constexpr uint16_t help_option  = 0x0800;  // help on parameter is requested; may set parser_unknown bit if the parameter is unknown

        static constexpr uint16_t all_specific_errors = error_unknown_name | error_missing_named | error_missing_plain |
                                                        error_overflow_named | error_overflow_plain | error_malformed;
    };

    struct cmdpar_option_flags
    {
        static constexpr uint8_t nothing       = 0;     // option state is correct (even if not present)
        static constexpr uint8_t present       = 0x01;  // option is present
        static constexpr uint8_t error_empty   = 0x02;  // data is required for the option type, but data is not found
        static constexpr uint8_t error_partial = 0x04;  // in split options, data portion shall occupy the entire argument, but the read terminates before end
        static constexpr uint8_t help_request  = 0x20;  // hack: first help request sets this bit instead of Present bit to allow help on help (double) requests
        static constexpr uint8_t required      = 0x40;  // "required" bit is set (or not) at initialization
        static constexpr uint8_t is_help       = 0x80;  // hack: this bit signals that the option is used in help subsystem
    };

    namespace priv
    {
    // ====================================================================================================
    // === Parser Status Data and I/O
    //
    // The member access operator (dot) from a CMDPAR class shall provide only user-defined entries
    // representing the individual command-line options.
    // The data structure with CMDPAR class internal state is kept in separate place and is addressed
    // by pointer to member operator ("->").

        template<class> class cmdpar_param_base;
        template<class> class cmdpar_status_base;
        template<class> class cmdpar_status_type;
        template<class> struct cmdpar_parser_prefix;
        template<class> class SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE;
        template<class letter_type> cmdpar_status_base<letter_type>*
            cmdpar_table_to_params_list_header(SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE<letter_type>*);

#define SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(letter_type) \
    typedef sklib::cmdpar_option_flags oflags;                   \
    typedef sklib::cmdpar_parser_flags pflags;                   \
    typedef cmdpar_param_base<letter_type> param_base;           \
    typedef cmdpar_status_base<letter_type> status_base;

        template<class letter_type>
        class cmdpar_status_base    // this is part of main parameter table; defined here because
        {                           // portions of cmdpar_status_type MUST be visible from here
            friend cmdpar_param_base<letter_type>;
            friend cmdpar_parser_prefix<letter_type>;

        protected:
            cmdpar_param_base<letter_type>* param_list_entry = nullptr;  // pointer to the list of all options

            static constexpr letter_type def_prefix = static_cast<letter_type>('-');
            letter_type cur_prefix = def_prefix;     // can be modified by including service macro (cannot be const)
        };


        SKLIB_AUX_DECLARE_TYPE_CONDUIT(SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE_CONDUIT, SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE);

    // ====================================================================================================
    // === Parameters Descriptors - Base
    //
    // all parameters have functions:
    //   name()      option name (user input), string
    //   status()    result of reading this individual parameter
    //   present()   helper function to interpret status field; true if command line contains this option
    //   value()     the value associated with parameter (equals present() for switch)
    // hidden functions, virtual:
    //   is_match()  tests if input string starts with name of itself
    //   do_decode() reads and interprets the value from input string
    //   get_string_value()  for string and help options, returns value (string), for all others, returns empty
    //   do_reset_impl()     parameter type-specific to clear value from previous parser run (if any)

        template<class letter_type>  // input_letter_type
        class cmdpar_param_base
        {
            friend SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE<letter_type>;

        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(letter_type);

            const uint8_t option_required_v = oflags::nothing;
            uint8_t option_status = oflags::nothing;

            bool is_required() const       { return (option_status & oflags::required); }
            bool is_help() const           { return (option_status & oflags::is_help); }
            bool is_named_param() const    { return this->name_len; }

            const unsigned name_len = 0;
            cmdpar_param_base* const next_param = nullptr;

            static constexpr letter_type global_defval_key       = 0;                       // = '\0';
            static constexpr letter_type global_defval_zstring[] = { global_defval_key };   // = "";

            // read and accept the value of the parameter, except for switch
            // derived classes for other parameter types shall override this fuction
            virtual const letter_type* do_decode(const letter_type* arg) { return nullptr; }  // special case, no value for switch

            // (placeholder) type-erased strings compare - derived classes use comparisons specific to input data
            virtual bool is_match(const letter_type* arg) const { return false; }

            // (placeholder) support reading param_string value
            virtual const letter_type* get_string_value() const { return global_defval_zstring; }

            // reset the parameter state and value
            virtual void do_reset_impl() {} // specific cleanup for a typed parameter; no action for switch
            void do_reset()
            {
                option_status = option_required_v;
                do_reset_impl();
            }

        public:
            auto get_status() const     { return option_status; }
            bool is_present() const     { return (option_status & oflags::present); }
            bool help_requested() const { return (option_status & oflags::help_request); }

            explicit constexpr cmdpar_param_base(status_base* root,
                                                 unsigned param_name_len,
                                                 bool param_required)
                : name_len(param_name_len)
                , next_param(root->param_list_entry)
                , option_required_v(param_required ? oflags::required : oflags::nothing)
            {
                option_status = option_required_v;
                root->param_list_entry = this;
            }
        };

// ==========================

        // As member access operator (dot) is concerned, CMDPAR tables shall consist
        // only from user-defined member variables each representing a command line option.
        // Although it is impossible to make a class with behavior without private
        // member variables controlling its state, we can keep the count to the absolute
        // minimum, and also make a name likely to be unique.

        template<class letter_type = char>
        class SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE : public sklib::priv::SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_LETTER_TYPE_CONDUIT<letter_type>
        {
            static_assert(std::is_same_v<letter_type, std::decay_t<letter_type>>, "SKLIB ** Data type representing a character must have no qualifiers.");
            static_assert(sklib::is_integer_v<letter_type>, "SKLIB ** Data type representing a character must be integer.");

            friend cmdpar_status_base<letter_type>*
                cmdpar_table_to_params_list_header<letter_type>(SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE*);

        private:
            struct  // Unnamed struct for private Embedded object; addressing from outside is impossible
            {
                SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(letter_type);

                class cmdpar_status_type : public cmdpar_status_base<letter_type>
                {
                    friend SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE;

                private:
                    uint16_t status_code = pflags::nothing;

                public: // Small subset of the Embedded, accessible by outside
                    auto get_status() const { return status_code; }
                    bool is_good() const    { return (status_code & pflags::good); }
                    bool is_empty() const   { return (status_code & pflags::empty); }
                    bool has_error() const  { return (status_code & pflags::error); }
                    bool help_requested() const { return (status_code & pflags::help_request); }

                    void reset()
                    {
                        status_code = pflags::nothing;
                        for (param_base* ptr = this->param_list_entry; ptr; ptr = ptr->next_param) ptr->do_reset();
                    }
                }
                Status;

                // === Helper Functions, manipulating parameters, help formatting/printing

                void set_pflags(uint16_t what)   { Status.status_code |= what; }
                void clear_pflags(uint16_t what) { Status.status_code &= ~what; }

                // match the parameter name to opt argument (which is past prefix, and may be in the middle of original string)
                // function selects the longest match that wasn't already taken
                // if multiple matches are possible, selects the first in the declaration order
                //
                // the first entry must not be empty, checked by caller
                // the second entry is normally the next command line parameter, unless it is logically start of another option or if argument array ends prematurely
                // the second entry may be non-existent, in such case caller shall supply "" string; caller guarantees that argument is never nullptr
                //
                // if accepted, update state of the associated object, read value, set presense, errors, etc
                // return: 0 - parameter not read (string doesn't match option name), state of the object didn't change
                // <positive> - length in characters that is read from the input, parser still reads first string in pair (but may be did read to the end)
                // <negative> - detached parameter is seen: first string is read to the end, and the second string is the value, also taken
                //              if the second parameter cannot be parsed to the end, the rest of it is ignored and error state for this parameter is set
                int apply_named_param(const letter_type* opt, const letter_type* next_arg)
                {
                    param_base* select = nullptr;
                    bool seen_match = false;

                    for (param_base* ptr = Status.param_list_entry; ptr; ptr = ptr->next_param)
                    {
                        if (ptr->is_named_param() && (!select || ptr->name_len >= select->name_len) && ptr->is_match(opt))   // search for longest match
                        {
                            seen_match = true;
                            if (!ptr->is_present()) select = ptr;   // name match while cannot select it may indicate repeating command line option
                        }
                    }
                    if (!select)   // didn't find
                    {
                        if (seen_match) set_pflags(pflags::error_overflow_named);
                        return 0;
                    }

                    // Hack: handling "-help" option is different. Flag option.present is reserved for "-help -help" combination to allow both
                    // "-help -option" and "-help option" use. Presense of the help request is reflacted in option status in option_help_request bit.
                    // (Not only in global param_help_request to enable different "help" options having different actions.)
                    select->option_status |= ((select->is_help() && !select->help_requested()) ? oflags::help_request : oflags::present);

                    unsigned len = select->name_len;
                    bool same_argument = (sklib::strlen<unsigned>(opt) > len);
                    auto value_start = (same_argument ? opt + len : next_arg);
                    auto value_end = select->do_decode(value_start);

                    if (!value_end) return len;    // it was switch

                    if (same_argument)
                    {
                        if (value_start == value_end) select->option_status |= oflags::error_empty;   // if value is not read
                        return (int)(value_end - opt);
                    }

                    // only the body of the value must be in the string, otherwise it is error
                    if (value_end - value_start != sklib::strlen<unsigned>(next_arg)) select->option_status |= oflags::error_partial;
                    return -1;
                }

                // enter unnamed (plain) parameter; string shall be parsed as the entire body
                // parameters are filled up in the declaration order
                // returns true if something is taken, false if the fist is full
                bool apply_plain_param(const letter_type* opt)
                {
                    param_base* select = nullptr;
                    for (param_base* ptr = Status.param_list_entry; ptr; ptr = ptr->next_param)
                    {
                        if (!ptr->is_named_param() && !ptr->is_present()) select = ptr;
                    }
                    if (!select) return false;   // didn't find

                    select->option_status |= oflags::present;

                    auto value_end = select->do_decode(opt);
                    if (!value_end || value_end - opt != sklib::strlen<unsigned>(opt)) select->option_status |= oflags::error_partial;
                    // only the body of the value must be in the string, otherwise it is error

                    return true;
                }

                // ====================================================================================================
                // === Main Parser Entry
                //
                // parse the command line arguments into the parameter set (defined in the class)
                // - named options, can be represented in 2 ways:
                //   1) -a##b##... all switches, keys and numbers in the same string without separation (only logical separation)
                //   2) -a ### - option in one command line argument, its value (assuming the option has a value) in the next argument
                //      (detached value). Note that next "-", but not "--" breaks option-value association.
                // - plain (not named) options, normally taken from command line arguments without prefix
                // - double prefix anywhere cancels special treatment of the next command line argument (when it starts with prefix)
                // - single prefix (alone or part of an option) breaks interpretation of the previous parameter if not finished

                bool run_parser(int argn, const letter_type* const* argc, int arg_start, bool do_reset)
                {
                    if (do_reset) Status.reset();

                    // 1. decode the input

                    bool signal_plain = false;  // if true, next arument is interpreted as plain parameter ("GNU style")

                    for (int k = arg_start; k < argn; k++)
                    {
                        auto arg_cur = argc[k];
                        if (!arg_cur || !*arg_cur) continue;  // this cannot happen

                        if (*arg_cur == Status.cur_prefix && !signal_plain)
                        {
                            unsigned arg_len = sklib::strlen<unsigned>(arg_cur);
                            for (unsigned t = 1; t < arg_len; t++)
                            {
                                if (arg_cur[t] == Status.cur_prefix)
                                {
                                    signal_plain = true;    // second prefix supresses next parameter: -- -string in command line => "-string" becomes plain parameter
                                    continue;               // to take effect, the secondary prefix shall be the last in the line
                                }
                                signal_plain = false;

                                // examine placement of prefix and arguments
                                // 0) if parameter value appears in the same argument with parameter name: -d6 => d is name, 6 is value
                                // 1) normal way to give parameter: -d 6 => when string ends after "-d", "6" is expected in the next argument
                                // 2) value is empty/skipped, if next argument is another parameter: -d -x => parameter d has empty value
                                // 3) value starting with "-" (prefix) can be forced by double-prefix: -d -- -x => "-x" becomes value of d
                                // 4) for completeness, tested in other place, use double-prefix outside parameter context to force next argument to be plain parameter: -- -d => "-d" is entered as plain parameter

                                auto kval = k + 1;
                                auto arg_val = (kval < argn ? argc[kval] : nullptr);

                                if (!arg_val)
                                {
                                    arg_val = param_base::global_defval_zstring;   // empty
                                }
                                else if (arg_val[0] != Status.cur_prefix)
                                {
                                    ;   // keep current arg_val, even if empty
                                }
                                else if (arg_val[1] != Status.cur_prefix)    // before check, arg_val = "-..."
                                {
                                    arg_val = param_base::global_defval_zstring;   // next element is parameter, no value
                                }
                                else if (!arg_val[2])    // arg_val = "--"
                                {
                                    arg_val = (++kval < argn ? argc[kval] : param_base::global_defval_zstring);    // use second extension as value, if present
                                }
                                else    // arg_val = "--something"
                                {
                                    arg_val = param_base::global_defval_zstring;   // for completeness, one shall not use that
                                }

                                auto ds = apply_named_param(arg_cur + t, arg_val);
                                if (ds > 0)
                                {
                                    t += ds - 1;
                                }
                                else if (ds < 0)
                                {
                                    t = arg_len - 1;
                                    if (*arg_val) k = kval;   // if there is a command line entry(ies) that we need to skip // if empty, we DON'T need to skip the next argument
                                }
                                else // ds == 0
                                {
                                    set_pflags(pflags::error_unknown_name);
                                }
                            }
                        }
                        else
                        {
                            if (!apply_plain_param(arg_cur)) set_pflags(pflags::error_overflow_plain);
                            signal_plain = false;
                        }
                    }

                    // 2. collect flags and conditions

                    set_pflags(pflags::empty);
                    for (param_base* ptr = Status.param_list_entry; ptr; ptr = ptr->next_param)
                    {
                        if (ptr->is_present())
                        {
                            clear_pflags(pflags::empty);
                        }
                        else if (ptr->is_required())
                        {
                            set_pflags(ptr->is_named_param() ? pflags::error_missing_named : pflags::error_missing_plain);
                        }

                        if (ptr->help_requested())
                        {
                            clear_pflags(pflags::empty);
                            set_pflags(pflags::help_request);
                        }

                        if (ptr->get_status() & (oflags::error_empty | oflags::error_partial))
                        {
                            set_pflags(pflags::error_malformed);
                        }
                    }

                    if (Status.status_code & pflags::all_specific_errors) set_pflags(pflags::error);

                    // lets check if -help has parameter; then it is different whether help request is about specific parameter
                    if (Status.help_requested())
                    {
                        for (const param_base* ptr = Status.param_list_entry; ptr; ptr = ptr->next_param)
                        {
                            if (ptr->help_requested())
                            {
                                const letter_type* argument = ptr->get_string_value();
                                unsigned argument_length = sklib::strlen<unsigned>(argument);
                                if (!argument_length) continue;

                                bool help_argument_taken = false;
                                for (param_base* chk = Status.param_list_entry; chk; chk = chk->next_param)
                                {
                                    if (chk->name_len == argument_length && chk->is_match(argument))
                                    {
                                        help_argument_taken = true;
                                        chk->option_status |= oflags::present;
                                    }
                                }

                                if (!help_argument_taken) set_pflags(pflags::error_unknown_name | pflags::error);
                            }
                        }

                        bool help_argument_seen = false;
                        for (const param_base* ptr = Status.param_list_entry; ptr; ptr = ptr->next_param)
                        {
                            if (ptr->is_present() && ptr->is_named_param()) help_argument_seen = true;
                        }

                        set_pflags(help_argument_seen ? pflags::help_option : pflags::help_banner);
                    }
                    else if (!Status.has_error())
                    {
                        set_pflags(pflags::good);  // not help and not error - for completeness
                    }

                    return (Status.is_good() || Status.help_requested());
                }
            }
            SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER;

        public:
            auto* operator-> ()
            { return &SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER.Status; }

            const auto* operator-> () const
            { return &SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER.Status; }

            bool operator() (int argn, const letter_type* const* argc, int arg_start = 1, bool do_reset = true)
            { return SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER.run_parser(argn, argc, arg_start, do_reset); }
        };

        template<class letter_type>
        cmdpar_status_base<letter_type>*
        cmdpar_table_to_params_list_header(SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_TABLE_BASE_TYPE<letter_type>* what)
        {
            return &(what->SKLIB_INTERNAL_CMDPAR_UNIQUE_NAME_EMBEDDED_LAYER.Status);
        }


        // ====================================================================================================
        // === Parameters Descriptors - Details

        template<class input_letter_type, class name_letter_type>
        class cmdpar_param_base_named : public cmdpar_param_base<input_letter_type>
        {
        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);

            const name_letter_type* const name_store = nullptr;

            bool is_match(const input_letter_type* arg) const
            {
                return sklib::strnequ(arg, name_store, this->name_len);
            }

        public:
            auto name() const { return name_store; }

            explicit constexpr cmdpar_param_base_named(status_base* root,
                                                       const name_letter_type* param_name,
                                                       bool param_required)
                : name_store(param_name)
                , cmdpar_param_base<input_letter_type>(root, sklib::strlen<unsigned>(param_name), param_required)
            {}
        };

        template<class input_letter_type, class name_letter_type>
        class cmdpar_param_switch : public cmdpar_param_base_named<input_letter_type, name_letter_type>
        {
        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);

        public:
            auto value() const    { return this->is_present(); }
            operator auto() const { return this->is_present(); }

            explicit constexpr cmdpar_param_switch(status_base* root,
                                                   const name_letter_type* param_name)
                : cmdpar_param_base_named<input_letter_type, name_letter_type>(root, param_name, false)
            {}
        };

#define SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(suffix,parser,type)      \
        template<class input_letter_type, class name_letter_type>                   \
        class cmdpar_param_##suffix : public cmdpar_param_base_named<input_letter_type, name_letter_type> \
        {                                                                           \
        protected:                                                                  \
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);         \
                                                                                    \
            const type value_initial = type();                                      \
            type value_store = type();                                              \
                                                                                    \
            const input_letter_type* do_decode(const input_letter_type* arg)        \
            {                                                                       \
                unsigned pstop = 0;                                                 \
                value_store = sklib::parser<type>(arg, &pstop);                     \
                if (!pstop) this->option_status |= oflags::error_empty;             \
                return arg + pstop;                                                 \
            }                                                                       \
                                                                                    \
            void do_reset_impl()  { value_store = value_initial; }                  \
                                                                                    \
        public:                                                                     \
            auto value() const    { return value_store; }                           \
            operator auto() const { return value_store; }                           \
                                                                                    \
            explicit constexpr cmdpar_param_##suffix(status_base* root,             \
                                              const name_letter_type* param_name,   \
                                              bool param_required = false,          \
                                              type defval = type())                 \
                : value_initial(defval)                                             \
                , value_store(defval)                                               \
                , cmdpar_param_base_named<input_letter_type, name_letter_type>(root, param_name, param_required) \
            {}                                                                      \
        }
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(int, stoi, int);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(uint, stoi, unsigned);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(int64, stoi, int64_t);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(double, stod, double);

        template<class input_letter_type, class name_letter_type>
        class cmdpar_param_key : public cmdpar_param_base_named<input_letter_type, name_letter_type>
        {
        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);

            const input_letter_type value_initial = this->global_defval_key;
            input_letter_type value_store = this->global_defval_key;

            const input_letter_type* do_decode(const input_letter_type* arg)
            {
                value_store = *arg;
                if (value_store) return arg + 1;
                this->option_status |= oflags::error_empty;
                return arg;
            }

            void do_reset_impl()  { value_store = value_initial; }

        public:
            auto value()    const { return value_store; }
            operator auto() const { return value_store; }

            explicit constexpr cmdpar_param_key(status_base* root,
                                         const name_letter_type* param_name,
                                         bool param_required = false,
                                         input_letter_type defval = param_base::global_defval_key)
                : value_initial(defval)
                , value_store(defval)
                , cmdpar_param_base_named<input_letter_type, name_letter_type>(root, param_name, param_required)
            {}
        };

        template<class input_letter_type, class name_letter_type>
        class cmdpar_param_str : public cmdpar_param_base_named<input_letter_type, name_letter_type>
        {
        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);

            const input_letter_type* const value_initial = param_base::global_defval_zstring;
            const input_letter_type* value_store = param_base::global_defval_zstring;

            const input_letter_type* do_decode(const input_letter_type* arg)
            {
                value_store = arg;
                return arg + sklib::strlen<unsigned>(arg);
            }

            const input_letter_type* get_string_value() const { return value_store; }
            void do_reset_impl()  { value_store = value_initial; }

        public:
            auto value() const    { return value_store; }
            operator auto() const { return value_store; }

            explicit constexpr cmdpar_param_str(status_base* root,
                                         const name_letter_type* param_name,
                                         bool param_required = false,
                                         const input_letter_type* defval = param_base::global_defval_zstring)
                : value_initial(defval)
                , value_store(defval)
                , cmdpar_param_base_named<input_letter_type, name_letter_type>(root, param_name, param_required)
            {}
        };

        template<class input_letter_type, class name_letter_type>
        class cmdpar_param_help : public cmdpar_param_str<input_letter_type, name_letter_type>
        {
        protected:
            SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES(input_letter_type);

            void do_reset_impl() { this->option_status |= oflags::is_help; }

        public:
            explicit constexpr cmdpar_param_help(status_base* root,
                                                 const name_letter_type* param_name)
                : cmdpar_param_str<input_letter_type, name_letter_type>(root, param_name)
            { this->option_status |= oflags::is_help; }
        };

    // === Alternative Settings

        template<class letter_type>
        struct cmdpar_parser_prefix
        {
            cmdpar_parser_prefix(cmdpar_status_base<letter_type>* root, letter_type custom_prefix)
            { root->cur_prefix = custom_prefix; }
        };
    };
};

// Cleanup

#undef SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER
#undef SKLIB_INTERNAL_CMDPAR_DECLARE_CONTROL_TYPES

#endif // SKLIB_INCLUDED_CMDPAR_HPP

