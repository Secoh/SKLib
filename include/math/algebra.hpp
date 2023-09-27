// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides certain algebraic objects and/or operations on them.
// This is internal SKLib file and must NOT be included directly.

// Euclidean domains
// Operators defined: addition, subtraction, multiplication, and representation according to Euclid's division lemma
// For each Euclidean type, the function sklib::edivrem() is defined: for A divided by B, get quotent, remainder
// For purpose of correction in certain operations, function sklib::supplement::e_isnegative() is introduced
// NB: In some Euclidean rings, there are no "negative" elements

#include "algebra/edom-int.hpp"             // support for regular C++ integers
#include "algebra/edom-signed-uint.hpp"     // "uint plus sign bit", maximum bit count for the data type

#include "algebra/bezout.hpp"               // GCD and Bezout coefficiets in Euclidean domain

// Fields
// Operators defined: addition, subtraction, multiplication, and division

#include "algebra/field-modular.hpp"        // modulo prime

// Misc

#include "algebra/pow.hpp"                  // exponentiation to integer power; positive only in ring; any in field

