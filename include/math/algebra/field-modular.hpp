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

    typedef sklib::priv::uint_extend<T> TT;
    typedef sklib::signed_uint<T> TS;

    void mmul(T x)
    {
        if (P<2)
        {
            sklib::priv::uint_extend_t<T> U(V);
            U.mul(x).div(P, &V);
        }
        else
        {
            err = true;
        }
    }

public:
    constexpr modp(T prime, T value = 0) : P(prime), V(value % std::max(prime, T(1))), err(prime<2) {}
    constexpr modp(const modp&) = default;
    constexpr modp() = default;

    bool is_valid() const { return (!err && P); }

    bool is_nonzero() const { return (is_valid() && V); }
    explicit operator bool() const { return is_nonzero(); }

    modp reciprocal() const
    {
        if (is_nonzero())
        {
            TS kP, kV;
            auto d = bezout(TS(P), TS(V), kP, kV);
            if (d == 1 && kV) return { P, ((kV.sign()<0) ? T(P-kV.abs()) : kV.abs()) };
        }

        return { 0, 0 }; // it also makes err=true of the returned object
    }

    modp& operator*= (const modp& X)
    {
        if (X.err) err = true;
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

    friend modp operator/ (const modp& X, const modp& Y) { return X * Y.reciprocal(); }
    friend modp operator/ (const modp& X, T Y) { return X * modp(P, Y).reciprocal(); }
    friend modp operator/ (T X, const modp& Y) { return X * Y.reciprocal(); }

    T operator() () const { return V; }

//    T& operator=(T value)
//    {
//        V = (T % P);
//    }
};

// -------------------------------------------------------------------------------
// Special field: A + B * sqrt(Q) mod P, where P - prime, Q - coprime with P,
// and sum and product are formal in the sense (A mod P) + sqrt(Q) * (B mod P)
// It is a ring, but it is also a field when Euler criterium is satisfied:
// Q^{P-1 \over 2} + 1 = 0 \mod P
//
SKLIB_TEMPLATE_IF_UINT(T)
class modp_duo
{
private:
    modp A, B;
    T Q;

public:
    constexpr modp(T prime, T value = 0) : P(prime), V(value% std::max(prime, T(1))), err(prime < 2) {}
    constexpr modp(const modp&) = default;
    constexpr modp() = default;

};

