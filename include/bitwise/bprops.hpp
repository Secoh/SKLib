// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides int-like bit property fields, with strond type check and mutually exclusive composition
// This is internal SKLib file and must NOT be included directly.

namespace priv
{
    struct bit_props_anchor {};
    struct bit_props_group_anchor {};
    struct bit_props_config_anchor {};
};

template<class Anchor, class T>
class bit_props_data_type : public Anchor, public sklib::priv::bit_props_anchor
{
    static_assert(sklib::is_integer_v<T>, "Second parameter of bit_props_data_type template must be integer");

public:
    typedef T data_type;
    const T data{};

    template<class CTest,
        SKLIB_INTERNAL_ENABLE_IF_CONDITION((std::is_base_of_v<sklib::priv::bit_props_group_anchor, CTest>) &&
                                           (std::is_base_of_v<bit_props_data_type, CTest>))>
    constexpr bool has(CTest what) const
    {
        return ((data & what.mask) == what.data);
    }

    template<class CTest,
        SKLIB_INTERNAL_ENABLE_IF_CONDITION((std::is_base_of_v<sklib::priv::bit_props_group_anchor, CTest>) &&
                                           (std::is_base_of_v<bit_props_data_type, CTest>))>
    constexpr bool operator[] (CTest what) const
    {
        return has(what);
    }

protected:
    // do we need it? // typedef Anchor CallerAnchor;
    constexpr bit_props_data_type(T x) : data(x) {}
};

template<class CData, CData::data_type gMask, SKLIB_INTERNAL_ENABLE_IF_DERIVED(CData, sklib::priv::bit_props_anchor)>
class bit_props_group_type : public CData, public sklib::priv::bit_props_group_anchor
{
protected:
    constexpr bit_props_group_type(CData::data_type x) : CData(x & gMask) {}

public:
    static constexpr CData::data_type mask = gMask;

    constexpr auto get() const { return *this; }         // for completeness, return itself
    constexpr auto operator+ () const { return *this; }  // in bit_props_group flavor

};

namespace priv
{
    template<class CData, CData::data_type gMask>
    struct bit_props_group_helper : public bit_props_group_type<CData, gMask>
    {
        constexpr bit_props_group_helper(CData::data_type x) : bit_props_group_type<CData, gMask>(x) {}
        constexpr auto get() const { return static_cast<bit_props_group_type<CData, gMask>>(*this); }
    };
};

template<class CData, CData::data_type gMask1, CData::data_type gMask2,
    SKLIB_INTERNAL_ENABLE_IF_CONDITION((std::is_base_of_v<sklib::priv::bit_props_anchor, CData>) && !(gMask1 & gMask2))>
constexpr auto operator+ (bit_props_group_type<CData, gMask1> X, bit_props_group_type<CData, gMask2> Y)
{
    return sklib::priv::bit_props_group_helper<CData, (gMask1 | gMask2)>(X.data | Y.data).get();
}

namespace priv
{
    template<class T>
    struct bit_props_dummy_config_type
    {
        static constexpr T start = 0;
        static constexpr T size = 0;
    };

    template<class CData, class CPrevConf, auto enumCap>
    class bit_props_config_placement_helper
    {
    private:
        typedef typename CData::data_type data_type;
        typedef std::conditional_t<std::is_base_of_v<CData, CPrevConf>,
            CPrevConf, bit_props_dummy_config_type<data_type>> CPrevConfEx;
        static constexpr data_type S = CPrevConfEx::start + CPrevConfEx::size;

    public:
        static constexpr data_type start = S;
        static constexpr data_type size = static_cast<data_type>((enumCap>1) ? sklib::bits_rank(enumCap-1) : 0);
    };

    template<class CData, class CPrevConf, auto enumCap>
    class bit_props_config_mask_helper //: public bit_props_config_placement_helper<CData, CPrevConf, enumCap>
    {
    private:
        typedef typename CData::data_type data_type;
        static constexpr data_type S = bit_props_config_placement_helper<CData, CPrevConf, enumCap>::start;
        static constexpr data_type L = bit_props_config_placement_helper<CData, CPrevConf, enumCap>::size;

    public:
        static constexpr data_type mask = static_cast<data_type>(sklib::bits_data_mask_v<data_type, L> << S);
    };
};

template<class CData, class CPrevConf, auto enumCap>
class bit_props_config_type
    : public bit_props_group_type<CData, sklib::priv::bit_props_config_mask_helper<CData, CPrevConf, enumCap>::mask>
    , public sklib::priv::bit_props_config_placement_helper<CData, CPrevConf, enumCap>
    , public sklib::priv::bit_props_config_anchor
{
    static_assert(std::is_base_of_v<sklib::priv::bit_props_anchor, CData>,
                  "First parameter of bit_props_config_type template must be caller specification of bit_props_data_type");
    static_assert(std::is_same_v<CPrevConf, void> || std::is_base_of_v<sklib::priv::bit_props_config_anchor, CPrevConf>,
                  "Second parameter of bit_props_config_type template must be either void "
                  "or previous declaration of bit_props_config_type");

private:
    typedef typename CData::data_type data_type;
    static constexpr data_type S = sklib::priv::bit_props_config_placement_helper<CData, CPrevConf, enumCap>::start;
    static constexpr data_type L = sklib::priv::bit_props_config_placement_helper<CData, CPrevConf, enumCap>::size;
    static constexpr data_type M = sklib::priv::bit_props_config_mask_helper<CData, CPrevConf, enumCap>::mask;

    static_assert(L <= sklib::bits_width_v<data_type>,
                  "Third parameter of bit_props_config_type template doesn\'t fit underlying data type");
    static_assert(S + L <= sklib::bits_width_v<data_type>,
                  "Collecting bit pack with bit_props_config_type: overall data set doesn\'t fit underlying data type");

public:
    constexpr bit_props_config_type(data_type x = data_type()) : bit_props_group_type<CData, M>((x << S) & M) {}

// need version that strips bits outside gMask
//    template<data_type gMask>
//    constexpr bit_props_config_type(bit_props_group_type<CData, gMask> X)
//        : bit_props_group_type<CData, gMask>((X. << S) & M) {}
};

