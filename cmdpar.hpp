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

// Designators: shortcuts used in help-related strings to indicate current option prefix and the option name in question

#define SKLIB_PARAM_DESG_PREFIX_S "\a"    /* replaces the Prefix letter */
#define SKLIB_PARAM_DESG_SELF_S   "\b"    /* replaces the name of the current parameter (self) */

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
//          PARAM_INT(option_name [ , default_value [ , context_help ] ] );
//
//    b) Optional named parameter, except Switch - same as required
//          OPTION_INT(option_name, [ default_value [ , context_help ] ] );
//
//    c) Switch, optional - special parameter that shows only presence (no value)
//          OPTION_SWITCH(option_name [ , context_help ] );
//
//    d) Help option declaration
//          OPTION_HELP(option_name [ , context_help ] [ , putchar_callback ] );
//
//    e) Required plain parameters - same as named, given without prefixed name
//          PLAIN_INT(option_name [ , default_value [ , context_help ] ] );
//
//    f) Optional plain parameters - similar as named
//          PLAIN_OPT_INT(option_name [ , default_value [ , context_help ] ] );
//
// 3) Declaring context help
//    Must be done in C++ context that allows static constexpr variables
//
//      DECLARE_OPTION_HELP(name, header_text [ , body_text_1, body_text_2, ... ] );
//

#define SKLIB_DECLARE_CMD_PARAMS(name) struct name : public ::sklib::params_table_base_type

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

#define SKLIB_DECLARE_OPTION_HELP(name,head,...) \
static constexpr auto name = ::sklib::param_help_type(head, ::sklib::collection_cstring(__VA_ARGS__));

// Param Parser classes

namespace sklib
{
    // Standalone string collection to describe an option in the command line parser
    template<unsigned U>
    struct param_help_type
    {
        const char* head = nullptr;
        const char* text[U] = { nullptr };

        // Standalone string collection to describe an option in the command line parser
        constexpr param_help_type() = default;

        // hd - single line for short description; ld - text page explaining the option
        constexpr param_help_type(const char* hd, const ::sklib::supplement::collection_cstring_type<U>& ld) : head(hd)
        {
            ld.fill_array(text);
        }
    };

    namespace internal
    {
        struct param_help_receiver_type
        {
            const char* head = nullptr;
            const char* const* text = nullptr;
            unsigned size = 0;

            template<unsigned U>
            constexpr param_help_receiver_type(const ::sklib::param_help_type<U>& ld)
                : head(ld.head)
                , text((U&& ld.text&& ld.text[0]) ? ld.text : nullptr)
                , size((U && ld.text && ld.text[0]) ? U : 0)
            {}
        };
    };

    class params_table_base_type
    {
    public:
        class param_switch;
        class param_help;

    // === Parser Defaults

    private:        //sk! TODO support different platforms
#ifdef _WIN32
        static void stdout_putc(char c) { putchar(c); }
        static constexpr auto def_putc = stdout_putc;
#else
        static constexpr auto def_putc = nullptr;
#endif

    protected:
        static constexpr char def_prefix = '-';
        static constexpr char def_sym_reqd = '*';
        static constexpr int def_min_help_spacing = 3;  // in help print, width between names column and "required" column

        static constexpr auto def_help_intro = ::sklib::param_help_type(nullptr,
                      ::sklib::collection_cstring("Named options use: " SKLIB_PARAM_DESG_PREFIX_S "option value   General use of named parameter",
                                                  "                   " SKLIB_PARAM_DESG_PREFIX_S "option         Without value, if parameter is a switch (presence only)",
                                                  "Options can be combined in one argument: " SKLIB_PARAM_DESG_PREFIX_S "abc, where a, b, c are different parameters.",
                                                  "Value can follow the option without space: " SKLIB_PARAM_DESG_PREFIX_S "OptionValue if parsing is not ambiguous.",
                                                  "Argument without leading \"" SKLIB_PARAM_DESG_PREFIX_S "\" is plain argument. Plain parameters are read one after another.",
                                                  "Double prefix, \"" SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_PREFIX_S "\", makes the next argument plain even if it starts with \"" SKLIB_PARAM_DESG_PREFIX_S "\".",
                                                  "If value in an option starts with prefix symbol, it can also be escaped with preceding \"" SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_PREFIX_S "\"." ));

    // === Hidden Settings and Configuration

        const int min_help_spacing = def_min_help_spacing;
        const char sym_reqd = def_sym_reqd;
        const ::sklib::internal::param_help_receiver_type descr_intro{ def_help_intro };

        param_switch* param_list_entry = nullptr;
        const param_help* param_help_entry = nullptr;

    // === Open Setting and Parse Status

    public:
        const char Prefix = def_prefix;

        unsigned parse_status = 0;
        static constexpr unsigned status_good       = 0x0001;  // indicates that parser has no problems
        static constexpr unsigned status_read       = 0x0002;  // parser has parameter(s) that are read (aka not empty), even with errors
        static constexpr unsigned status_help       = 0x0004;  // help is requested
        static constexpr unsigned status_errors     = 0x0008;  // parser has errors; other parse_status bits and/or status of specific parameter(s) may contain more details
        static constexpr unsigned status_incomplete = 0x0010;  // some of (or all) required parameters are not present
        static constexpr unsigned status_unknown    = 0x0020;  // error: unrecognized (or duplicate) parameter(s)
        static constexpr unsigned status_overflow   = 0x0040;  // error: too many plain parameters

    // === Constructors

        params_table_base_type(char option_prefix = def_prefix, char custom_sym_reqd = def_sym_reqd, int custom_help_spacing = def_min_help_spacing)
            : params_table_base_type(option_prefix, custom_sym_reqd, custom_help_spacing, def_help_intro) {}

        template<unsigned U>
        params_table_base_type(char option_prefix,
                               char custom_sym_reqd,
                               int custom_help_spacing,
                               const ::sklib::param_help_type<U>& help_intro)
            : Prefix(option_prefix)
            , sym_reqd(custom_sym_reqd)
            , min_help_spacing(custom_help_spacing)
            , descr_intro(help_intro)
        {}

    // ====================================================================================================
    // === Parameters Descriptors
    //
    // all parameters have fields:
    //  - present     true if command line contains this parameter
    //  - status      result of reading this individual parameter
    //  - decode()    protected function that is used to read and interpret the value - advances pointer and sets error status
    //
    // all parameters except param_switch have:
    //  - value       the value associated with this parameter

    public:
        class param_switch  // also, base type for all other parameters
        {
            friend params_table_base_type;

        public:
            bool present = false;
            uint8_t status = param_no_errors;

            static constexpr uint8_t param_no_errors     = 0x00;   // option state is correct (even if not present)
            static constexpr uint8_t param_error_empty   = 0x01;   // option is present but data portion is not
            static constexpr uint8_t param_error_partial = 0x02;   // data portion shall be read in full but it was not
            static constexpr uint8_t param_is_help       = 0x80;   // hack: this bit signals that the parameter is used in help subsystem

        protected:
            const char* const name = "";
            const unsigned name_len = 0;
            const bool required = false;
            const ::sklib::internal::param_help_receiver_type descr;
            param_switch* const next = nullptr;

            static constexpr auto help_null = ::sklib::param_help_type<1>();
            static constexpr auto help_help = ::sklib::param_help_type("Prints help on options. Use \"" SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_SELF_S " " SKLIB_PARAM_DESG_SELF_S "\" for more.",
                              ::sklib::collection_cstring(SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_SELF_S "             prints the list of available options",
                                         SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_SELF_S " [" SKLIB_PARAM_DESG_PREFIX_S "]option   prints the help text on the selected option (if available)"));
        public:
            constexpr param_switch(params_table_base_type* root,
                                   const char* param_name,
                                   bool param_required = false)
                : param_switch(root, param_name, param_required, help_null)
            {}

            template<unsigned U>
            constexpr param_switch(params_table_base_type* root,
                                   const char* param_name,
                                   bool param_required,
                                   const ::sklib::param_help_type<U>& param_descr)
                : name(param_name)
                , name_len((unsigned)::sklib::strlen(param_name))
                , required(param_required)
                , descr(param_descr)
                , next(root->param_list_entry)
            {
                root->param_list_entry = this;
            }

        protected:
            static constexpr int     global_defval_int       = int();
            static constexpr double  global_defval_double    = double();
            static constexpr char    global_defval_key       = '\0';
            static constexpr char    global_defval_cstring[] = "";

            // read and accept the value of the parameter, escept for switch
            // derived classes for other parameter types shall override this fuction
            virtual const char* decode(const char* arg)
            {
                return nullptr;  // special case, no value for switch
            }
        };

        class param_int : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                char* pstop;
                value = strtol(arg, &pstop, 10);
                return pstop;
            }

        public:
            int value = global_defval_int;

            constexpr param_int(params_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                int defval = global_defval_int)
                : param_int(root, param_name, param_required, defval, help_null)
            {}

            template<unsigned U>
            constexpr param_int(params_table_base_type* root,
                                const char* param_name,
                                bool param_required,
                                int defval,
                                const ::sklib::param_help_type<U>& param_descr)
                : param_switch(root, param_name, param_required, param_descr)
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
                return pstop;
            }

        public:
            int64_t value = global_defval_int;

            constexpr param_int64(params_table_base_type* root,
                                  const char* param_name,
                                  bool param_required = false,
                                  int64_t defval = global_defval_int)
                : param_int64(root, param_name, param_required, defval, help_null)
            {}

            template<unsigned U>
            constexpr param_int64(params_table_base_type* root,
                                  const char* param_name,
                                  bool param_required,
                                  int64_t defval,
                                  const ::sklib::param_help_type<U>& param_descr)
                : param_switch(root, param_name, param_required, param_descr)
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
                return pstop;
            }

        public:
            double value = global_defval_double;

            constexpr param_double(params_table_base_type* root,
                                   const char* param_name,
                                   bool param_required = false,
                                   double defval = global_defval_double)
                : param_double(root, param_name, param_required, defval, help_null)
            {}

            template<unsigned U>
            constexpr param_double(params_table_base_type* root,
                                   const char* param_name,
                                   bool param_required,
                                   double defval,
                                   const ::sklib::param_help_type<U>& param_descr)
                : param_switch(root, param_name, param_required, param_descr)
                , value(defval)
            {}
        };

        class param_key : public param_switch
        {
        protected:
            const char* decode(const char* arg)
            {
                value = *arg;
                if (value) arg++;
                return arg;
            }

        public:
            char value = global_defval_key;

            constexpr param_key(params_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                char defval = global_defval_key)
                : param_key(root, param_name, param_required, defval, help_null)
            {}

            template<unsigned U>
            constexpr param_key(params_table_base_type* root,
                                const char* param_name,
                                bool param_required,
                                char defval,
                                const ::sklib::param_help_type<U>& param_descr)
                : param_switch(root, param_name, param_required, param_descr)
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

            constexpr param_str(params_table_base_type* root,
                                const char* param_name,
                                bool param_required = false,
                                const char* defval = global_defval_cstring)
                : param_str(root, param_name, param_required, defval, help_null)
            {}

            template<unsigned U>
            constexpr param_str(params_table_base_type* root,
                                const char* param_name,
                                bool param_required,
                                const char* defval,
                                const ::sklib::param_help_type<U>& param_descr)
                : param_switch(root, param_name, param_required, param_descr)
                , value(defval)
            {}
        };

        class param_help : public param_str
        {
            friend params_table_base_type;

        protected:
            params_table_base_type* const parent = nullptr;

            typedef sklib::internal::c_callback_type<void, char> help_putc_callback_type;
            const help_putc_callback_type putc_callback;

            void write(char c, const char* name = "") const
            {
                if (c == SKLIB_PARAM_DESG_SELF_S[0])
                {
                    // class scope cannot have forward declarations - enforce that the function comes from this class even if similar name already exists
                    // and, potential recursion breaks at step 3, because the substitution string will be empty
                    this->write(name);
                }
                else
                {
                    putc_callback(c == SKLIB_PARAM_DESG_PREFIX_S[0] ? parent->Prefix : c);
                }
            }
            void write(const char* str, const char* name = "") const
            {
                if (str && putc_callback) while(*str) write(*str++, name);
            }
            void writeln(const char* str, const char* name = "") const
            {
                write(str, name);
                write('\n');
            }

            // problem: parent class doesn't know where is the help class located - it only has L1 list of param_switch base classes
            // (extreme case - multiple help-bound parameters were programmed, one was called - which one?)
            // lets exploit the fact that virtual decode() is called for each parameter encountered in the line
            const char* decode(const char* arg)
            {
                parent->param_help_entry = this;
                return param_str::decode(arg);
            }

        public:
            constexpr param_help(params_table_base_type* root,
                                 const char* param_name,
                                 const help_putc_callback_type& print_line_func = help_putc_callback_type{ stdout_putc })
                : param_help(root, param_name, help_help, print_line_func)
            {}

            template<unsigned U>
            constexpr param_help(params_table_base_type* root,
                                 const char* param_name,
                                 const ::sklib::param_help_type<U>& param_descr,
                                 const help_putc_callback_type& print_line_func = help_putc_callback_type{ stdout_putc })
                : param_str(root, param_name, false, global_defval_cstring, param_descr)
                , parent(root)
                , putc_callback(print_line_func)
            {
                status |= param_is_help;
            }
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
            parse_status = 0;

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
                    if (ptr->status & ~param_switch::param_is_help) parse_status |= status_errors;
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

    // ====================================================================================================
    // === Help Subsystem

        // If Help option is engaged be parse() finction, show_help() displays appropriate text block relevant to the request
        //
        bool show_help() const
        {
            if (!param_help_entry) return false;

            if (param_help_entry->value && *param_help_entry->value)
            {
                auto subject_ptr = this->find_param(param_help_entry->value).first;
                if (!subject_ptr)
                {
                    this->help_diagnistics("This option doesn\'t exist:", param_help_entry->value);
                    return false;
                }

                this->help_show_for_one_param(subject_ptr);   // class scope cannot have forward declarations - enforce that the function comes from this class even if similar name already exists
                return true;
            }

            // else, if another option is entered, print specifc help and exit
            // keep track of maximum name length for the next operation

            unsigned max_name_length = 0;
            int depth = -1;
            while (true)
            {
                auto entry = this->find_param(nullptr, depth);
                depth = entry.second;

                auto subject_ptr = entry.first;
                if (!subject_ptr) break;

                max_name_length = std::max(max_name_length, subject_ptr->name_len);

                if (subject_ptr->present && !(subject_ptr->status & param_switch::param_is_help))
                {
                    this->help_show_for_one_param(subject_ptr);
                    return true;
                }
            }

            // else, help alone - print list of all options
            // first, print intro, if requested

            if (descr_intro.head)
            {
                param_help_entry->writeln(descr_intro.head);
            }
            else if (descr_intro.text)
            {
                for (unsigned k = 0; k < descr_intro.size; k++) param_help_entry->writeln(descr_intro.text[k]);
            }

            // then, print the list of options

            param_help_entry->write("Available options: (");
            param_help_entry->write(sym_reqd);
            param_help_entry->writeln(" if required)");

            depth = -1;
            while (true)
            {
                auto entry = this->find_param(nullptr, depth);
                depth = entry.second;

                auto subject_ptr = entry.first;
                if (!subject_ptr) break;

                int spacing = min_help_spacing + max_name_length - subject_ptr->name_len;
                this->help_prhead(subject_ptr->name, spacing, subject_ptr->required, subject_ptr->descr.head);
            }

            param_help_entry->writeln("Plain parameters:");

            depth = -1;
            while (true)
            {
                auto entry = this->find_param("", depth);
                depth = entry.second;

                auto subject_ptr = entry.first;
                if (!subject_ptr) break;

                this->help_prhead(nullptr, 1, subject_ptr->required, (subject_ptr->descr.head ? subject_ptr->descr.head : "< no description >"));
            }

            return true;
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

        // help formatting and printing
        void help_diagnistics(const char* str, const char* name) const
        {
            param_help_entry->writeln(str);
            param_help_entry->writeln("  " SKLIB_PARAM_DESG_PREFIX_S SKLIB_PARAM_DESG_SELF_S, name);
        }
        void help_prhead(const char* name, int spacer, bool required, const char* head) const
        {
            if (name) param_help_entry->write(Prefix);
            param_help_entry->write(name);
            for (int i=0;i<spacer; i++) param_help_entry->write(" ");
            param_help_entry->write(required ? sym_reqd : ' ');
            param_help_entry->write(' ');
            param_help_entry->writeln(head, name);
        }
        void help_show_for_one_param(const param_switch* subject_ptr) const
        {
            auto& subject_help = subject_ptr->descr;
            if (subject_help.size && subject_help.text && !(subject_help.size==1 && !subject_help.text[0]))   // additional test for single nullptr in the text section
            {
                for (unsigned k=0; k<subject_help.size; k++) param_help_entry->writeln(subject_help.text[k], subject_ptr->name);
            }
            else if (subject_help.head)
            {
                help_prhead(subject_ptr->name, min_help_spacing, subject_ptr->required, subject_help.head);
            }
            else
            {
                help_diagnistics("This option doesn\'t have help text:", subject_ptr->name);
            }
        }
    };
};

#endif // SKLIB_INCLUDED_CMDPAR_HPP

