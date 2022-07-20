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

#ifndef SKLIB_INCLUDED_CMDPAR_HPP
#define SKLIB_INCLUDED_CMDPAR_HPP

#include<type_traits>

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
// 1) Declaring set of parameters
//
//      DECLARE_CMD_PARAMS(my_param_class_name[,character_type])
//      {
//          param
//          param
//          param
//      };
//      ...
//      int main(int argn, char *argc[])
//      {
//          my_param_class_name ParamsBlock [ constructor options ];
//          ParamsBlock.parse(argn, argc);
//
// 2) Using parameters declarations
//
//    a) Required named parameters: int, int64, double, key (single character), string
//          PARAM_INT(option_name [ , default_value ] );
//
//    b) Optional named parameter, except Switch - same as required
//          OPTION_INT(option_name, [ default_value ] );
//
//    c) Switch, optional - special parameter that shows only presence (no value)
//          OPTION_SWITCH(option_name);
//
//    d) Help option declaration
//          OPTION_HELP(option_name);
//
//    e) Required plain parameters - same as named, given without prefixed name
//          PLAIN_INT(option_name [ , default_value ] );
//
//    f) Optional plain parameters - similar as named
//          PLAIN_OPT_INT(option_name [ , default_value ] );
//

// === Parameters collection is class template, declared by the header:
//
//   #define SKLIB_DECLARE_CMD_PARAMS(name)       struct name : public ::sklib::cmdpar_table_base_type<char>
//   #define SKLIB_DECLARE_CMD_PARAMS(name,type)  struct name : public ::sklib::cmdpar_table_base_type<type>
//
#define SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_ONE(name)      struct name : public ::sklib::cmdpar_table_base_type<char>
#define SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_TWO(name,type) struct name : public ::sklib::cmdpar_table_base_type<type>
#define SKLIB_DECLARE_CMD_PARAMS(...) SKLIB_MACRO_SELECT_ONE_TWO(SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_ONE, SKLIB_INTERNAL_CMDPAR_DECLARE_CMD_PARAMS_TWO, __VA_ARGS__)

// === Parameters, Options declarations

#define SKLIB_PARAM_INT(x,...)      param_int<char>    x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_UINT(x,...)     param_uint<char>   x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_I64(x,...)      param_int64<char>  x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_DOUBLE(x,...)   param_double<char> x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_KEY(x,...)      param_key<char>    x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_STRING(x,...)   param_str<char>    x{ this, #x , true, __VA_ARGS__ };

#define SKLIB_OPTION_SWITCH(x)      param_switch<char> x{ this, #x };

#define SKLIB_OPTION_INT(x,...)     param_int<char>    x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_UINT(x,...)    param_uint<char>   x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_INT64(x,...)   param_int64<char>  x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_DOUBLE(x,...)  param_double<char> x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_KEY(x,...)     param_key<char>    x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_STRING(x,...)  param_str<char>    x{ this, #x , false, __VA_ARGS__ };

#define SKLIB_OPTION_HELP(x,...)    param_help<char>   x{ this, #x , __VA_ARGS__ };

#define SKLIB_PLAIN_INT(x,...)      param_int<char>    x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_INT64(x,...)    param_int64<char>  x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_DOUBLE(x,...)   param_double<char> x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_KEY(x,...)      param_key<char>    x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_STRING(x,...)   param_str<char>    x{ this, "" , true, __VA_ARGS__ };

#define SKLIB_PLAIN_OPTION_INT(x,...)      param_int<char>    x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPTION_INT64(x,...)    param_int64<char>  x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPTION_DOUBLE(x,...)   param_double<char> x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPTION_KEY(x,...)      param_key<char>    x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPTION_STRING(x,...)   param_str<char>    x{ this, "" , false, __VA_ARGS__ };

#define SKLIB_PARAM_INT_NAME(x,m,...)      param_int<std::decay_t<decltype(*m)>>    x{ this, m , true, __VA_ARGS__ };
#define SKLIB_PARAM_UINT_NAME(x,m,...)     param_uint<std::decay_t<decltype(*m)>>   x{ this, m , true, __VA_ARGS__ };
#define SKLIB_PARAM_I64_NAME(x,m,...)      param_int64<std::decay_t<decltype(*m)>>  x{ this, m , true, __VA_ARGS__ };
#define SKLIB_PARAM_DOUBLE_NAME(x,m,...)   param_double<std::decay_t<decltype(*m)>> x{ this, m , true, __VA_ARGS__ };
#define SKLIB_PARAM_KEY_NAME(x,m,...)      param_key<std::decay_t<decltype(*m)>>    x{ this, m , true, __VA_ARGS__ };
#define SKLIB_PARAM_STRING_NAME(x,m,...)   param_str<std::decay_t<decltype(*m)>>    x{ this, m , true, __VA_ARGS__ };

#define SKLIB_OPTION_SWITCH_NAME(x,m)      param_switch<std::decay_t<decltype(*m)>> x{ this, m };

#define SKLIB_OPTION_INT_NAME(x,m,...)     param_int<std::decay_t<decltype(*m)>>    x{ this, m , false, __VA_ARGS__ };
#define SKLIB_OPTION_UINT_NAME(x,m,...)    param_uint<std::decay_t<decltype(*m)>>   x{ this, m , false, __VA_ARGS__ };
#define SKLIB_OPTION_INT64_NAME(x,m,...)   param_int64<std::decay_t<decltype(*m)>>  x{ this, m , false, __VA_ARGS__ };
#define SKLIB_OPTION_DOUBLE_NAME(x,m,...)  param_double<std::decay_t<decltype(*m)>> x{ this, m , false, __VA_ARGS__ };
#define SKLIB_OPTION_KEY_NAME(x,m,...)     param_key<std::decay_t<decltype(*m)>>    x{ this, m , false, __VA_ARGS__ };
#define SKLIB_OPTION_STRING_NAME(x,m,...)  param_str<std::decay_t<decltype(*m)>>    x{ this, m , false, __VA_ARGS__ };

#define SKLIB_OPTION_HELP_NAME(x,m,...)    param_help<std::decay_t<decltype(*m)>>   x{ this, m , __VA_ARGS__ };

#define SKLIB_PARAMS_ALT_PREFIX(c)  parser_prefix alternative_prefix{ this, c };


// === Parameters Collection and Parser classes

namespace sklib
{
    template<class T = char>
    class cmdpar_table_base_type
    {
    public:
        class param_base;

        typedef std::decay_t<T> letter_type;
        static_assert(is_integer_val<letter_type>, "SKLIB ** Data type representing a character must be integer.");

    // === Parser Defaults, Settings, Configuration

    protected:
        static constexpr letter_type def_prefix = static_cast<letter_type>('-');

        letter_type cur_prefix = def_prefix;     // can be modified by including service macro (cannot be const)
        param_base* param_list_entry = nullptr;  // list of all options

    // === Parser Status and Status Flags

    public:
        class
        {
            friend cmdpar_table_base_type;

        public:
            struct flags
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
            }
            flag;

        protected:
            uint16_t status = flags::nothing;

            void rst()              { status = flags::nothing; }
            void set(uint16_t what) { status |= what; }
            void clr(uint16_t what) { status &= ~what; }

        public:
            auto get() const      { return status; }
            bool is_good() const  { return (status & flags::good); }
            bool is_error() const { return (status & flags::error); }
            bool is_help() const  { return (status & flags::help_request); }
        }
        parser_status;

    // ====================================================================================================
    // === Parameters Descriptors
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

        class param_base
        {
            friend cmdpar_table_base_type;

        public:
            struct option_flag
            {
                static constexpr uint8_t nothing       = 0;     // option state is correct (even if not present)
                static constexpr uint8_t present       = 0x01;  // option is present
                static constexpr uint8_t error_empty   = 0x02;  // data is required for the option type, but data is not found
                static constexpr uint8_t error_partial = 0x04;  // in split options, data portion shall occupy the entire argument, but the read terminates before end
                static constexpr uint8_t help_request  = 0x20;  // hack: first help request sets this bit instead of Present bit to allow help on help (double) requests
                static constexpr uint8_t required      = 0x40;  // "required" bit is set (or not) at initialization
                static constexpr uint8_t is_help       = 0x80;  // hack: this bit signals that the option is used in help subsystem
            };

        protected:
            uint8_t option_status = option_flag::nothing;

            bool is_required() const       { return (option_status & option_flag::required); }
            bool is_help() const           { return (option_status & option_flag::is_help); }
            bool seen_help_request() const { return (option_status & option_flag::help_request); }
            bool is_named_param() const    { return this->name_len; }

            const unsigned name_len = 0;
            param_base* const next_param = nullptr;

            static constexpr letter_type global_defval_key       = 0;                       // = '\0';
            static constexpr letter_type global_defval_zstring[] = { global_defval_key };   // = "";

            // read and accept the value of the parameter, escept for switch
            // derived classes for other parameter types shall override this fuction
            virtual const letter_type* do_decode(const letter_type* arg) { return nullptr; }  // special case, no value for switch

            // (placeholder) type-erased strings compare - derived classes use comparisons specific to input data
            virtual bool is_match(const letter_type* arg) const { return false; }

            // (placeholder) support reading param_string value
            virtual const letter_type* get_string_value() const { return global_defval_zstring; }

        public:
            auto status() const  { return option_status; }
            bool present() const { return (status() & option_flag::present); }

            explicit constexpr param_base(cmdpar_table_base_type* root,
                                          unsigned param_name_len,
                                          bool param_required)
                : name_len(param_name_len)
                , next_param(root->param_list_entry)
            {
                if (param_required) option_status |= option_flag::required;
                root->param_list_entry = this;
            }
        };

    protected:
        template<class name_letter_type>
        class param_base_named : public param_base
        {
        protected:
            const name_letter_type* const name_store = nullptr;

            bool is_match(const letter_type* arg) const
            {
                return strnequ(arg, name_store, this->name_len);
            }

        public:
            auto name() const { return name_store; }

            explicit constexpr param_base_named(cmdpar_table_base_type* root,
                                       const name_letter_type* param_name,
                                       bool param_required)
                : name_store(param_name)
                , param_base(root, strlen<unsigned>(param_name), param_required)
            {}
        };

    public:
        template<class name_letter_type>
        class param_switch : public param_base_named<name_letter_type>
        {
        public:
            auto value() const    { return this->present(); }
            operator auto() const { return this->present(); }

            explicit constexpr param_switch(cmdpar_table_base_type* root,
                                            const name_letter_type* param_name)
                : param_base_named<name_letter_type>(root, param_name, false)
            {}
        };

#ifdef SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER
#error SKLIB ** INTERNAL ERROR ** Macro SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER is declared elsewhere. Unable to continue.
#endif
#define SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(suffix,parser,type)      \
        template<class name_letter_type>                                            \
        class param_##suffix : public param_base_named<name_letter_type>            \
        {                                                                           \
        protected:                                                                  \
            type value_store = type();                                              \
                                                                                    \
            const letter_type* do_decode(const letter_type* arg)                    \
            {                                                                       \
                const auto error_empty = param_base::option_flag::error_empty;      \
                unsigned pstop = 0;                                                 \
                value_store = parser<type>(arg, &pstop);                            \
                if (!pstop) this->option_status |= error_empty;                     \
                return arg + pstop;                                                 \
            }                                                                       \
                                                                                    \
        public:                                                                     \
            auto value() const    { return value_store; }                           \
            operator auto() const { return value_store; }                           \
                                                                                    \
            explicit constexpr param_##suffix(cmdpar_table_base_type* root,         \
                                              const name_letter_type* param_name,   \
                                              bool param_required = false,          \
                                              type defval = type())                 \
                : value_store(defval)                                               \
                , param_base_named<name_letter_type>(root, param_name, param_required) \
            {}                                                                      \
        }
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(int, stoi, int);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(uint, stoi, unsigned);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(int64, stoi, int64_t);
        SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER(double, stod, double);
#undef SKLIB_INTERNAL_CMDPAR_DECLARE_NUMERIC_PARAM_HOLDER

        template<class name_letter_type>
        class param_key : public param_base_named<name_letter_type>
        {
        protected:
            letter_type value_store = param_base::global_defval_key;

            const letter_type* do_decode(const letter_type* arg)
            {
                value_store = *arg;
                if (value_store) return arg + 1;
                this->option_status |= param_base::option_flag::error_empty;
                return arg;
            }

        public:
            auto value()    const { return value_store; }
            operator auto() const { return value_store; }

            explicit constexpr param_key(cmdpar_table_base_type* root,
                                         const name_letter_type* param_name,
                                         bool param_required = false,
                                         letter_type defval = param_base::global_defval_key)
                : value_store(defval)
                , param_base_named<name_letter_type>(root, param_name, param_required)
            {}
        };

        template<class name_letter_type>
        class param_str : public param_base_named<name_letter_type>
        {
        protected:
            const letter_type* value_store = param_base::global_defval_zstring;

            const letter_type* do_decode(const letter_type* arg)
            {
                value_store = arg;
                return arg + strlen<unsigned>(arg);
            }

            const letter_type* get_string_value() const // access from param_base
            {
                return value_store;
            }

        public:
            auto value() const    { return value_store; }
            operator auto() const { return value_store; }

            explicit constexpr param_str(cmdpar_table_base_type* root,
                                         const name_letter_type* param_name,
                                         bool param_required = false,
                                         const letter_type* defval = param_base::global_defval_zstring)
                : value_store(defval)
                , param_base_named<name_letter_type>(root, param_name, param_required)
            {}
        };

        template<class name_letter_type>
        class param_help : public param_str<name_letter_type>
        {
        public:
            explicit constexpr param_help(cmdpar_table_base_type* root,
                                          const name_letter_type* param_name)
                : param_str<name_letter_type>(root, param_name)
            {
                this->option_status |= param_base::option_flag::is_help;
            }
        };

    // === Alternative Settings

        struct parser_prefix
        {
            parser_prefix(cmdpar_table_base_type* root, letter_type custom_prefix)
            { root->cur_prefix = custom_prefix; }
        };

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

        bool parser_run(int argn, const letter_type* const* argc, int arg_start = 1)
        {
            typedef param_base::option_flag option_flag;
            typedef typename decltype(parser_status)::flags parser_flags;

            parser_status.rst();

            // 1. decode the input

            bool signal_plain = false;  // if true, next arument is interpreted as plain parameter ("GNU style")

            for (int k = arg_start; k < argn; k++)
            {
                auto arg_cur = argc[k];
                if (!arg_cur || !*arg_cur) continue;  // this cannot happen

                if (*arg_cur == cur_prefix && !signal_plain)
                {
                    unsigned arg_len = strlen<unsigned>(arg_cur);
                    for (unsigned t = 1; t < arg_len; t++)
                    {
                        if (arg_cur[t] == cur_prefix)
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
                            arg_val = "";   // empty
                        }
                        else if (arg_val[0] != cur_prefix)
                        {
                            ;   // keep current arg_val, even if empty
                        }
                        else if (arg_val[1] != cur_prefix)    // before check, arg_val = "-..."
                        {
                            arg_val = "";   // next element is parameter, no value
                        }
                        else if (!arg_val[2])    // arg_val = "--"
                        {
                            arg_val = (++kval < argn ? argc[kval] : "");    // use second extension as value, if present
                        }
                        else    // arg_val = "--something"
                        {
                            arg_val = "";   // for completeness, one shall not use that
                        }

                        auto ds = this->apply_named_param(arg_cur + t, arg_val);
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
                            parser_status.set(parser_flags::error_unknown_name);
                        }
                    }
                }
                else
                {
                    if (!this->apply_plain_param(arg_cur)) parser_status.set(parser_flags::error_overflow_plain);
                    signal_plain = false;
                }
            }

            // 2. collect flags and conditions

            parser_status.set(parser_flags::empty);
            for (param_base* ptr = param_list_entry; ptr; ptr = ptr->next_param)
            {
                if (ptr->present())
                {
                    parser_status.clr(parser_flags::empty);
                }
                else if (ptr->is_required())
                {
                    parser_status.set(ptr->is_named_param() ? parser_flags::error_missing_named : parser_flags::error_missing_plain);
                }

                if (ptr->seen_help_request())
                {
                    parser_status.clr(parser_flags::empty);
                    parser_status.set(parser_flags::help_request);
                }

                if (ptr->status() & (option_flag::error_empty | option_flag::error_partial))
                {
                    parser_status.set(parser_flags::error_malformed);
                }
            }

            const uint16_t all_parser_errors =
                (parser_flags::error_unknown_name | parser_flags::error_missing_named | parser_flags::error_missing_plain |
                 parser_flags::error_overflow_named | parser_flags::error_overflow_plain | parser_flags::error_malformed);

            if (parser_status.get() & all_parser_errors) parser_status.set(parser_flags::error);

            // lets check if -help has parameter; then it is different whether help request is about specific parameter
            if (parser_status.is_help())
            {
                for (const param_base* ptr = param_list_entry; ptr; ptr = ptr->next_param)
                {
                    if (ptr->seen_help_request())
                    {
                        const letter_type* argument = ptr->get_string_value();
                        unsigned argument_length = strlen<unsigned>(argument);
                        if (!argument_length) continue;

                        bool help_argument_taken = false;
                        for (param_base* chk = param_list_entry; chk; chk = chk->next_param)
                        {
                            if (chk->name_len == argument_length && chk->is_match(argument))
                            {
                                help_argument_taken = true;
                                chk->option_status |= option_flag::present;
                            }
                        }

                        if (!help_argument_taken) parser_status.set(parser_flags::error_unknown_name);
                    }
                }

                bool help_argument_seen = false;
                for (param_base* ptr = param_list_entry; ptr; ptr = ptr->next_param)
                {
                    if (ptr->present() && ptr->is_named_param()) help_argument_seen = true;
                }

                parser_status.set(help_argument_seen ? parser_flags::help_option : parser_flags::help_banner);
            }
            else if (!parser_status.is_error())
            {
                parser_status.set(parser_flags::good);  // not help and not error - for completeness
            }

            return (parser_status.is_good() || parser_status.is_help());
        }

    // === Helper Functions, manipulating parameters, help formatting/printing

    protected:
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
            typedef param_base::option_flag option_flag;

            param_base* select = nullptr;
            bool seen_match = false;
            for (param_base* ptr = param_list_entry; ptr; ptr = ptr->next_param)
            {
                if (ptr->is_named_param() && (!select || ptr->name_len >= select->name_len) && ptr->is_match(opt))   // search for longest match
                {
                    seen_match = true;
                    if (!ptr->present()) select = ptr;   // name match while cannot select it may indicate repeating command line option
                }
            }
            if (!select)   // didn't find
            {
                if (seen_match) parser_status.set(parser_status.flag.error_overflow_named);
                return 0;
            }

            // Hack: handling "-help" option is different. Flag option.present is reserved for "-help -help" combination to allow both
            // "-help -option" and "-help option" use. Presense of the help request is reflacted in option status in option_help_request bit.
            // (Not only in global param_help_request to enable different "help" options having different actions.)
            select->option_status |= ((select->is_help() && !select->seen_help_request()) ? option_flag::help_request : option_flag::present);

            unsigned len = select->name_len;
            bool same_argument = (::sklib::strlen<unsigned>(opt) > len);
            auto value_start = (same_argument ? opt+len : next_arg);
            auto value_end = select->do_decode(value_start);

            if (!value_end) return len;    // it was switch

            if (same_argument)
            {
                if (value_start == value_end) select->option_status |= option_flag::error_empty;   // if value is not read
                return (int)(value_end - opt);
            }

            // only the body of the value must be in the string, otherwise it is error
            if (value_end - value_start != ::sklib::strlen<unsigned>(next_arg)) select->option_status |= option_flag::error_partial;
            return -1;
        }

        // enter unnamed (plain) parameter; string shall be parsed as the entire body
        // parameters are filled up in the declaration order
        // returns true if something is taken, false if the fist is full
        bool apply_plain_param(const letter_type* opt)
        {
            typedef param_base::option_flag option_flag;

            param_base* select = nullptr;
            for (param_base* ptr = param_list_entry; ptr; ptr = ptr->next_param)
            {
                if (!ptr->is_named_param() && !ptr->present()) select = ptr;
            }
            if (!select) return false;   // didn't find

            select->option_status |= option_flag::present;

            auto value_end = select->do_decode(opt);
            if (!value_end || value_end - opt != ::sklib::strlen<unsigned>(opt)) select->option_status |= option_flag::error_partial;
            // only the body of the value must be in the string, otherwise it is error

            return true;
        }

    };
};

#endif // SKLIB_INCLUDED_CMDPAR_HPP

