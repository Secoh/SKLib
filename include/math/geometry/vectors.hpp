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

#ifdef SKLIB_MATH_VECTOR_DEBUG_CON
#define SKLIB_INTERNAL_MATH_VECTOR_CONSTEXPR
#define SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS(params_str,op_str,next_op_str) \
std::cout << "N=" << N << "; Call " << op_str << " ("<< params_str << ") -> " << next_op_str << (*next_op_str ? "\n" : "");
#define SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(what) \
for (unsigned i=0; i<N; i++) std::cout << (i?",":"") << (what).data[i]; std::cout << "\n";
#else
#define SKLIB_INTERNAL_MATH_VECTOR_CONSTEXPR constexpr
#define SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS(params_str,op_str,next_op_str)
#define SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(what)
#endif

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(op_compound_token,op_binary_token) \
auto& operator op_compound_token (const element_wise_type& src)                        \
{   \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("EW",#op_compound_token, "");                                                                                        \
    auto src_inner = src.get_vect();                                        \
    for (unsigned i=0; i<N; i++) data[i] op_compound_token src_inner.data[i];               \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);         \
    return thisVect();                                                                           \
}                                                                                           \
friend auto operator op_binary_token (Vect A, const element_wise_type& B) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("Vec,EW", #op_binary_token, #op_compound_token);                                                                                        \
    return A op_compound_token B; \
}                   \
friend auto operator op_binary_token (const element_wise_type& A, const Vect& B) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("EW,Vec", #op_binary_token, #op_compound_token);                                                                                        \
    return A.copy_vect() op_binary_token ~B; \
} \
friend auto operator op_binary_token (const element_wise_type& A, const element_wise_type& B) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("EW,EW", #op_binary_token, #op_compound_token);                                                                                        \
    return A.copy_vect() op_binary_token B; \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(op_token)    \
friend auto operator op_token (const element_wise_type& src)       \
{                                                                       \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("EW", #op_token, "copy");    \
    auto copy = src.copy_vect();                                        \
    for (unsigned i=0; i<N; i++) copy.data[i] = op_token copy.data[i];  \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(copy); \
    return copy;                                                        \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(op_token)  \
SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(op_token);      \
friend auto operator op_token (const Vect& src) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("Vec", #op_token, #op_token "(~)");    \
    return op_token (~src); \
}

#define SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(op_compound_token,op_binary_token)        \
auto& operator op_compound_token (T k)                                                     \
{                                                                                               \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("T", #op_compound_token, "");    \
    for (unsigned i=0; i<N; i++) data[i] op_compound_token k;                                   \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this); \
    return thisVect();                                                                               \
}                                                                                               \
friend auto operator op_binary_token (Vect A, T k) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("Vec,T", #op_binary_token, #op_compound_token);    \
    return A op_compound_token k; \
}  \
friend auto operator op_binary_token (const element_wise_type& A, T k) \
{ \
    SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("EW,T", #op_binary_token, #op_compound_token);    \
    return A.copy_vect() op_binary_token k; \
}

namespace opaque
{
    // using Curiously Recurring Template Pattern idiom
    // see, for example https://en.cppreference.com/w/cpp/language/crtp
    template<class Vect, unsigned N, class T>
    class vect_impl
    {
    protected:
        T data[N] = {};

        Vect&       thisVect()       { return static_cast<Vect&>(*this); }
        const Vect& thisVect() const { return static_cast<const Vect&>(*this); }

        template<class Source>
        auto& load_array(const Source& input)
        {
            for (unsigned i=0; i<N; i++) data[i] = input[i];
            return thisVect();
        }

    public:
        vect_impl()                       = default;    // normal predefined transformations
        vect_impl(const vect_impl& input) = default;
        vect_impl(vect_impl&& input) noexcept = default;
        ~vect_impl()                      = default;

        vect_impl& operator= (const vect_impl& input) = delete; //default;
        vect_impl& operator= (vect_impl&& input)      = delete; //default;

// do we need it at all?
//        vect_impl            (const std::array<T, N>& input)  { load_array(input); }
//        vect_impl& operator= (const std::array<T, N>& input)  { return load_array(input); }

        class element_wise_type
        {
        protected:
            const Vect &mirror;

        public:
            explicit element_wise_type(const Vect& parent) : mirror(parent)
            {
                SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("const Vect&", "EW constructor", "");
                SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(mirror);
            }
            const Vect& get_vect() const { return mirror; }
            Vect copy_vect() const { return mirror; }
        };

        const element_wise_type operator~ () const
        {
            return element_wise_type(thisVect());
        }

        // for completeness
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(~);

        // common element-wise vector operations
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(+);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(-);

        // common element-wise vector operations
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(+=,+);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(-=,-);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(*=,*);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(/=,/);

        // potentially valid for some types
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(%=,%);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(<<=, <<);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(>>=, >>);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(&=,&);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(|=,|);
        SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(^=,^);

        // vector-specific operations

        // addition
        auto& operator+= (const Vect& src) { return operator+= (~src); }
        friend auto operator+ (Vect A, const Vect& B) { return A += B; }

        // subtraction
        auto& operator-= (const Vect& src) { return operator-= (~src); }
        friend auto operator- (Vect A, const Vect& B) { return A -= B; }

        // scalar product
        friend auto operator* (const Vect& A, const Vect& B)
        {
            auto sum = T();
            for (unsigned i=0; i<N; i++) sum += A.data[i] * B.data[i];
            return sum;
        }

        // length
        auto abs() const
        {
            const Vect& V = thisVect();
            return std::sqrt(V * V);
        }
        auto norm() const { return thisVect() / abs(); }

        // angle (radians)
        friend auto operator^ (const Vect& A, const Vect& B)
        { return std::acos( (A * B) / (A.abs() * B.abs()) ); }

        // scale
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(*=,*);
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(/=,/);
        friend auto operator* (T k, Vect A) { return A * k; }
        friend auto operator* (T k, const element_wise_type& A) { return A * k; }
    };

    template<unsigned N, class T, SKLIB_INTERNAL_ENABLE_IF_CONDITION(N>3)>
    class vect_impl_N : public vect_impl<vect_impl_N<N,T>, N, T>
    {
    };

    template<class T>
    class vect_impl_2 : public vect_impl<vect_impl_2<T>, 2, T>
    {
    public:
        T& X = this->data[0];
        T& Y = this->data[1];

        vect_impl_2()                         = default;
        vect_impl_2(const vect_impl_2& input) = default;
        vect_impl_2(vect_impl_2&& input)      = default;
        ~vect_impl_2()                        = default;

        vect_impl_2(const std::array<T, 2>& input) : vect_impl<2, T>(input) {}
        vect_impl_2(T x, T y, T z)                 : vect_impl<2, T>({x, y, z}) {}

        vect_impl_2& operator= (const vect_impl_2& input) { this->load_array(input.data); return *this; }
        vect_impl_2& operator= (vect_impl_2&& input) = default;

        friend T operator% (const vect_impl_2& A, const vect_impl_2& B) { return A.X * B.Y - A.Y * B.X; }
    };

    template<class T>
    class vect_impl_3 : public vect_impl<vect_impl_3<T>, 3, T>
    {
    private:
        static constexpr int N = 3;

    public:
        T& X = this->data[0];
        T& Y = this->data[1];
        T& Z = this->data[2];

        vect_impl_3()  = default;
        ~vect_impl_3() = default;

        vect_impl_3(const vect_impl_3& input) : vect_impl<vect_impl_3<T>, 3, T>(input)
        {
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("const Vect&", "constructor", "base constructor");
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);
        }

        vect_impl_3(vect_impl_3&& input) noexcept : vect_impl<vect_impl_3<T>, 3, T>(input)
        {
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("Vect&&", "constructor", "base constructor");
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);
        }

        vect_impl_3(const std::array<T, 3>& input)
        {
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("const array&", "constructor", "load");
            this->load_array(input);
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);
        }

        vect_impl_3(T x, T y, T z)
        {
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("XYZ", "constructor", "load");
            this->load_array(std::array<T, 3>{x, y, z});
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);
        }

        vect_impl_3& operator= (const vect_impl_3& input)
        {
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("const Vect&", "operator=", "load");
            this->load_array(input.data);
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_SHOW(*this);
            return *this;
        }

#ifndef SKLIB_MATH_VECTOR_DEBUG_CON
        vect_impl_3& operator= (vect_impl_3&& input) = default;
#else
        vect_impl_3& operator= (vect_impl_3&& input)
        {
            const vect_impl_3& persist = input;
            SKLIB_INTERNAL_MATH_VECTOR_DEBUG_CALLS("Vect&&", "operator=", "load");
            this->load_array(persist.data);
            return *this;
        }
#endif

        vect_impl_3& operator%= (const vect_impl_3& src)
        {
            T tmpX = Y*src.Z - Z*src.Y;
            T tmpY = Z*src.X - X*src.Z;
            Z = X*src.Y - Y*src.X;
            X = tmpX;
            Y = tmpY;
            return *this;
        }

        friend vect_impl_3 operator% (vect_impl_3 A, const vect_impl_3& B) { return A %= B; }
    };

}; // namespace opaque

#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW
#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY
#undef SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY
#undef SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY

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

