// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides the basic Vector algebra.
// This is internal SKLib file and must NOT be included directly.

#define SKLIB_INTERNAL_MATH_VECTOR_FOREACH for (unsigned i=0; i<N; i++)

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fname,op_compound_token,op_binary_token) \
private:                                                                            \
static constexpr void fname (T *dest, const T *src)                                 \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH dest[i] op_compound_token src[i];            \
}                                                                                   \
static constexpr void fname##alt (T *dest, const T *src)                            \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH dest[i] = dest[i] op_binary_token src[i];    \
}                                                                                   \
public:                                                                             \
auto& operator op_compound_token (const element_wise_type& X)                       \
{                                                                                   \
    return fname(data, X.copy.data), thisVect();                                    \
}                                                                                   \
friend constexpr auto operator op_binary_token (Vect A, const element_wise_type& B) \
{                                                                                   \
    return fname##alt(A.data, B.copy.data), A;                                      \
}                                                                                   \
friend constexpr auto operator op_binary_token (element_wise_type A, const Vect& B) \
{                                                                                   \
    return fname##alt(A.copy.data, B.data), A.copy;                                 \
}                                                                                   \
friend constexpr auto operator op_binary_token (element_wise_type A, const element_wise_type& B) \
{                                                                                   \
    return fname##alt(A.copy.data, B.copy.data), A.copy;                            \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(fname,op_token)          \
private:                                                                            \
static constexpr void fname##unary (T *data)                                        \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH data[i] = op_token data[i];                  \
}                                                                                   \
public:                                                                             \
friend constexpr auto operator op_token (element_wise_type X)                       \
{                                                                                   \
    return fname##unary(X.copy.data), X.copy;                                       \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(fname,op_token)             \
SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(fname,op_token);                 \
friend constexpr auto operator op_token (Vect X)                                    \
{                                                                                   \
    return fname##unary(X.data), X;                                                 \
}

#define SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(fscale,op_compound_token,op_binary_token) \
private:                                                                            \
static constexpr void fscale (T *data, T coeff)                                     \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH data[i] op_compound_token coeff;             \
}                                                                                   \
static constexpr void fscale##alt (T *data, T coeff)                                \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH data[i] = data[i] op_binary_token coeff;     \
}                                                                                   \
static constexpr void fscale##alt (T coeff, T *data)                                \
{                                                                                   \
    SKLIB_INTERNAL_MATH_VECTOR_FOREACH data[i] = coeff op_binary_token data[i];     \
}                                                                                   \
public:                                                                             \
auto& operator op_compound_token (T coeff)                                          \
{                                                                                   \
    return fscale(data, coeff), thisVect();                                         \
}                                                                                   \
friend constexpr auto operator op_binary_token (Vect X, T coeff)                    \
{                                                                                   \
    return fscale##alt(X.data, coeff), X;                                           \
}                                                                                   \
friend constexpr auto operator op_binary_token (element_wise_type X, T coeff)       \
{                                                                                   \
    return fscale##alt(X.copy.data, coeff), X.copy;                                 \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ENSURE_VALID_CRTP                                \
static_assert(std::is_base_of_v<vect_impl, Vect>,                                   \
"SKLIB ** INTERNAL ERROR ** Target class must be derived from sklib::opaque::vect_impl");

namespace opaque
{
    // using Curiously Recurring Template Pattern idiom
    // see, for example https://en.cppreference.com/w/cpp/language/crtp
    template<class Vect, unsigned N, class T>
    class vect_impl
    {
    protected:
        T data[N] = {};

        constexpr Vect& thisVect()
        {
            SKLIB_INTERNAL_MATH_VECTOR_ENSURE_VALID_CRTP;
            return static_cast<Vect&>(*this);
        }
        constexpr const Vect& thisVect() const
        {
            SKLIB_INTERNAL_MATH_VECTOR_ENSURE_VALID_CRTP;
            return static_cast<const Vect&>(*this);
        }

        template<class Source>
        constexpr void load_array(const Source& input)
        { SKLIB_INTERNAL_MATH_VECTOR_FOREACH { data[i] = input[i]; } }

    public:
        constexpr vect_impl()                   = default;
        constexpr vect_impl(const vect_impl&)   = default;
        vect_impl& operator= (const vect_impl&) = delete;
        ~vect_impl()                            = default;
        // the "move" context is deliberately left undefined -  for PODs, identical to copying

        struct element_wise_type
        {
            Vect copy{};
            constexpr explicit element_wise_type(const Vect& content) : copy(content) {}
            ~element_wise_type() = default;
            element_wise_type& operator= (const element_wise_type&) = delete;
        };

        constexpr element_wise_type operator~ () const { return element_wise_type(thisVect()); }

        // for completeness
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(ftilde, ~);

        // common element-wise vector operations
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(f_plus, +);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(fminus, -);

        // common element-wise vector operations
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fadd, +=, +);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fsub, -=, -);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fmul, *=, *);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fdiv, /=, /);

        // potentially valid for some types
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fmod, %=, %);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fshl, <<=, <<);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fshr, >>=, >>);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fand, &=, &);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(f_or, |=, |);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(fxor, ^=, ^);

        // vector-specific operations - will reuse helped functions defined above

        // addition
        auto& operator+= (const Vect& X)                        { return fadd(data, X.data), thisVect(); }
        friend constexpr auto operator+ (Vect A, const Vect& B) { return faddalt(A.data, B.data), A; }

        // subtraction
        auto& operator-= (const Vect& X)                        { return fsub(data, X.data), thisVect(); }
        friend constexpr auto operator- (Vect A, const Vect& B) { return fsubalt(A.data, B.data), A; }

        // scalar product
        friend constexpr auto operator* (const Vect& A, const Vect& B)
        {
            auto sum = T();
            SKLIB_INTERNAL_MATH_VECTOR_FOREACH { sum += A.data[i] * B.data[i]; }
            return sum;
        }

        // length of vector
        constexpr auto abs() const  { return std::sqrt(thisVect() * thisVect()); }
        // normalized vector
        constexpr auto norm() const { return thisVect() / abs(); }

        // angle (radians)
        friend constexpr auto operator^ (const Vect& A, const Vect& B)
        { return std::acos(std::clamp((A * B) / (A.abs() * B.abs()), -1., 1.)); }

        // scale
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(smul, *=, *);
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(sdiv, /=, /);
        friend constexpr auto operator* (T coeff, Vect X)              { return smulalt(coeff, X.data), X; }
        friend constexpr auto operator* (T coeff, element_wise_type X) { return smulalt(coeff, X.copy.data), X.copy; }
    };

    template<unsigned N, class T, SKLIB_INTERNAL_ENABLE_IF_CONDITION(N>3)>
    class vect_impl_N : public vect_impl<vect_impl_N<N,T>, N, T>
    {
    public:
        constexpr vect_impl_N()                   = default;
        constexpr vect_impl_N(const vect_impl_N&) = default;
        ~vect_impl_N()                            = default;

        constexpr vect_impl_N(const std::array<T, N>& input) { this->load_array(input); }
        vect_impl_N& operator= (const vect_impl_N& input)    { return this->load_array(input.data), *this; }
    };

    template<class T>
    class vect_impl_2 : public vect_impl<vect_impl_2<T>, 2, T>
    {
    public:
        // vector component, rvalue or lvalue
        constexpr T& X()      { return this->data[0]; }
        constexpr T& Y()      { return this->data[1]; }
        constexpr T X() const { return this->data[0]; }
        constexpr T Y() const { return this->data[1]; }

        constexpr vect_impl_2()                   = default;
        constexpr vect_impl_2(const vect_impl_2&) = default;
        ~vect_impl_2()                            = default;

        constexpr vect_impl_2(const std::array<T, 2>& input) { this->load_array(input); }
        constexpr vect_impl_2(T x, T y)                      { this->load_array(std::array<T, 2>{x, y}); }
        vect_impl_2& operator= (const vect_impl_2& input)    { return this->load_array(input.data), *this; }

        friend constexpr T operator% (const vect_impl_2& A, const vect_impl_2& B)
        { return A.X() * B.Y() - A.Y() * B.X(); }
    };

    template<class T>
    class vect_impl_3 : public vect_impl<vect_impl_3<T>, 3, T>
    {
    public:
        // vector component, rvalue or lvalue
        constexpr T& X()      { return this->data[0]; }
        constexpr T& Y()      { return this->data[1]; }
        constexpr T& Z()      { return this->data[2]; }
        constexpr T X() const { return this->data[0]; }
        constexpr T Y() const { return this->data[1]; }
        constexpr T Z() const { return this->data[2]; }

        constexpr vect_impl_3()                   = default;
        constexpr vect_impl_3(const vect_impl_3&) = default; //: vect_impl<vect_impl_3<T>, 3, T>(input) {}
        ~vect_impl_3()                            = default;

        constexpr vect_impl_3(const std::array<T, 3>& input) { this->load_array(input); }
        constexpr vect_impl_3(T x, T y, T z)                 { this->load_array(std::array<T, 3>{x, y, z}); }
        vect_impl_3& operator= (const vect_impl_3& input)    { return this->load_array(input.data), *this; }

        constexpr vect_impl_3& operator%= (const vect_impl_3& U)
        {
            T tmpX = Y()*U.Z() - Z()*U.Y();
            T tmpY = Z()*U.X() - X()*U.Z();
            Z() = X()*U.Y() - Y()*U.X();
            X() = tmpX;
            Y() = tmpY;
            return *this;
        }

        friend constexpr vect_impl_3 operator% (vect_impl_3 A, const vect_impl_3& B) { return A %= B; }
    };

}; // namespace opaque

#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW
#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY
#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY
#undef SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY
#undef SKLIB_INTERNAL_MATH_VECTOR_FOREACH
#undef SKLIB_INTERNAL_MATH_VECTOR_ENSURE_VALID_CRTP

template<unsigned N, class T = double>
using Vect =
    std::conditional_t<std::less<int>{}(N, 3),
        std::conditional_t<std::less<int>{}(N, 2), T, sklib::opaque::vect_impl_2<T> >,
        std::conditional_t<std::less<int>{}(N, 4), sklib::opaque::vect_impl_3<T>, sklib::opaque::vect_impl_N<N, T> > >;

using Vect2d = sklib::opaque::vect_impl_2<double>;
using Vect3d = sklib::opaque::vect_impl_3<double>;

// useful stuff

inline Vect3d Spherical(double R, double Fi, double Th)
{
    auto rsth = R * sin(Th);
    return { rsth*cos(Fi), rsth*sin(Fi), R*cos(Th) };
}

