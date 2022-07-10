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


// Work In Progress: monitor for TODO tags


#ifndef SKLIB_INCLUDED_CMDPAR_HPP
#define SKLIB_INCLUDED_CMDPAR_HPP

#include "helpers.hpp"

// Option vs Parameter vs Argument
//   Parameter is required, option is optional
//   Definitions of classes and members in the list are Parameters
//   When something in context is optional, it is Option
//   Argument is string portion of command line to be interpreted by parser

// Shortcuts for Parameter Set declaration
//
// 1) Declaring set of parameters
//
//      DECLARE_CMD_PARAMS(my_param_class_name)
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

#define SKLIB_DECLARE_CMD_PARAMS(name) struct name : public ::sklib::cmdpar_table_base_type

#define SKLIB_PARAM_INT(x,...)      param_int    x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_INT64(x,...)    param_int64  x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_DOUBLE(x,...)   param_double x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_KEY(x,...)      param_key    x{ this, #x , true, __VA_ARGS__ };
#define SKLIB_PARAM_STRING(x,...)   param_str    x{ this, #x , true, __VA_ARGS__ };

#define SKLIB_OPTION_SWITCH(x,...)  param_switch x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_INT(x,...)     param_int    x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_INT64(x,...)   param_int64  x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_DOUBLE(x,...)  param_double x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_KEY(x,...)     param_key    x{ this, #x , false, __VA_ARGS__ };
#define SKLIB_OPTION_STRING(x,...)  param_str    x{ this, #x , false, __VA_ARGS__ };

#define SKLIB_OPTION_HELP(x,...)    param_help   x{ this, #x , __VA_ARGS__ };

#define SKLIB_PLAIN_INT(x,...)      param_int    x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_INT64(x,...)    param_int64  x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_DOUBLE(x,...)   param_double x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_KEY(x,...)      param_key    x{ this, "" , true, __VA_ARGS__ };
#define SKLIB_PLAIN_STRING(x,...)   param_str    x{ this, "" , true, __VA_ARGS__ };

#define SKLIB_PLAIN_OPT_INT(x,...)    param_int    x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPT_INT64(x,...)  param_int64  x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPT_DOUBLE(x,...) param_double x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPT_KEY(x,...)    param_key    x{ this, "" , false, __VA_ARGS__ };
#define SKLIB_PARAM_OPT_STRING(x,...) param_str    x{ this, "" , false, __VA_ARGS__ };

#define SKLIB_PARAMS_ALT_PREFIX(c)  setter_prefix alternative_prefix{ this, c };


// Param Parser classes

namespace sklib
{
    template<class T = char>
    class cmdpar_table_base_type
    {
    public:
        class param_switch;
        class param_help;

        typedef std::remove_cv_t<T> letter_type;

    // === Parser Defaults

    protected:
        static constexpr letter_type def_prefix = static_cast<letter_type>('-');

    // === Hidden Settings and Configuration

        letter_type Prefix = def_prefix;   // cannot be const because order of initialization: can be modified from derived class

        param_switch* param_list_entry = nullptr;
        const param_help* param_help_entry = nullptr;

    // === Open Setting and Parse Status

    public:
        uint16_t parser_status = parser_nothing;

        static constexpr uint16_t parser_nothing      = 0;       // parser wasn't run
        static constexpr uint16_t parser_good         = 0x0001;  // all checks satisfied, no errors, and not help request
        static constexpr uint16_t parser_empty        = 0x0002;  // set if the input is empty; still may be valid state
        static constexpr uint16_t parser_error        = 0x0004;  // error state; check clarification bit for specific problem
        static constexpr uint16_t parser_error_unknown_name   = 0x0008;  // unrecognized parameter(s); also, unrecognized help request
        static constexpr uint16_t parser_error_missing_named  = 0x0010;  // some (or all) required named parameters are not present
        static constexpr uint16_t parser_error_missing_plain  = 0x0020;  // some (or all) required plain parameters are not present
        static constexpr uint16_t parser_error_overflow_named = 0x0040;  // too many (repeating) named parameters
        static constexpr uint16_t parser_error_overflow_plain = 0x0080;  // too many plain parameters
        static constexpr uint16_t parser_error_malformed      = 0x0100;  // some (or all) parameters are malformed (represented incorrectly) - check individual options
        static constexpr uint16_t parser_help_request = 0x0200;  // help is requested; check specific bit(s) to see the request(s)
        static constexpr uint16_t parser_help_banner  = 0x0400;  // general help is requested; -help without parameter
        static constexpr uint16_t parser_help_help    = 0x0800;  // help on help is requested; -help help
        static constexpr uint16_t parser_help_option  = 0x1000;  // help on parameter is requested; may set parser_unknown bit if the parameter is unknown

        bool is_good() const            { return (parser_status & parser_good); }
        bool is_parser_error() const    { return (parser_status & parser_error); }
        bool is_help_requested() const  { return (parser_status & parser_help_request); }

    // ====================================================================================================
    // === Parameters Descriptors
    //
    // all parameters have fields:
    //  - status      result of reading this individual parameter
    //  - decode()    protected function that is used to read and interpret the value - advances pointer and sets error status
    //  - present()   helper function to interpret status field; true if command line contains this option
    //
    // all parameters except param_switch have:
    //  - value       the value associated with this parameter

        class param_switch  // also, base type for all other parameters
        {
            friend cmdpar_table_base_type;

        public:
            uint8_t status = option_nothing;
            bool present() const { return (status & option_present); }

            static constexpr uint8_t option_nothing       = 0;     // option state is correct (even if not present)
            static constexpr uint8_t option_present       = 0x01;  // option is present
            static constexpr uint8_t option_error_empty   = 0x02;  // data is required for the option type, but data is not found
            static constexpr uint8_t option_error_partial = 0x04;  // in split options, data portion shall occupy the entire argument, but the read terminates before end
            static constexpr uint8_t option_help_request  = 0x40;  // hack: first help request sets this bit instead of Present bit to allow help on help (double) requests
            static constexpr uint8_t option_is_help       = 0x80;  // hack: this bit signals that the option is used in help subsystem

        protected:
            const letter_type* const name = "";
            const unsigned name_len = 0;
            const bool required = false;
            param_switch* const next = nullptr;

        public:
            constexpr param_switch(cmdpar_table_base_type* root,
                                   const letter_type* param_name,
                                   bool param_required = false)
                : name(param_name)
                , name_len((unsigned)::sklib::strlen(param_name))
                , required(param_required)
                , next(root->param_list_entry)
            {
                root->param_list_entry = this;
            }

        protected:
            static constexpr int         global_defval_int       = int();
            static constexpr double      global_defval_double    = double();
            static constexpr letter_type global_defval_key       = 0;                       // = '\0';
            static constexpr letter_type global_defval_cstring[] = { global_defval_key };   // = "";

            // read and accept the value of the parameter, escept for switch
            // derived classes for other parameter types shall override this fuction
            virtual const letter_type* decode(const letter_type* arg)
            {
                return nullptr;  // special case, no value for switch
            }
        };

        class param_int : public param_switch
        {
        protected:
            const letter_type* decode(const letter_type* arg)
            {
                char* pstop;
                value = strtol(arg, &pstop, 10);
                if (pstop == arg) status |= option_error_partial;
                return pstop;
            }

        public:
            int value = global_defval_int;

            constexpr param_int(cmdpar_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                int defval = global_defval_int)
                : param_switch(root, param_name, param_required)
                , value(defval)
            {}
        };

        class param_int64 : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                char* pstop;
                value = strtoll(arg, &pstop, 10);
                if (pstop == arg) status |= option_error_partial;
                return pstop;
            }

        public:
            int64_t value = global_defval_int;

            constexpr param_int64(cmdpar_table_base_type* root,
                                  const char* param_name,
                                  bool param_required = false,
                                  int64_t defval = global_defval_int)
                : param_switch(root, param_name, param_required)
                , value(defval)
            {}
        };

        class param_double : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                char* pstop;
                value = strtod(arg, &pstop);
                if (pstop == arg) status |= option_error_partial;
                return pstop;
            }

        public:
            double value = global_defval_double;

            constexpr param_double(cmdpar_table_base_type* root,
                                   const char* param_name,
                                   bool param_required = false,
                                   double defval = global_defval_double)
                : param_switch(root, param_name, param_required)
                , value(defval)
            {}
        };

        class param_key : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                value = *arg;
                if (value) arg++; else status |= option_error_partial;
                return arg;
            }

        public:
            char value = global_defval_key;

            constexpr param_key(cmdpar_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                char defval = global_defval_key)
                : param_switch(root, param_name, param_required)
                , value(defval)
            {}
        };

        class param_str : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                value = arg;
                return arg + ::sklib::strlen(arg);
            }

        public:
            const char* value = global_defval_cstring;

            constexpr param_str(cmdpar_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                const char* defval = global_defval_cstring)
                : param_switch(root, param_name, param_required)
                , value(defval)
            {}
        };

        class param_help : public param_str
        {
            friend cmdpar_table_base_type;

        protected:
            cmdpar_table_base_type* const parent = nullptr;

            // problem: parent class doesn't know where is the help class located - it only has L1 list of param_switch base classes
            // (extreme case - multiple help-bound parameters were programmed, one was called - which one?)
            // lets exploit the fact that virtual decode() is called for each parameter encountered in the line
            const char* decode(const char* arg)
            {
                parent->param_help_entry = this;
                return param_str::decode(arg);
            }

        public:
            constexpr param_help(cmdpar_table_base_type* root,
                                 const char* param_name)
                : param_str(root, param_name)
                , parent(root)
            {
                status |= option_is_help;
            }
        };

    // === Alternative Settings

        struct setter_prefix
        {
            setter_prefix(cmdpar_table_base_type* root, char custom_prefix)
            { root->Prefix = custom_prefix; }
        };

    // ====================================================================================================
    // === Main Parser Entry

        // parse the command line arguments into the parameter set (defined in the class)
        // - named options, can be represented in 2 ways:
        //   1) -a##b##... all switches, keys and numbers in the same string without separation (only logical separation)
        //   2) -a ### - option in one command line argument, its value (assuming the option has a value) in the next argument (detached parameter)
        // - plain (not named) options, normally taken from command line arguments without prefix
        // - double prefix anywhere cancels special treatment of the next command line argument (when it starts with prefix)
        //
        bool parse(int argn, const char* const* argc, int arg_start = 1)
        {
            // note that this function is intended for single call, but lets clear this variable anyway
            parser_status = parser_nothing;

            bool signal_plain = false;  // if true, next arument is interpreted as plain parameter ("GNU style")

            for (int k = arg_start; k < argn; k++)
            {
                auto arg_cur = argc[k];
                if (!arg_cur || !*arg_cur) continue;  // this cannot happen

                if (*arg_cur == Prefix && !signal_plain)
                {
                    unsigned arg_len = (unsigned)::sklib::strlen(arg_cur);
                    for (unsigned t = 1; t < arg_len; t++)
                    {
                        if (arg_cur[t] == Prefix)
                        {
                            signal_plain = true;    // second prefix supresses next parameter: -- -string in command line => "-string" becomes plain parameter
                            continue;
                        }

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
                        else if (arg_val[0] != Prefix)
                        {
                            ;   // keep current arg_val, even if empty
                        }
                        else if (arg_val[1] != Prefix)    // before check, arg_val = "-..."
                        {
                            arg_val = "";   // next element is parameter, no value
                        }
                        else if (arg_val[2] == '\0')    // arg_val = "--"
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
                            parse_status |= status_unknown;
                        }
                    }
                }
                else
                {
                    if (!this->apply_plain_param(arg_cur)) parse_status |= status_overflow;
                    signal_plain = false;
                }
            }

            bool event_present = false;
            bool event_help = false;

            for (param_switch* ptr = param_list_entry; ptr; ptr = ptr->next)
            {
                if (ptr->present)
                {
                    event_present = true;
                    if (ptr->status & param_switch::param_is_help) event_help = true;
                    if (ptr->status & ~param_switch::param_is_help) parse_status |= (status_errors | status_malformed);
                }
                else
                {
                    if (ptr->required) parse_status |= status_incomplete;
                }
            }

            parse_status |= (parse_status ? status_errors : status_good);   // force "have errors" bit, or "good" bit if no errors

            if (event_present) parse_status |= status_read;
            if (event_help) parse_status |= status_help;

            return (parse_status & (status_good | status_help));
        }

    // === Helper Functions, manipulating parameters, help formatting/printing

    protected:
        // find the param_switch entry in the L1 list
        // criteria for selection: most distant, name match, and depth is strictly less than the given depth
        // if name is a null, any non-empty name is a match; if name is not null, exact match is tested
        // if depth limit is negative, any depth is a match
        // returns pointer to the element or null if not found;
        // returns the current depth of the found element or negative if not found
        std::pair<const param_switch*, int> find_param(const char* name, int depth_cap = -1) const
        {
            int depth = 0;
            int depth_seen = -1;
            param_switch* ptr_select = nullptr;
            for (param_switch* ptr = param_list_entry; ptr && (depth_cap < 0 || depth < depth_cap); depth++, ptr = ptr->next)
            {
                if (name ? ::sklib::strequ(name, ptr->name) : ptr->name_len)
                {
                    depth_seen = depth;
                    ptr_select = ptr;
                }
            }
            return { ptr_select, depth_seen };
        }

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
        int apply_named_param(const char* opt, const char* next)
        {
            param_switch* select = nullptr;
            for (param_switch* ptr = param_list_entry; ptr; ptr = ptr->next)
            {
                if (ptr->name_len && !ptr->present &&                   // named param, wasn't read
                    (!select || ptr->name_len >= select->name_len) &&   // search for longest match
                    ::sklib::strnequ(opt, ptr->name, ptr->name_len))
                {
                    select = ptr;
                }
            }
            if (!select) return 0;   // didn't find

            select->present = true;

            unsigned len = select->name_len;
            bool same_argument = (::sklib::strlen(opt) > len);
            auto value_start = (same_argument ? opt+len : next);
            auto value_end = select->decode(value_start);

            if (!value_end) return len;    // it was switch

            if (same_argument)
            {
                if (value_start == value_end) select->status |= param_switch::param_error_empty;   // if value is not read
                return (int)(value_end - opt);
            }

            if (value_end - value_start != ::sklib::strlen(next)) select->status |= param_switch::param_error_partial;  // only the body of the value must be in the string, otherwise it is error
            return -1;
        }

        // enter unnamed (plain) parameter; string shall be parsed as the entire body
        // parameters are filled up in the declaration order
        // returns true if something is taken, false if the fist is full
        bool apply_plain_param(const char* opt)
        {
            param_switch* select = nullptr;
            for (param_switch* ptr = param_list_entry; ptr; ptr = ptr->next)
            {
                if (!ptr->name_len && !ptr->present) select = ptr;
            }
            if (!select) return false;   // didn't find

            select->present = true;

            auto value_end = select->decode(opt);
            if (!value_end || value_end - opt != ::sklib::strlen(opt)) select->status |= param_switch::param_error_partial;  // only the body of the value must be in the string, otherwise it is error

            return true;
        }

    };
};

#endif // SKLIB_INCLUDED_CMDPAR_HPP

