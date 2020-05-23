/*
    based on projet https://github.com/Erriez/ErriezCRC32/
    src https://github.com/Erriez/ErriezCRC32/blob/master/src/ErriezCRC32.c
*/

#include <string.h>

#include "crc32.h"

uint32_t StringToCRC32(const char* message, size_t crc32_type){

    switch (crc32_type)
    {
    case CRC32:
        return toCRC32(message);
        break;
    case CRC32mpeg2:
        return toCRC32mpeg2(message);
        break;
    
    default:
        break;
    }
    return 0;
}

uint32_t toCRC32(const char* buffer){

    uint32_t mask;
    uint32_t crc = CRC32_INITIAL;
    size_t bufferLength = strlen(buffer);

    for (size_t i = 0; i < bufferLength; i++) {
        crc ^= ((uint8_t *)buffer)[i];
        for (int8_t j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLYNOMIAL & mask);
        }
    }

    return crc;
}

uint32_t toCRC32mpeg2(const char* message) {
   size_t i, j;
   unsigned int crc, msb;
   size_t l = strlen(message);

   crc = 0xFFFFFFFF;
   for(i = 0; i < l; i++) {
      // xor next byte to upper bits of crc
      crc ^= (((unsigned int)message[i])<<24);
      for (j = 0; j < 8; j++) {    // Do eight times.
            msb = crc>>31;
            crc <<= 1;
            crc ^= (0 - msb) & 0x04C11DB7;
      }
   }
   return crc;         // don't complement crc on output
}

/*

uint32_t StringToCRC32(const char *buffer, uint32_t crc32_type){
    return crc32Buffer(buffer, strlen(buffer), crc32_type);
}

uint32_t crc32Buffer(const void *buffer, size_t bufferLength, uint32_t crc32_type){

    switch (crc32_type)
    {
    case CRC32:
        //return ~toCRC32(buffer, bufferLength);
        return ~crc32Update(buffer, bufferLength, CRC32_INITIAL);
        break;
    case CRC32mpeg2:
            return ~toCRC32mpeg2(buffer, bufferLength);
        break;
    
    default:
        break;
    }
    //return ~toCRC32mpeg2(buffer, bufferLength);
    return ~crc32Update(buffer, bufferLength, CRC32_INITIAL);
}

uint32_t crc32Update(const void *buffer, size_t bufferLength, uint32_t crc)
{
    size_t i;
    int8_t j;
    uint32_t mask;

    for (i = 0; i < bufferLength; i++) {
        crc ^= ((uint8_t *)buffer)[i];
        for (j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLYNOMIAL & mask);
        }
    }

    return crc;
}

uint32_t crc32Final(uint32_t crc)
{
    return ~crc;
}

uint32_t toCRC32(const void *buffer, size_t bufferLength)
{
   size_t i, j;
   uint32_t crc, mask;

   crc = 0xFFFFFFFF;
    for (i = 0; i < bufferLength; i++) {          // Get next byte.
        crc ^= ((uint8_t *)buffer)[i];
        for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLYNOMIAL & mask);
        }
    }
   return crc;
}

uint32_t toCRC32mpeg2(const void *buffer, size_t bufferLength)
{
   size_t i, j;
   unsigned int crc, msb;

   crc = 0xFFFFFFFF;
   for(i = 0; i < bufferLength; i++) {
      // xor next byte to upper bits of crc
      crc ^= ((((uint8_t *)buffer)[i]) <<24);
      for (j = 0; j < 8; j++) {    // Do eight times.
            msb = crc>>31;
            crc <<= 1;
            crc ^= (0 - msb) & CRC32mpeg2_POLYNOMIAL;
      }
   }
   return crc;         // don't complement crc on output
}
*/