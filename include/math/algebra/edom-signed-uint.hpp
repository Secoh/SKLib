// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides signed integer functionality using maximum bit range (at performance cost).
// This is internal SKLib file and must NOT be included directly.

// Rationale. While signed int64 is in fact 63-bit integer, using undelying uint64_t + extra variable for sign
// allows for full 64-bit range and also ability to negate values (needed to support algebraic rings)
// Remark. Does not monitor overflows
SKLIB_TEMPLATE_IF_UINT(T) class signed_uint
{
private:
    bool P = true;
    T V = 0;

    void add_self(T v2, bool p2)
    {
        if (!::sklib::bool_xor(P, p2)) V += v2;
        else if (V > v2) V -= v2;
        else
        {
            P = !P;
            V = v2 - V;
        }
    }

public:
    typedef T data_type;

//    template<class T1, std::enable_if_t<SKLIB_TYPES_IS_SIGNED_INTEGER(T1), bool> = true>
    SKLIB_TEMPLATE_IF_SINT(T1)
    constexpr signed_uint(T1 x) : P(x>=0), V(::sklib::abs(x)) {}

//    template<class T1, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T1), bool> = true>
    SKLIB_TEMPLATE_IF_UINT(T1)
    constexpr signed_uint(T1 x, bool s = true) : P(s), V(x) {}

    constexpr signed_uint(const signed_uint& X) = default;
    constexpr signed_uint() = default;

    // assignment operator is made up by C++ compiler using constructors
    // arithmetic operators with numbers are made up also with constructors

    T abs() const { return V; }
    int sign() const { return (V ? (P ? 1 : -1) : 0); }

    signed_uint& operator += (const signed_uint& X) { return (add_self(X.V, X.P), *this); }
    signed_uint& operator -= (const signed_uint& X) { return (add_self(X.V, !X.P), *this); }

    friend signed_uint<T> operator+ (signed_uint<T> X, const signed_uint<T>& Y) { return (X += Y); }
    friend signed_uint<T> operator- (signed_uint<T> X, const signed_uint<T>& Y) { return (X -= Y); }
    friend signed_uint<T> operator- (signed_uint<T> X) { return ((X.P = !X.P), X); }

    signed_uint& operator *= (const signed_uint& X)
    {
        V *= X.V;
        P = !::sklib::bool_xor(P, X.P);
        return *this;
    }

    friend signed_uint<T> operator* (signed_uint<T> X, const signed_uint<T>& Y) { return (X *= Y); }

    //sk TODO: comparison operators; explicit bool()


};

SKLIB_TEMPLATE_IF_UINT(T)
signed_uint<T> edivrem(const signed_uint<T>& A, const signed_uint<T>& B, signed_uint<T>* R = nullptr)
{
    auto x = B.abs();
    if (x)
    {
        auto q = sklib::implementation::uidivrem(A.abs(), x, x);
        bool s = (B.sign() > 0);

        if (A.sign() < 0)
        {
            q++;
            s = !s;
            x = B.abs() - x;
        }

        if (R) *R = x;
        return { q, s };
    }

    if (R) *R = 0;
    return {};
}

