#include"SHA.h"

/* -----------------------------------------------------------------------
 * SHA1
 *
 * Last modified: Christian Scheideler, Aug. 10 2002
 *
 * Description:
 * This function gets a 512 bit field via Message_Block and returns an
 * unsigned 32 bit integer via Result. (For this only variable A is used.
 * Using variables B,C,D,E, in the future also larger values may be returned.)
 * The return value of the function indicates whether there was an error (>0).
 *
 * The implementation of this function is based on IETF RFC 3174.
 */
uint8_t SHA1Block (uint8_t object_type, uint8_t object_copy, uint8_t level,
                   uint32_t seed, uint32_t object_id, uint8_t *Message_Block)
{
  uint8_t i;

  if (!Message_Block)
    return shNull;

  /* Initialize Message_Block */
  for (i=0; i<64; i++)
     Message_Block[i] = 0;

  Message_Block[0] = object_type;
  Message_Block[1] = object_copy;
  Message_Block[2] = level;
  Message_Block[3] = (uint8_t) ((seed & 0xFF000000) >> 24);
  Message_Block[4] = (uint8_t) ((seed & 0x00FF0000) >> 16);
  Message_Block[5] = (uint8_t) ((seed & 0x0000FF00) >> 8);
  Message_Block[6] = (uint8_t)  (seed & 0x000000FF);
  Message_Block[7] = (uint8_t) ((object_id & 0xFF000000) >> 24);
  Message_Block[8] = (uint8_t) ((object_id & 0x00FF0000) >> 16);
  Message_Block[9] = (uint8_t) ((object_id & 0x0000FF00) >> 8);
  Message_Block[10] = (uint8_t) (object_id & 0x000000FF);

  return shSuccess;
}

/* -----------------------------------------------------------------------
 * SHA1
 *
 * Last modified: Sascha Effert
 *
 * Description:
 * This function gets a 512 bit field via Message_Block and returns an
 * unsigned 64 bit integer via Result. (For this only variable A and B are used.
 * Using variables C,D,E, in the future also larger values may be returned.)
 * The return value of the function indicates whether there was an error (>0).
 *
 * The implementation of this function is based on IETF RFC 3174.
 */

uint8_t SHA1 (uint8_t *Message_Block, uint64_t *Result)
{
    const uint32_t K[] =  {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                          };
    int           t;                 /* Loop counter                */
    uint32_t      temp;              /* Temporary word value        */
    uint32_t      W[80];             /* Word sequence               */
    uint32_t      A, B, C, D, E;     /* Word buffers                */
    uint64_t      tmpResult;

    if (!Message_Block || !Result)
      return shNull;

    /* Initialize W */
    for(t = 0; t < 16; t++)
    {
        W[t] = Message_Block[t * 4] << 24;
        W[t] |= Message_Block[t * 4 + 1] << 16;
        W[t] |= Message_Block[t * 4 + 2] << 8;
        W[t] |= Message_Block[t * 4 + 3];
    }
    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    /* Initialize the word buffers A..E */
    A = 0x67452301;
    B = 0xEFCDAB89;
    C = 0x98BADCFE;
    D = 0x10325476;
    E = 0xC3D2E1F0;
    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    tmpResult = A;
    tmpResult <<= 32;
    tmpResult = tmpResult | B;
    *Result = tmpResult;
    return shSuccess;
}
