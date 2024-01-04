// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides crash-safe and throw-safe analogs of few std::string function.
// This is internal SKLib file and must NOT be included directly.

// Copies substring from start, at most length characters.
std::string safe_substring(const std::string& str, size_t start = 0, size_t length = std::string::npos) noexcept
{
    if (!length) return {};

    size_t srclen = str.length();
    if (srclen <= start) return {};

    return str.substr(start, ((length == std::string::npos) ? (srclen-start) : sklib::opaque::alt_min(srclen-start, length)));
}

