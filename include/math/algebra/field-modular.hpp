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

    typedef sklib::opaque::uint_extend<T> TT;
    typedef sklib::signed_uint<T> TS;

    void mmul(T x)
    {
        sklib::opaque::uint_extend_t<T> U(V);
        U.mul(x).div(P, &V);
    }

public:
    modp(T prime, T value = 0) : P(prime), V(value % std::max(prime, T(1))), err(!prime) {}
    modp(const modp&) = default;
    modp() = default;

    bool is_valid() const { return !err; }

    modp reciprocal()
    {
        if (!err)
        {
            TS kP, kV;
            auto d = bezout(TS(P), TS(V), kP, kV);
            if (d == 1 && kV) return { P, ((kV.sign()<0) ? T(P-kV.abs()) : kV.abs()) };
        }

        return { 0, 0 };
    }

    modp& operator*= (const modp& X)
    {
        mmul(X.V);
        return *this;
    }

    modp& operator*= (T x)
    {
        mmul(x);
        return *this;
    }

    friend modp operator* (modp X, const modp& Y) { return X *= Y; }
    friend modp operator* (modp X, T Y) { return X *= Y; }
    friend modp operator* (T X, modp Y) { return Y *= X; }

    T operator() () const { return V; }

//    T& operator=(T value)
//    {
//        V = (T % P);
//    }
};

