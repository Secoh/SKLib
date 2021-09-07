#pragma once

namespace sklib_internal
{

#ifdef SKLIB_CHECKSUM_LINK_CRC_TABLES  /* NB: preprocessor define is reused with opposite meaning */
    static constexpr  /* visible directly to all the compilation unit */
#else
    const             /* hidden from end user, must be linked */
#endif
    uint16_t checksum_crc16_table[checksum_crc_table_size] = 

