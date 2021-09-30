/* 
 * File:   DistRedundantShare.h
 * Author: fermat
 *
 * Created on 20. Januar 2010, 17:39
 */

#ifndef _DISTREDUNDANTSHARE_H
#define	_DISTREDUNDANTSHARE_H

#include"RSDisk.h"
#include"Distributor.h"
#include "RSCache.h"
#include<iostream>
#include<fstream>
#include<map>
#include<set>
#include<tr1/unordered_map>
#include<sys/time.h>

#ifndef USE_P_CACHE
//#define USE_P_CACHE
#endif

#ifndef USE_R_CACHE
//#define USE_R_CACHE
#endif

#ifndef USE_R2_CACHE
//#define USE_R2_CACHE
#endif

namespace VDRIVE {

    /**
     * Implementation of the RedundantShare algorithm.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class DistRedundantShare : public Distributor {
    public:

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a DistRedundantShare.
         */
        DistRedundantShare(xercesc::DOMElement* data);

        /**
         * generate a new, uninitialized Redundant Share Implementation.
         */
        DistRedundantShare(int argc, char** argv);

        /**
         * copy constructor
         */
        DistRedundantShare(const DistRedundantShare& orig);

        /**
         * Destructor
         */
        virtual ~DistRedundantShare();

        /**
         * @see Distributor::placeExtent
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;

        /**
         * @see Distributor::setConfiguration
         */
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies);

        /**
         * @see Distributor::setDisks
         */
        virtual void setDisks(std::list<Disk*>* disks);

        /**
         * @see Distributor::toXML
         */
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const;

        /**
         * Get the Root-Type of XML-Elements representing this class.
         *
         * @return the Root-Type of XML-Elements representing this class.
         */
        static std::string getXMLRootType() {
            return std::string("RedundantShare");
        }

        /**
         * @see Distributor::getDisks
         */
        virtual std::list<Disk*>* getDisks() const;

        /**
         * @see Distributor::getExtentsize
         */
        virtual int64_t getExtentsize() const {
            return extentsize;
        }

        /**
         * @see Distributor::getCopies
         */
        virtual int32_t getCopies() const {
            return copies;
        }

        RSDisk** getRSDisks() {
            return disks;
        }

        int32_t getNumDisks() {
            return numDisks;
        }

    private:

        /**
         * The number of copies returned by each call of placeExtent.
         */
        int32_t copies;

        /**
         * The size of each extent passed to this Distributor
         */
        int64_t extentsize;

        /**
         * Tha alligned capacity of all Disks
         */
        int64_t capacity;

        /**
         * if true die disks with the same size will be sorted by decresing IDs
         */
        bool revertIDOrder;

        /**
         * RS needs some additional data to be hold for each Disk. This is hold
         * in this array.
         */
        RSDisk** disks;

        /**
         * Number of entries in the array disks.
         */
        int32_t numDisks;

        /**
         * File to print debug messages to.
         */
        std::ofstream debugfile;

        /**
         * cache with the values we dice the Extents against
         */
        uint64_t** hashValues;

        /**
         * cache with the cStar-Values we dice against.
         */
        uint64_t* errorValues;

        /**
         * Disks we hold an errorValue (cStar) for.
         */
        int64_t* errorDisks;

        /**
         * initialize the value caches. Has to be called after the configuration changed (so in setDisks and in the XML-Constructor).
         */
        void initValueCaches();

#ifdef USE_P_CACHE
        /**
         * Implementation of a cache to be used while initialisation. This can
         * get VERY HUGE.
         */
        //std::tr1::unordered_map<int32_t,std::tr1::unordered_map<int32_t,std::tr1::unordered_map<int32_t,mpq_class*>*>*>* p_cache;
        std::map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>* p_cache;

        /**
         * get the given P out of the cache.
         */
        mpq_class* getP(int32_t a, int32_t b, int32_t l);

        /**
         * search the best matching P out of Cache to calculate the wanted
         * value.
         */
        std::pair<const int32_t, mpq_class*>* getBestP(int32_t a, int32_t b, int32_t l);

        /**
         * Add a new P value to cache.
         */
        void setP(int32_t a, int32_t b, int32_t l, mpq_class* val);

        /**
         * clear the whole P cache
         */
        void clearP();
#endif

#ifdef USE_R_CACHE
        /**
         * cache the results of the r_inner function in here
         */
        RSCache *r_cache;

        void initR() {
            r_cache = RSCache::createInstance((int32_t) 4);
        }

        mpq_class* getR(int32_t o, int32_t m, int32_t j, int32_t i) {
            int32_t* query = new int32_t[4];
            query[0] = o;
            query[1] = m;
            query[2] = j;
            query[3] = i;
            mpq_class* result = r_cache->getValue(query);
            delete[] query;
            return result;
        }

        void setR(int32_t o, int32_t m, int32_t j, int32_t i, mpq_class& val) {
            int32_t* query = new int32_t[4];
            query[0] = o;
            query[1] = m;
            query[2] = j;
            query[3] = i;
            r_cache->setValue(query, val);
            delete[] query;
        }

        void clearR() {
            delete r_cache;
        }
#endif

#ifdef USE_R2_CACHE
        /**
         * cache the results of the r_inner function in here
         */
        RSCache *r2_cache;

        void initR2() {
            r2_cache = RSCache::createInstance((int32_t) 4);
        }

        mpq_class* getR2(int32_t o, int32_t m, int32_t j, int32_t i) {
            int32_t* query = new int32_t[4];
            query[0] = o;
            query[1] = m;
            query[2] = j;
            query[3] = i;
            mpq_class* result = r2_cache->getValue(query);
            delete[] query;
            return result;
        }

        void setR2(int32_t o, int32_t m, int32_t j, int32_t i, mpq_class& val) {
            int32_t* query = new int32_t[4];
            query[0] = o;
            query[1] = m;
            query[2] = j;
            query[3] = i;
            r2_cache->setValue(query, val);
            delete[] query;
        }

        void clearR2() {
            delete r2_cache;
        }
#endif
        /**
         * place a single copy over the given range of disks (used in
         * placeExtent)
         */
        int32_t placeCopy(int64_t position, int32_t copy, int32_t firstDisk) const;

        /**
         * For Redundant Share (and in fact for any fair k-Distribution) there
         * is a upper bound for how heteregeneous a disk may be. The Disks are
         * cut here to fit this bound.
         */
        uint64_t alignDisks(std::set<RSDisk*, bool (*) (RSDisk*, RSDisk*)>* disks, int32_t copies);

        /**
         * calculate the function descided in my PhD. Uses the cache to speed up.
         *
         * \f[P_{a,b,l}=\mbox{max}(0, \prod_{v=a}^{b-1}(1-l\cdot \check{c}_v)) \mbox{ for } a<b\f]
         *
         * and
         *
         * \f[P_{a,b,l}=1 \mbox{ for } a=b\f]
         */
        mpq_class P(int32_t a, int32_t b, int32_t l);

        /**
         * Used in r(o, m, j, i)
         */
        mpq_class r_inner(int32_t o, int32_t m, int32_t j, int32_t i);

        /**
         * calculate the function descided my PhD.
         *
         * \f[r_{o,m,j,i}=P_{j,i,m}\cdot\mbox{min}(1,m\cdot\check{c}_i) \mbox{ for } m=o\f]
         *
         * und
         *
         * \f[r_{o,m,j,i}=\sum_{v=j}^{i-m+o}P_{j,v,m}\cdot m\cdot\check{c}_v\cdot r_{o,m-1,v+1,i} \mbox{ for } o<m\f]
         */
        mpq_class r(int32_t o, int32_t m, int32_t j, int32_t i);

        /**
         * Used in r2(o, m, j, i)n descided my PhD.
         */
        mpq_class r2_inner(int32_t o, int32_t m, int32_t j, int32_t i);

        /**
         * calculate the function descided my PhD.
         *
         * \f[r2_{o,m,j,i}=P_{j,i,m}\cdot\mbox{min}(1,m\cdot\check{c}_i) \mbox{ for } m=o\f]
         *
         * und
         *
         * \f[r2_{o,m,j,i}=\sum_{v=j}^{i-m+o-1}P_{j,v,m}\cdot m\cdot\check{c}_v\cdot r2_{o,m-1,v+1,i} \mbox{ for } o<l\f]
         */
        mpq_class r2(int32_t o, int32_t m, int32_t j, int32_t i);

        /**
         * calculate the function descided my PhD.
         *
         * \f[k_i = k\cdot\frac{c_i}{C_0}-(\sum_{o=1}^{k}r_{o,k,0,i}) \mbox{ for } o\neq l-1\f]
         *
         * und
         *
         * \f[k_i = k\cdot\frac{c_i}{C_0}-(\sum_{o=1}^{k}r2_{o,k,0,i}) \mbox{ for } o=l-1\f]
         */
        mpq_class ki(int32_t i, int32_t l);

        /**
         * calculate the function descided my PhD.
         *
         * \f[c^* = \frac{\frac{k_i}{(l-1)\cdot r_{l,k,0,i-1}}\cdot C_{i+1}}{1-\frac{k_i}{(l-1)\cdot r_{l,k,0,i-1}}}\f]
         */
        mpq_class cStar(int32_t i, int32_t l);

        /**
         * print the configuration to std out.
         */
        void printConfiguration();

        /**
         * clean up this Object
         */
        void clear();

        uint64_t getTimeMS() {
            struct timeval before;
            gettimeofday(&before, 0);
            return (before.tv_sec * 1000 + before.tv_usec / 1000);
        }
    };

    /**
     * compare two RSDisks, needed to sort them. Returns if ld > rd. For two
     * disks of same capacity it returns if ld.id < rd.id.
     *
     * @param ld The first disk
     * @param rd the second disk
     *
     * @return ld > rd, for rd=ld ld.id < rd.id
     */
    bool rsDiskCompare(RSDisk* ld, RSDisk* rd);

    /**
     * compare two RSDisks, needed to sort them. Returns if ld > rd. For two
     * disks of same capacity it returns if ld.id > rd.id.
     *
     * @param ld The first disk
     * @param rd the second disk
     *
     * @return ld > rd, for rd=ld ld.id > rd.id
     */
    bool rsDiskCompareRevertID(RSDisk* ld, RSDisk* rd);
}
#endif	/* _DISTREDUNDANTSHARE_H */

