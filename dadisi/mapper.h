#ifndef _CRUSH_MAPPER_H
#define _CRUSH_MAPPER_H

/*
 * CRUSH functions for find rules and then mapping an input to an
 * output set.
 *
 * LGPL2
 */

#include "crush.h"
#include "Distributor.h"

extern int crush_find_rule(struct crush_map *map, int pool, int type, int size);
int crush_do_rule(VDRIVE::Distributor *dist, struct crush_map *map,
		  int ruleno, int64_t x, int *result, int result_max,
		  int force, uint32_t *weight);


#endif
