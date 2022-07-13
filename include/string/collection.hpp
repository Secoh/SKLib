#pragma once

namespace supplement
{
    struct collection_cstring_check_type
    {};

    template<unsigned U>
    struct collection_cstring_type : public collection_cstring_check_type
    {
        static constexpr unsigned N = U;
        const char* text[N] = { nullptr };

        constexpr collection_cstring_type() = default;

        constexpr collection_cstring_type(const collection_cstring_type<::sklib::internal::alternate_maximum(N, 2u) - 1>& in)   // for completeness, only N >= 2 are used
        {
            for (unsigned k = 0; k < in.N; k++) text[k] = in.text[k];
        }

        constexpr void fill_array(const char* (&out)[N]) const
        {
            for (unsigned k = 0; k < N; k++) out[k] = text[k];
        }
    };
};

namespace internal
{
    template<unsigned N>
    constexpr auto partial_collection_cstring(const ::sklib::supplement::collection_cstring_type<N>& A, const char* str)
    {
        ::sklib::supplement::collection_cstring_type<N + 1> R{ A };
        R.text[N] = str;
        return R;
    }

    template<unsigned N, class ...Args>
    constexpr auto partial_collection_cstring(const ::sklib::supplement::collection_cstring_type<N>& A, const char* str, Args... args)
    {
        ::sklib::supplement::collection_cstring_type<N + 1> R{ A };
        R.text[N] = str;
        return partial_collection_cstring(R, args...);
    }
};

template<class ...Args>
constexpr auto collection_cstring(const char* str, Args... args)
{
    ::sklib::supplement::collection_cstring_type<1> R;
    R.text[0] = str;
    return ::sklib::internal::partial_collection_cstring(R, args...);
}

template<>
constexpr auto collection_cstring(const char* str)
{
    ::sklib::supplement::collection_cstring_type<1> R;
    R.text[0] = str;
    return R;
}

constexpr auto collection_cstring()  // for completeness
{
    return collection_cstring(nullptr);
}

