// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides misc. helper types and/or classes.
// This is internal SKLib file and must NOT be included directly.

namespace supplement
{
// -------------------------------------------------------
// Provides member type for a class depending on class declaration, so the name of the type is known

#define SKLIB_SUPPLEMENT_DECLARE_TYPE_CONDUIT(type_conduit_name,type_reflection_name) \
template<class T> class type_conduit_name { protected: typedef T type_reflection_name; };

    SKLIB_SUPPLEMENT_DECLARE_TYPE_CONDUIT(type_conduit, self_type)

// Example how to use
// -----------------------
// Class X receives "self" member type regardless of T:
// template<class T> class X : public type_conduit<X<T>>
//
// Class Myclass receives member type with predefined name, same as provided to its base class:
// #define MYCLASS(tname) class Myclass : public X<tname>, public type_conduit<tname>
// 
// Reference: https://stackoverflow.com/questions/21143835/can-i-implement-an-autonomous-self-member-type-in-c

// -------------------------------------------------------
// Lightweight simplified callback facility - can call functions: global, C-style with void* payload, or with class pointer payload
// Can be used as an "almost" drop-in replacement for a function address variable, with initialization, test for nullptr, and calling
// NB: Must use CA=int when declaring callback to function wihout payload

    template<class CA, class FR, class ...FA>
    class callback_type
    {
    protected:
        typedef void (*cfunc_type)();                // pointer to non-member function (or static class member)

        typedef FR (*cfunc_self_type)(CA*, FA...);    // "normal" self-addressing callback
        typedef FR (*cfunc_bare_type)(FA...);         // simplified callback, good for single IO point
        typedef FR (*cfunc_void_type)(void*, FA...);  // general C-style callback with payload

        union { void* const descr = nullptr;  // pointer to external descriptor,
                CA*   const root;  };         // or pointer to parent class, used mutually exclusively

        const cfunc_type func = nullptr;      // address of end-user callback - routed from member call() according to init options

        static FR routing_self(const callback_type* self, FA... args) { return ((cfunc_self_type)(self->func))(self->root, args...); }
        static FR routing_bare(const callback_type* self, FA... args) { return ((cfunc_bare_type)(self->func))(args...); }
        static FR routing_void(const callback_type* self, FA... args) { return ((cfunc_void_type)(self->func))(self->descr, args...); }
        static FR routing_dead(const callback_type* /*self*/, FA... /*args*/) { return FR(); }

        FR (* const reflector)(const callback_type*, FA...) = routing_dead;  // internal static function redirects call to requested parameters formatting

    public:
        explicit callback_type(cfunc_self_type f, CA* parent) : func((cfunc_type)f), reflector(routing_self), root(parent) {}
        explicit callback_type(cfunc_bare_type f)             : func((cfunc_type)f), reflector(routing_bare)               {}
        explicit callback_type(cfunc_void_type f, void* ext)  : func((cfunc_type)f), reflector(routing_void), descr(ext)   {}

        bool is_valid()     const { return func != nullptr; }                                  // verify whether constructor was called with invalid function address
        FR call(FA... args) const { return (is_valid() ? reflector(this, args...) : FR()); }   // make call to the "programmable" callback

        explicit operator bool()  const { return is_valid(); }
        FR operator()(FA... args) const { return call(args...); }
    };

// for completeness, simplified callback classthat only does C-style callbacks
    template<class FR, class ...FA>
    class c_callback_type : public callback_type<int, FR, FA...>
    {
    protected:
        typedef FR (*cfunc_bare_type)(FA...);         // lets just copy that from the base class declaration
        typedef FR (*cfunc_void_type)(void*, FA...);  // to avoid declaring them "public" (namespace pollution)
    public:
        explicit c_callback_type(cfunc_bare_type f) : callback_type<int, FR, FA...>(f) {}
        explicit c_callback_type(cfunc_void_type f, void* ext) : callback_type<int, FR, FA...>(f, ext) {}
    };

//sk: on how to make deducible function arguments from function address type, in template - for future reference
//        template<class CA, class FN> struct callback_type;
//        template<class CA, class FR, class ...FA>
//        struct callback_type<CA, FR(*)(FA...)>        { ... };

}; // supplement

