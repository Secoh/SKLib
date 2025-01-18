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

#ifndef SKLIB_INCLUDED_CRYPTO_HPP
#define SKLIB_INCLUDED_CRYPTO_HPP

#include <algorithm>
//#include <string>

#include "bitwise.hpp"

// for the purpose of data mangling, "page" refers to array of octets (bytes) having size OCTET_ADDRESS_SPAN

namespace sklib
{
    namespace aux
    {
        // synonym for the span addressed by a byte
//sk        inline constexpr unsigned crypto_page_size = (unsigned)OCTET_ADDRESS_SPAN;
#define SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) for (unsigned k=0; k<(unsigned)OCTET_ADDRESS_SPAN; k++)
#define SKLIB_INTERNAL_CRYPTO_PAGE_SIZE ((unsigned)OCTET_ADDRESS_SPAN)

        // Fill a page (or a substitution function) by a constant
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_init(uint8_t* target, uint8_t defval = 0)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[k] = defval;
        }

        // Copy a page (or a substitution function) to other memory location
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_copy(uint8_t* target, const uint8_t* source)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[k] = source[k];
        }

        // Compare pages (or a substitution functions)
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        bool crypto_page_is_equal(const uint8_t* A, const uint8_t* B)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) if (A[k] != B[k]) return false;
            return true;
        }

        // Compare pages (or a substitution functions)
        // Return true if and only if A < B
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        bool crypto_page_is_less(const uint8_t* A, const uint8_t* B)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) if (A[k] >= B[k]) return false;
            return true;
        }

        // Compare page (or a substitution functions) to a constant
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        bool crypto_page_is_constant(const uint8_t* A, uint8_t testval = 0)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) if (A[k] != testval) return false;
            return true;
        }

        // Make "Unity" substitution A:page -> page, such as A(x)=x for any x
        // Complexity: linear
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_substitution_make_unity(uint8_t* target)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[k] = (uint8_t)k;
        }

        // Given a page with arbitrary data in source array (S), create
        // bijective mapping A:page -> page as deterministic function of S:
        // A=A(S). A is represented as an array (page) in tagret.
        // Source array is NOT modified in the process
        // Complexity: linear
        // Limitations: target and source must be different areas in memory
        // Guarantee: no memory allocations and no exceptions
        // NB: decomposition of A may consist of more than one loop
        void crypto_page_substitution_from_set(uint8_t* target, const uint8_t* source)
        {
            crypto_page_substitution_make_unity(target);

            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(p)
            {
                auto q = p + source[p] % (SKLIB_INTERNAL_CRYPTO_PAGE_SIZE-p);
                if (p!=q) std::swap(target[p], target[q]);
            }
        }

        // Given a page with arbitrary data in source array (S), create
        // bijective mapping A:page -> page as deterministic function of S:
        // A=A(S), with additional condition: A is equivalent to a single loop.
        // A is represented as an array (page) in tagret.
        // (!) Source array is modified in the process
        // Complexity: linear
        // Limitations: target and source must be different areas in memory
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_substitution_single_loop_from_set(uint8_t* target, uint8_t* source)
        {
            crypto_page_substitution_from_set(target, source);
            crypto_page_copy(source, target);

            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[source[k]] = source[(k+1) % SKLIB_INTERNAL_CRYPTO_PAGE_SIZE];
        }

        // Given the substitution A:page -> page in source, determine whether A
        // is bijective. Return true if so, otherwise false.
        // Needs the scratch array for temporary page
        // Complexity: linear
        // Limitations: source and scrath must be different areas in memory
        // Guarantee: no memory allocations and no exceptions
        bool crypto_page_substitution_is_bijective(const uint8_t* source, uint8_t* scratch)
        {
            const uint8_t testv = 1;
            crypto_page_init(scratch);
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) scratch[source[k]] = testv;  // if source is bijective, all points in scratch are visited
            return crypto_page_is_constant(scratch, testv);
        }

        // Given the bijective substitution A:page -> page in source,
        // creage A reversal - substitution B such A(B(x)) = x for all x in page
        // Complexity: linear
        // Limitations: target and source must be different areas in memory;
        // source must be bijective substitution (not tested)
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_substitution_inverse(uint8_t* target, const uint8_t* source)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[source[k]] = (uint8_t)k;
        }

        // Given 2 substitutions A and B:page -> page in source_outer, source_inner,
        // creage "product" substitution as target(x) = source_outer(source_inner(x)) for any x
        // Complexity: linear
        // Limitations: target must be different area of memory than source(s)
        // Guarantee: no memory allocations and no exceptions
        void crypto_page_substitution_product(uint8_t* target, const uint8_t* source_outer, const uint8_t* source_inner)
        {
            SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL(k) target[k] = source_outer[source_inner[k]];
        }

#undef SKLIB_INTERNAL_CRYPTO_PAGE_FOR_ALL
#undef SKLIB_INTERNAL_CRYPTO_PAGE_SIZE
    };
};

#endif // SKLIB_INCLUDED_CRYPTO_HPP
