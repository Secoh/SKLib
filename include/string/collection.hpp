// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Constexpr function(s) to create static array of null-terminated strings from initializer-like list without memory allocations.
// This is internal SKLib file and must NOT be included directly.

namespace aux
{
    struct collection_cstring_check_type
    {};

    template<unsigned U>
    struct collection_cstring_type : public collection_cstring_check_type
    {
        static constexpr unsigned N = U;
        const char* text[N] = { nullptr };

        constexpr collection_cstring_type() = default;

        constexpr collection_cstring_type(const collection_cstring_type<sklib::priv::alt_max(N, 2u) - 1>& in)   // for completeness, only N >= 2 are used
        {
            for (unsigned k = 0; k < in.N; k++) text[k] = in.text[k];
        }

        constexpr void fill_array(const char* (&out)[N]) const
        {
            for (unsigned k = 0; k < N; k++) out[k] = text[k];
        }
    };
};

namespace priv
{
    template<unsigned N>
    constexpr auto partial_collection_cstring(const sklib::aux::collection_cstring_type<N>& A, const char* str)
    {
        sklib::aux::collection_cstring_type<N + 1> R{ A };
        R.text[N] = str;
        return R;
    }

    template<unsigned N, class ...Args>
    constexpr auto partial_collection_cstring(const sklib::aux::collection_cstring_type<N>& A, const char* str, Args... args)
    {
        sklib::aux::collection_cstring_type<N + 1> R{ A };
        R.text[N] = str;
        return partial_collection_cstring(R, args...);
    }
};

template<class ...Args>
constexpr auto collection_cstring(const char* str, Args... args)
{
    sklib::aux::collection_cstring_type<1> R;
    R.text[0] = str;
    return sklib::priv::partial_collection_cstring(R, args...);
}

template<>
constexpr auto collection_cstring(const char* str)
{
    sklib::aux::collection_cstring_type<1> R;
    R.text[0] = str;
    return R;
}

constexpr auto collection_cstring()  // for completeness
{
    return collection_cstring(nullptr);
}

