/*
    based on projet https://github.com/Erriez/ErriezCRC32/
    src https://github.com/Erriez/ErriezCRC32/blob/master/src/ErriezCRC32.h
*/
#ifndef CRC32_H_
#define CRC32_H_

#include <inttypes.h> // uint32_t type
#include <stddef.h>   // size_t type

#ifdef __cplusplus
extern "C" {
#endif

#define CRC32_INITIAL           0xFFFFFFFFUL
#define CRC32_POLYNOMIAL        0xEDB88320UL
#define CRC32mpeg2_POLYNOMIAL   0x04C11DB7UL

enum {
    CRC32,
    CRC32mpeg2
};

/*
uint32_t StringToCRC32(const char *buffer, uint32_t crc32_type);
uint32_t crc32Buffer(const void *buffer, size_t bufferLength, uint32_t crc32_type);
uint32_t crc32Update(const void *buffer, size_t bufferLength, uint32_t crc);
uint32_t crc32Final(uint32_t crc);

uint32_t toCRC32(const void *buffer, size_t bufferLength);
*/
uint32_t StringToCRC32(const char* message, size_t crc32_type);

uint32_t toCRC32(const char* message);
uint32_t toCRC32mpeg2(const char* message);

#ifdef __cplusplus
}
#endif

#endif