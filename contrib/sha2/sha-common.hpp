#pragma once

#include <stdint.h>

/*
 *  These constants hold size information for each of the SHA
 *  hashing operations
 */
enum {
//    SHA1_Message_Block_Size = 64,
    SHA224_Message_Block_Size = 64,
    SHA256_Message_Block_Size = 64,
    SHA384_Message_Block_Size = 128,
    SHA512_Message_Block_Size = 128,
    USHA_Max_Message_Block_Size = SHA512_Message_Block_Size,

//    SHA1HashSize = 20,
    SHA224HashSize = 28, SHA256HashSize = 32,
    SHA384HashSize = 48, SHA512HashSize = 64,
    USHAMaxHashSize = SHA512HashSize,

//    SHA1HashSizeBits = 160,
    SHA224HashSizeBits = 224, SHA256HashSizeBits = 256,
    SHA384HashSizeBits = 384, SHA512HashSizeBits = 512,
    USHAMaxHashSizeBits = SHA512HashSizeBits
};

/*
 *  This structure will hold context information for the SHA-256
 *  hashing operation.
 */
struct SHA256Context {
    uint32_t Intermediate_Hash[SHA256HashSize / 4]; /* Message Digest */

    uint64_t Length;               /* Message length in bits */

    int16_t Message_Block_Index;  /* Message_Block array index */
                                        /* 512-bit message blocks */
    uint8_t Message_Block[SHA256_Message_Block_Size];
};

/*
 *  This structure will hold context information for the SHA-512
 *  hashing operation.
 */
struct SHA512Context {
    uint64_t Intermediate_Hash[SHA512HashSize / 8]; /* Message Digest */
    uint64_t Length;   /* Message length in bits */                                         //SK: real-world sizes do not exceed 64 bits

    int16_t Message_Block_Index;  /* Message_Block array index */
                                        /* 1024-bit message blocks */
    uint8_t Message_Block[SHA512_Message_Block_Size];
};

/*
 *  This structure will hold context information for the SHA-224
 *  hashing operation.  It uses the SHA-256 structure for computation.
 */
typedef struct SHA256Context SHA224Context;

/*
 *  This structure will hold context information for the SHA-384
 *  hashing operation.  It uses the SHA-512 structure for computation.
 */
typedef struct SHA512Context SHA384Context;

/*
 *  This structure holds context information for all SHA
 *  hashing operations.
 */
typedef struct USHAContext {
    int whichSha;               /* which SHA is being used */
    union {
//        SHA1Context sha1Context;
        SHA224Context sha224Context; SHA256Context sha256Context;
        SHA384Context sha384Context; SHA512Context sha512Context;
    } ctx;
} USHAContext;

/* SHA-224 */
int SHA224Reset(SHA224Context*);
int SHA224Input(SHA224Context*, const uint8_t* bytes, unsigned int bytecount);
int SHA224FinalBits(SHA224Context*, uint8_t bits, unsigned int bit_count);
int SHA224Result(SHA224Context*, uint8_t Message_Digest[SHA224HashSize]);

/* SHA-256 */
int SHA256Reset(SHA256Context*);
int SHA256Input(SHA256Context*, const uint8_t* bytes, unsigned int bytecount);
int SHA256FinalBits(SHA256Context*, uint8_t bits, unsigned int bit_count);
int SHA256Result(SHA256Context*, uint8_t Message_Digest[SHA256HashSize]);

/* SHA-384 */
int SHA384Reset(SHA384Context*);
int SHA384Input(SHA384Context*, const uint8_t* bytes, unsigned int bytecount);
int SHA384FinalBits(SHA384Context*, uint8_t bits, unsigned int bit_count);
int SHA384Result(SHA384Context*, uint8_t Message_Digest[SHA384HashSize]);

/* SHA-512 */
int SHA512Reset(SHA512Context*);
int SHA512Input(SHA512Context*, const uint8_t* bytes, unsigned int bytecount);
int SHA512FinalBits(SHA512Context*, uint8_t bits, unsigned int bit_count);
int SHA512Result(SHA512Context*, uint8_t Message_Digest[SHA512HashSize]);

/* Unified SHA functions, chosen by whichSha */
int USHAReset(USHAContext* context, SHAversion whichSha);
int USHAInput(USHAContext* context, const uint8_t* bytes, unsigned int bytecount);
int USHAFinalBits(USHAContext* context, uint8_t bits, unsigned int bit_count);
int USHAResult(USHAContext* context, uint8_t Message_Digest[USHAMaxHashSize]);
int USHABlockSize(enum SHAversion whichSha);
int USHAHashSize(enum SHAversion whichSha);
int USHAHashSizeBits(enum SHAversion whichSha);
//const char* USHAHashName(enum SHAversion whichSha);

