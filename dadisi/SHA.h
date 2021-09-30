/* 
 * File:   SHA.h
 * Author: fermat
 *
 * Created on 31. März 2010, 17:50
 */

#ifndef _SHA_H
#define	_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdint.h>

#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

    enum {
        shSuccess = 0,
        shNull /* Null pointer parameter */
    };

    uint8_t SHA1Block(uint8_t object_type, uint8_t object_copy, uint8_t level,
            uint32_t seed, uint32_t object_id, uint8_t *Message_Block);

    uint8_t SHA1(uint8_t *Message_Block, uint64_t *Result);

#ifdef __cplusplus
}
#endif

#endif	/* _SHA_H */

