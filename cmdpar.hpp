#pragma once


namespace sklib
{
    // help subsystem

    namespace supplement
    {
        struct help_data_base_type
        {
            const char* head;
            const unsigned N = 0;
            const char* const * text = nullptr;

            // poor man virtualization :)
            constexpr help_data_base_type(const char* str_head, unsigned count, const char* const * pstr_body) : head(str_head), N(count), text(pstr_body) {}

            //sk this shall go away in the end
            void demo() const
            {
                std::cout << "H " << head << '\n';
                for (unsigned k = 0; k < N; k++) std::cout << k << " " << text[k] << '\n';
            }
        };

    };

    namespace internal
    {
        template<unsigned U>
        struct help_data_type : public ::sklib::supplement::help_data_base_type
        {
            const char* data[U];
            help_data_type(const char* str) : help_data_base_type(str, U, data) {}
        };
    };

    template<class SET>
    constexpr auto par_create_help_item(const char* str, const SET& str_set)
    {
        static_assert(std::is_base_of_v<::sklib::supplement::collection_cstring_check_type, SET>, "par_create_help_item() error: 2nd parameter must be string collection");
        const unsigned N = SET::N;
        ::sklib::internal::help_data_type<N> R{ str };
        str_set.fill_array(R.data);   // NB: address of the string array is already saved during construction; here we are rolling in the data
        return R;
    }

    // classes for parameters descriptors





};

/*
class param
{
public:
    bool present;
    const char* name;
};

class par_int : public param
{
public:
    int value;

};

//!! need to avoid allocations

// need class entry for help entries:
// 1) short description, 1 line (displayed in list for command: -help)
// 2) text to be displayed for: -help param

#define SKLIB_PARAM(type,name,defval) ::sklib::internal::param_declare<type> name{##name, static_cast<type>(defval), this};

class what
{
    SKLIB_PAR_
};
*/
