/* 
 * File:   helper.h
 * Author: fermat
 *
 * Created on 26. März 2010, 16:03
 */

#ifndef _HELPER_H
#define	_HELPER_H
//#define no_sqlite

#include <stdint.h>
#include<cstring>
#include <stdint.h>
#include <gmpxx.h>
#ifndef no_sqlite
#include <sqlite3.h>
#endif

int64_t readInt64_t(const char* value, int8_t base);

uint64_t readUint64_t(const char* value, int8_t base);

/**
 * read a signed GMP Integer value and put it in a 64 bit signed value.
 *
 * @param value The GMP value
 *
 * @return the 64-bit value
 */
int64_t read_int64_t(mpq_class value);

/**
 * read a unsigned GMP Integer value and put it in a 64 bit unsigned value.
 *
 * @param value The GMP value
 *
 * @return the 64-bit value
 */
uint64_t read_uint64_t(mpz_class value);

/**
 * create a new GMP integer value out of the 64 bit value (GMP only
 * supports 32 Bit values)
 *
 * @param num The divident of the rational value
 *
 * @return the rational value
 */
mpz_class create_mpz(int64_t num);

/**
 * create a new GMP rational value out of 2 64 bit values (GMP only
 * supports 32 Bit values)
 *
 * @param num The divident of the rational value
 * @param den The devisor of the rational value
 *
 * @return the rational value
 */
mpq_class create_mpq(int64_t num, int64_t den);

/**
 * create a new GMP rational value out of the 64 bit value (GMP only
 * supports 32 Bit values)
 *
 * @param num The divident of the rational value
 *
 * @return the rational value
 */
mpq_class create_mpq_n(int64_t num);


#ifndef no_sqlite
sqlite3 *loadDB(std::string filename);

void execQuery(sqlite3 *db, std::string queryStr);

sqlite3 *createDB(std::string filename);
#endif
#endif	/* _HELPER_H */

