// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_UTILITY_TST_RNDSZINT_HPP
#define SKLIB_INCLUDED_UTILITY_TST_RNDSZINT_HPP

#include "../configure.hpp"
#ifdef SKLIB_TARGET_TEST

#include <type_traits>
#include <random>

namespace sklib {

// Provides helper for random integers / random range for testing.
// This is internal SKLib file and must NOT be included directly.

namespace priv
{
    template<class T, int N_down, typename = void>
    class random_size_integer_device_unsigned
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Random size integers are available only for fundamental types");
    };

    template<class T, int N_down>
    class random_size_integer_device_unsigned<T, N_down, std::enable_if_t<is_native_integer_v<T>>>
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
}; // namespace priv

template<class T, int N_down>
using random_size_integer_device = std::conditional_t<sklib::is_signed_v<T>,
    sklib::priv::random_size_integer_device_signed<T, N_down>,
    sklib::priv::random_size_integer_device_unsigned<T, N_down>>;

}; // namespace sklib

#endif // SKLIB_TARGET_TEST

#endif // SKLIB_INCLUDED_UTILITY_TST_RNDSZINT_HPP

