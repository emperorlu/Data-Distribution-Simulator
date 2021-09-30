/* 
 * File:   Prime.h
 * Author: ywkang
 *
 * Created on February 27, 2010, 7:55 PM
 */

#ifndef _PRIME_H
#define	_PRIME_H
#include "Distributor.h"

class Prime {
public:
    Prime();
    Prime(const Prime& orig);
    virtual ~Prime();
    static int getSize() { return 10000; }
    static int64_t getPrimeNumberByIndex(int32_t index);
    static int64_t getPrimeNumber(double hashvalue) ;
    static int getLowestPrimeIndex(int32_t largest_sub_cluster);
    static int64_t getPrimeNumberIntHash(uint64_t hash);
private:
    static int64_t primes[];
};

#endif	/* _PRIME_H */

