// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides basic operations for modular arithmetics, including division.
// This is internal SKLib file and must NOT be included directly.

SKLIB_TEMPLATE_IF_UINT(T)
class modp
{
private:
    T P = 0;
    T V = 0;
    bool err = false;

    typedef sklib::implementation::uint_extend<T> T_ex;

    modp& copy(T_ex value)
    {
        if (P) V = value % P;
        else err = true;
    }

public:
    modp(T prime, T value) : P(prime), V(value) {}
    modp(const modp&) = default;
    modp() = default;

    T& operator=(T value)
    {
        V = (T % P);
    }
};

