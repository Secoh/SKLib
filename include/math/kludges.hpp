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

// This is internal SKLib file and must NOT be included directly.

namespace internal
{
    // workaround for notorious windows.h definition
    template<class T>
    constexpr auto alternate_maximum(const T& a, const T& b)
    {
        return (a < b ? b : a);
    }

    // workaround for notorious windows.h definition
    template<class T>
    constexpr auto alternate_minimum(const T& a, const T& b)
    {
        return (a < b ? a : b);
    }
};

// test function dearly missed in STL, esp. for integers, shall go away when implemented
template<class T>
constexpr bool is_clamped(const T& x, const T& low, const T& high)
{
    return (x >= low && x <= high);
}

// test function dearly missed in STL, esp. for integers, shall go away when implemented
template<class T>
constexpr bool is_clamped_cap(const T& x, const T& low, const T& cap)
{
    return (x >= low && x < cap);
}

template<class T, std::enable_if_t<SKLIB_TYPES_IS_SIGNED_INTEGER(T) || SKLIB_TYPES_IS_FLOATING_POINT(T), bool> = true>
constexpr T abs(const T& what)
{
    return ((what < 0) ? -what : what);
}

// helper for random integers / random range for testing
namespace implementation
{
    template<class T, int N_down>
    class random_size_integer_device_unsigned
    {
    private:
        // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
        std::random_device rd;
        std::mt19937 gen { rd() };
        std::uniform_int_distribution<T> distrib;

    public:
        T operator() ()
        {
            auto shift = int(distrib(gen) % N_down);
            return distrib(gen) >> shift;
        }
    };

    template<class T, int N_down>
    class random_size_integer_device_signed
    {
    private:
        random_size_integer_device_unsigned<std::make_unsigned_t<T>, N_down> uRND;

    public:
        T operator() ()
        {
            auto V = uRND();
            bool pos = (V & 1);
            T V1 = T(V >> 1);
            return (pos ? V1 : -V1);
        }
    };
}; // namespace implementation

template<class T, int N_down, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>
using random_size_integer_device = std::conditional_t<std::is_signed_v<T>,
    sklib::implementation::random_size_integer_device_signed<T, N_down>,
    sklib::implementation::random_size_integer_device_unsigned<T, N_down>>;

