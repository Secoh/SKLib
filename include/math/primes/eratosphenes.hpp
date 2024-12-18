// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides functions related to prime numbers search by the Sieve of Eratosphenes (brute force search)
// This is internal SKLib file and must NOT be included directly.

enum class eratosphenes_status_type
{
    prime=0, composite, error
};

// Given the Primes is the list of all prime numbers starting with 2 and up to some maximum = max(Primes),
// and the input value is a prime candidate, verify whether it is divisible to any available prime number.
// The integer type in template argument defines the bit size of the prime candidate being considered.
// Prime numbers in the list are 32-bit. The array index is also 32-bit. Index 0 corresponds to number 5.
// When the prime list is shorter than needed to verify the prime candidate, it is possible to extend
// search by dividing by the next prime candidates rather than primes, at expense of reduced performance.
// Maximum allowed range is taken from maximum prime gap among all 32-bit primes. (Delta index < 128.)
// While technically this function is constexpr, for the sake of sanity lets NOT make it constexpr.
SKLIB_TEMPLATE_IF_UINT(T)
inline eratosphenes_status_type eratosphenes(T Idx, const std::vector<uint32_t>& Primes, bool forgive_margin = false)
{
    // for the sake of argument, verify few useful primes and their indexes
    static_assert( 5 == prime_candidate<unsigned>(0), "Index of prime 5 is 0");
    static_assert( 7 == prime_candidate<unsigned>(1), "Index of prime 7 is 1");
    static_assert(11 == prime_candidate<unsigned>(2), "Index of prime 11 is 2");
    static_assert(13 == prime_candidate<unsigned>(3), "Index of prime 13 is 3");
    static_assert(17 == prime_candidate<unsigned>(4), "Index of prime 17 is 4");
    static_assert(19 == prime_candidate<unsigned>(5), "Index of prime 19 is 5");
    static_assert(23 == prime_candidate<unsigned>(6), "Index of prime 23 is 6");

    // until P=23 (index=6), all indexed candidates are primes
    if (Idx <= 6) return eratosphenes_status_type::prime;

    T value = prime_candidate<T>(Idx);
    uint32_t prime = 0;

    // array of primes always starts with 2, 3, 5, 7, ...
    // a prime candidate never divedes by 2 or 3, so the search shall start from number 5,
    // which position in the array is 2 (0-based)
    uint32_t N = (uint32_t)Primes.size();
    for (uint32_t k=2; k<N; k++)
    {
        prime = Primes[k];
        if (value % prime == 0) return eratosphenes_status_type::composite;
        if (value / prime <= prime) return eratosphenes_status_type::prime;
    }

    // Just in case, if the list of primes falls short, we can go past it by testing
    // prime candidates at the expense of reduced performance. The set of candidates
    // contains all primes. The search is limited to until next prime that was missing.
    // To do that, the index scans through the maximum prime index gap for 32-bit
    // primes, which is less than 128. The technique is useful for dealing with high
    // 64-bit primes, and for testing small numbers for primality without maintaining
    // the list of primes at all. (All way up to and including the prime 116683 -
    // so all 17-bit primes can be searched without reference array.)
    if (forgive_margin)
    {
        T Idx1 = (prime ? prime_candidate_to_index<T>(prime)+1 : 0);

        for (int k=0; k<128; k++, Idx1++)
        {
            T p = prime_candidate<T>(Idx1);
            if (value % p == 0) return eratosphenes_status_type::composite;
            if (value / p <= p) return eratosphenes_status_type::prime;
        }
    }

    // not enough test primes and/or candidates to determine primality of the input
    return eratosphenes_status_type::error;
}

SKLIB_TEMPLATE_IF_UINT(T)
inline eratosphenes_status_type eratosphenes_general(T value, const std::vector<uint32_t>& Primes, bool forgive_margin = false)
{
    if (value % 2 == 0 || value % 3 == 0) return eratosphenes_status_type::composite;
    return eratosphenes(prime_candidate_to_index(value), Primes, forgive_margin);
}

inline bool is_eratosphenes_status_error(eratosphenes_status_type code)
{ return (code == eratosphenes_status_type::error); }

inline bool is_eratosphenes_status_prime(eratosphenes_status_type code)
{ return (code == eratosphenes_status_type::prime); }

//sk: need to formally prove that anything not divisible by 2 and by 3 is the prime candidate
// provide "article" for extended desctiption/explanation of the technique

//sk2: have to prove "if (value / p <= p)" stop condition

