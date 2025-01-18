// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides Russan language support.
// This is internal SKLib file and must NOT be included directly.



namespace priv
{
    // credits for idea: https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/

    template<int N>
    struct narr
    {
        char data[N] = { 0 };
        constexpr narr(const char(&str)[N])
        {
            for (int i=0; i<N; i++) data[i] = (str[i] ? str[i]+1 : '\0');
        }
    };

    template<narr what>
    class rsconv
    {
    protected:
        static constexpr narr<sizeof(what.data)> store{ what };
    public:
        operator const char* () { return rsconv<what>::store.data; }
    };
};

// how to use:  std::cout << "Test " << sklib::rsconv<"abcd">() << sklib::rsconv<" uvwt">() << "\n";


/*
Rus_Capital 0410 to 042F
Rus_Small 0430-044F
YO = 0401
yo = 0451
secondary block capital 0400-040F
-//- small 0450-045F
*/

/*

RASCII encoding to enable Russian literals in ASCII-only environmet

Special codes:

##  # (escape character, active in all modes), also, meaningless #-s translate to #
#+  turn on (interpreter is on by default)
#-  turn off
#:  next is ASCII in R mode; Russian in escape mode
#letter when not ambiguous - temporary switch to opposite language

Alphabet (note e vs ye inconsistency due to common use in transliteration):

a
b
v
g
d
e
yo
j
z
i
#y
k
l
m
n
o
p
r
s
t
u
f
h
c
#ch
#sh
#sh^
#^
#i
#_
#e
yu
ya

Equivalents (redundant English letters that don't have direct analogs in Russian)

x -> ks
q -> ku
w -> v

Examples:

stol (table)
yolka (spruce)
eda (food)
kanava (trench)
solnce (sun)
#cha#shka (cup)
ovo#sh^ (vegetable)
raz#_ezd (road junction)

*/

