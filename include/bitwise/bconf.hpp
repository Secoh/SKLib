// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides functions: bits flip, hamming distance, integer rank
// This is internal SKLib file and must NOT be included directly.

namespace opaque
{
    struct bit_props_anchor {};
    struct bit_props_config_anchor {};
};

template<class Anchor, class T>
class bit_props_data_type : public Anchor, public sklib::opaque::bit_props_anchor
{
    static_assert(sklib::is_integer_v<T>, "Second parameter of bit_props_data_type template must be integer");

public:
    typedef T data_type;
    const T data{};

protected:
    constexpr bit_props_data_type(T x) : data(x) {}
};

template<class CData, CData::data_type gMask, SKLIB_INTERNAL_ENABLE_IF_DERIVED(CData, sklib::opaque::bit_props_anchor)>
class bit_props_group_type : public CData
{
protected:
    constexpr bit_props_group_type(CData::data_type x) : CData(x & gMask) {}

public:  // for completeness, return itself in bit_props_group flavor
    constexpr auto operator+ () const { return *this; }
    constexpr auto get() const { return *this; }
};

namespace opaque
{
    template<class CData, CData::data_type gMask>
    struct bit_props_group_helper : public bit_props_group_type<CData, gMask>
    {
        constexpr bit_props_group_helper(CData::data_type x) : bit_props_group_type<CData, gMask>(x) {}
        constexpr auto get() const { return static_cast<bit_props_group_type<CData, gMask>>(*this); }
    };
};

template<class CData, CData::data_type gMask1, CData::data_type gMask2,
    SKLIB_INTERNAL_ENABLE_IF_CONDITION((std::is_base_of_v<sklib::opaque::bit_props_anchor, CData>) && !(gMask1 & gMask2))>
constexpr auto operator+ (bit_props_group_type<CData, gMask1> X, bit_props_group_type<CData, gMask2> Y)
{
    return sklib::opaque::bit_props_group_helper<CData, (gMask1 | gMask2)>(X.data | Y.data).get();
}

namespace opaque
{
    template<class T>
    struct bit_props_dummy_config_type
    {
        static constexpr T start = 0;
        static constexpr T size = 0;
    };

    template<class CData, class CPrevConf, auto enumCap>
    class bit_props_config_helper
    {
    private:
        typedef typename CData::data_type data_type;
        typedef std::conditional_t<std::is_base_of_v<CData, CPrevConf>,
            CPrevConf, bit_props_dummy_config_type<data_type>> CPrevConfEx;
        static constexpr data_type S = CPrevConfEx::start + CPrevConfEx::size;

    public:
        static constexpr data_type start = S;
        static constexpr data_type size = static_cast<data_type>((enumCap>1) ? sklib::bits_rank(enumCap-1) : 0);
        static constexpr data_type mask = static_cast<data_type>(sklib::bits_data_mask_v<data_type, size> << start);
    };
};

template<class CData, class CPrevConf, auto enumCap>
class bit_props_config_type
    : public bit_props_group_type<CData, sklib::opaque::bit_props_config_helper<CData, CPrevConf, enumCap>::mask>
    , public sklib::opaque::bit_props_config_helper<CData, CPrevConf, enumCap>
    , public sklib::opaque::bit_props_config_anchor
{
    static_assert(std::is_base_of_v<sklib::opaque::bit_props_anchor, CData>,
                  "First parameter of bit_props_config_type template must be caller specification of bit_props_data_type");
    static_assert(std::is_same_v<CPrevConf, void> || std::is_base_of_v<sklib::opaque::bit_props_config_anchor, CPrevConf>,
                  "Second parameter of bit_props_config_type template must be either void "
                  "or previous declaration of bit_props_config_type");

private:
    typedef typename CData::data_type data_type;
    static constexpr data_type S = sklib::opaque::bit_props_config_helper<CData, CPrevConf, enumCap>::start;
    static constexpr data_type L = sklib::opaque::bit_props_config_helper<CData, CPrevConf, enumCap>::size;
    static constexpr data_type M = sklib::opaque::bit_props_config_helper<CData, CPrevConf, enumCap>::mask;

    static_assert(L <= sklib::bits_width_less_sign_v<data_type>,
                  "Third parameter of bit_props_config_type template doesn\'t fit underlying data type");
    static_assert(S + L <= sklib::bits_width_less_sign_v<data_type>,
                  "Collecting bit pack with bit_props_config_type: overall data set doesn\'t fit underlying data type");

public:
    constexpr bit_props_config_type(data_type x = data_type()) : bit_props_group_type<CData, M>((x << S) & M) {}
};



/////////// delete later


/* example


struct my_anchor {};
using my_props_data = sklib::bit_props_data_type<my_anchor, int>;
template<class Prev, auto enumCap> using my_props_elem = sklib::bit_props_config_type<my_props_data, Prev, enumCap>;

struct fruit_anchor {};
using fruit_props_data = sklib::bit_props_data_type<fruit_anchor, int>;
template<class Prev, auto enumCap> using fruit_props_elem = sklib::bit_props_config_type<fruit_props_data, Prev, enumCap>;

// --

class collection
{
private:
enum { baud_9k, baud_1k, baud_2k, baud_cap };
enum { data_8, data_6, data_7, data_cap };
enum { stop_1, stop_2, stop_cap };

enum fruit { apple, orange, plum, peach, banana, lemon, fruit_cap };
enum color { nocolor, red, yellow, green, blue, brown, color_cap };
enum taste { notaste, sweet, salty, spicy, sour, taste_cap };
enum shape { noshape, sphere, cube, pyramid, shape_cap };

public:
typedef my_props_elem<void,       baud_cap> baud_field;
typedef my_props_elem<baud_field, data_cap> data_field;
typedef my_props_elem<data_field, stop_cap> stop_field;

static constexpr baud_field Baud1200 {baud_1k};
static constexpr baud_field Baud2400 {baud_2k};
static constexpr baud_field Baud9600 {baud_9k};

static constexpr data_field Data6 {data_6};
static constexpr auto Data7 = +data_field{data_7};
static constexpr data_field Data8 {data_8};

static constexpr stop_field Stop1 {stop_1};
static constexpr stop_field Stop2 {stop_2};

typedef fruit_props_elem<void,        fruit_cap> what_fruit;
typedef fruit_props_elem<what_fruit,  color_cap> fruit_color;
typedef fruit_props_elem<fruit_color, taste_cap> fruit_taste;
typedef fruit_props_elem<fruit_taste, shape_cap> fruit_shape;

static constexpr what_fruit Apple  {apple};
static constexpr auto Orange = what_fruit{orange} + fruit_shape{}; // example: prohibit Shape when Orange is used
static constexpr what_fruit Peach  {peach};
static constexpr what_fruit Banana {banana};
static constexpr what_fruit Lemon  {lemon};

static constexpr fruit_color Red  {red};
static constexpr fruit_color Yellow  {yellow};
static constexpr fruit_color Green  {green};
static constexpr fruit_color Blue  {blue};
static constexpr auto Brown = fruit_color{brown}.get();

static constexpr fruit_taste Sweet  {sweet};
static constexpr fruit_taste Salty  {salty};
static constexpr fruit_taste Spicy  {spicy};
static constexpr fruit_taste Sour  {sour};

static constexpr fruit_shape Sphere  {sphere};
static constexpr fruit_shape Cube  {cube};
static constexpr fruit_shape Pyramid  {pyramid};

};



*/