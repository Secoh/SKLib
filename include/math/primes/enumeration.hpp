// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides functions related to prime numbers enumeration
// This is internal SKLib file and must NOT be included directly.

// While the prime numbers are 2,3,5,7,11..., many applications can take advantege of the fact
// that all primes except 2 and 3 can be represented in form 6n+-1, where n is non-negative integer (0 and up).
// Sequence 6n+-1 contains all primes, but it also contains composites, thus "a prime candidate"
// has to be tested for primality. Note that there is no need to verify divisors 2 and 3.
// There is reversible function that represens a member of the sequence by its 0-based index.

// Sequence 6n+-1 in ascending order, starting with 5, by 0-based index
SKLIB_TEMPLATE_IF_UINT(T) constexpr T prime_candidate(sklib::supplement::do_not_deduce<T> idx)
{
    return 3*idx + 5 - (idx & 1);
}

// Reversal of prime_candidate() function. Actual prime numbers 5 and above can be indexed, too.
// For numbers that are not prime candidates, returns index of closest greater prime candidate.
SKLIB_TEMPLATE_IF_UINT(T) constexpr T prime_candidate_to_index(T p)
{
    return (p - 2 - (p & 1)) / 3;
}

SKLIB_TEMPLATE_IF_UINT(T)
struct prime_candidate_index_cap
{
    static constexpr T value = sklib::supplement::bits_data_mask<T>();
};

SKLIB_TEMPLATE_IF_UINT(T)
using prime_candidate_index_cap_v = prime_candidate_index_cap<T>::value;

