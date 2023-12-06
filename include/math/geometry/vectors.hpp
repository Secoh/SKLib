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

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_BINARY(op_compound_token,op_binary_token) \
vect_impl& operator op_compound_token (const element_wise_type& src)                        \
{                                                                                           \
    auto src_inner = src.get_vect();                                                        \
    for (unsigned i=0; i<N; i++) data[i] op_compound_token src_inner.data[i];               \
    return *this;                                                                           \
}                                                                                           \
friend vect_impl operator op_binary_token (vect_impl A, const element_wise_type& B) { return A op_compound_token B; }                   \
friend vect_impl operator op_binary_token (const element_wise_type& A, const vect_impl& B) { return A.copy_vect() op_binary_token ~B; } \
friend vect_impl operator op_binary_token (const element_wise_type& A, const element_wise_type& B) { return A.copy_vect() op_binary_token B; }

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(op_token)    \
friend vect_impl operator op_token (const element_wise_type& src)       \
{                                                                       \
    auto copy = src.copy_vect();                                        \
    for (unsigned i=0; i<N; i++) copy.data[i] = op_token copy.data[i];  \
    return copy;                                                        \
}

#define SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY(op_token)  \
SKLIB_INTERNAL_MATH_VECTOR_ELEMWISE_OPDEF_UNARY_EW(op_token);      \
friend vect_impl operator op_token (const vect_impl& src) { return op_token (~src); }

#define SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(op_compound_token,op_binary_token)        \
vect_impl& operator op_compound_token (T k)                                                     \
{                                                                                               \
    for (unsigned i=0; i<N; i++) data[i] op_compound_token k;                                   \
    return *this;                                                                               \
}                                                                                               \
friend vect_impl operator op_binary_token (vect_impl A, T k) { return A op_compound_token k; }  \
friend vect_impl operator op_binary_token (const element_wise_type& A, T k) { return A.copy_vect() op_binary_token k; }

namespace opaque
{
    template<unsigned N, class T>
    class vect_impl
    {
    protected:
        T data[N] = {};

        template<class Source>
        void load_array(const Source& input)
        {
            for (unsigned i=0; i<N; i++) data[i] = input[i];
        }

    public:
        vect_impl() = default;
        vect_impl(const std::array<T, N>& input) { load_array(input); }
        vect_impl(const vect_impl& input)        { load_array(input.data); }

        vect_impl& operator= (const vect_impl& input)        { load_array(input.data); }
        vect_impl& operator= (const std::array<T, N>& input) { load_array(input); }

        class element_wise_type
        {
        protected:
            const vect_impl &mirror;

        public:
            explicit element_wise_type(const vect_impl& parent) : mirror(parent) {}
            const vect_impl& get_vect() const { return mirror; }
            vect_impl copy_vect() const { return mirror; }
        };

        const element_wise_type operator~ () const { return (const element_wise_type)(*this); }

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
        vect_impl& operator+= (const vect_impl& src) { return operator+= (~src); }
        friend vect_impl operator+ (vect_impl A, const vect_impl& B) { return A += B; }

        // subtraction
        vect_impl& operator-= (const vect_impl& src) { return operator-= (~src); }
        friend vect_impl operator- (vect_impl A, const vect_impl& B) { return A -= B; }

        // scalar product
        friend T operator* (const vect_impl& A, const vect_impl& B)
        {
            auto sum = T();
            for (unsigned i=0; i<N; i++) sum += A.data[i] * B.data[i];
            return sum;
        }

        // length
        T abs() const { return std::sqrt(*this * *this); }
        vect_impl norm() const { return *this / abs(); }

        // angle (radians)
        friend double operator^ (const vect_impl& A, const vect_impl& B)
        { return std::acos( (A * B) / (A.abs() * B.abs()) ); }

        // scale
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(*=,*);
        SKLIB_INTERNAL_MATH_VECTOR_SCALE_OPDEF_BINARY(/=,/);
        friend vect_impl operator* (T k, vect_impl A) { return A * k; }
        friend vect_impl operator* (T k, const element_wise_type& A) { return A * k; }
    };

    template<class T>
    class vect_impl_2 : public vect_impl<2, T>
    {
    public:
        T& X = this->data[0];
        T& Y = this->data[1];

        vect_impl_2() = default;
        vect_impl_2(const std::array<T, 2>& input) : vect_impl<2, T>(input) {}
        vect_impl_2(const vect_impl_2& input)      : vect_impl<2, T>(input) {}
        vect_impl_2(T x, T y, T z)                 : vect_impl<2, T>({x, y, z}) {}

        friend T operator% (const vect_impl_2& A, const vect_impl_2& B) { return A.X * B.Y - A.Y * B.X; }
    };

    template<class T>
    class vect_impl_3 : public vect_impl<3, T>
    {
    public:
        T& X = this->data[0];
        T& Y = this->data[1];
        T& Z = this->data[2];

        vect_impl_3() = default;
        vect_impl_3(const std::array<T, 3>& input) : vect_impl<3, T>(input) {}
        vect_impl_3(const vect_impl_3& input)      : vect_impl<3, T>(input) {}
        vect_impl_3(T x, T y, T z)                 : vect_impl<3, T>({x, y, z}) {}

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

    //    typedef typename vect_impl<3, T>::element_wise_type elem_wise_3;
    //    vect_impl_3& operator%= (const elem_wise_3& src) { return operator%= (src.get_vect()); }
    //    friend vect_impl_3 operator% (vect_impl_3 A, const elem_wise_3& B) { return A %= B; }
    //    friend vect_impl_3 operator% (const elem_wise_3& A, const vect_impl_3& B) { return A.copy_vect() % B; }
    //    friend vect_impl_3 operator% (const elem_wise_3& A, const elem_wise_3& B) { return A.copy_vect() % B; }
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
        std::conditional_t<std::less<int>{}(N, 4), sklib::opaque::vect_impl_3<T>, sklib::opaque::vect_impl<N, T> > >;

using Vect2d = sklib::opaque::vect_impl_2<double>;
using Vect3d = sklib::opaque::vect_impl_3<double>;

