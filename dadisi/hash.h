#ifndef _CRUSH_HASH_H
#define _CRUSH_HASH_H

#include<stdint.h>
#include "Distributor.h"

#define CRUSH_HASH_RJENKINS1   0

#define CRUSH_HASH_DEFAULT CRUSH_HASH_RJENKINS1

extern const char *crush_hash_name(int type);
/*
extern uint32_t crush_hash32(int type, uint32_t a);
extern uint32_t crush_hash32_2(int type, uint32_t a, uint32_t b);
extern uint32_t crush_hash32_3(int type, uint32_t a, uint32_t b, uint32_t c);
extern uint32_t crush_hash32_4(int type, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
extern uint32_t crush_hash32_5(int type, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
			    uint32_t e);
*/
uint64_t disthash(VDRIVE::Distributor *crush_dist, int64_t key1);
uint64_t disthash_2(VDRIVE::Distributor *crush_dist, int64_t key1, int64_t key2);
uint64_t disthash_3(VDRIVE::Distributor *crush_dist, int64_t key1, int64_t key2, int64_t key3);
uint64_t disthash_4(VDRIVE::Distributor *crush_dist, int64_t key1, int64_t key2, int64_t key3, int64_t key4);
uint64_t disthash_5(VDRIVE::Distributor *crush_dist, int64_t key1, int64_t key2, int64_t key3, int64_t key4, int64_t key5);


#endif
