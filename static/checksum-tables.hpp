// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#ifndef SKLIB_INCLUDED_CHECKSUM_TABLES_HPP
#define SKLIB_INCLUDED_CHECKSUM_TABLES_HPP

namespace sklib_internal
{

#ifdef SKLIB_CHECKSUM_LINK_CRC_TABLES  /* NB: preprocessor define is reused with opposite meaning */
    static constexpr  /* visible directly to all the compilation unit */
#else
    const             /* hidden from end user, must be linked */
#endif
    uint16_t checksum_crc16_table[checksum_crc_table_size] = 




#endif // SKLIB_INCLUDED_CHECKSUM_TABLES_HPP
