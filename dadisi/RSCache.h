/* 
 * File:   RSCache.h
 * Author: fermat
 *
 * Created on 11. März 2010, 15:36
 */

#ifndef _RSCACHE_H
#define	_RSCACHE_H

#include <stdint.h>
#include<tr1/unordered_map>
#include <cstring>
#include<gmpxx.h>

#define VDRIVE_RSCACHE_LT std::tr1::unordered_map<int32_t, mpq_class*>

#define VDRIVE_RSCACHE_LTit std::tr1::unordered_map<int32_t, mpq_class*>::iterator

#define VDRIVE_RSCACHE_NT std::tr1::unordered_map<int32_t, RSCache*>

#define VDRIVE_RSCACHE_NTit std::tr1::unordered_map<int32_t, RSCache*>::iterator

namespace VDRIVE {

    /**
     * The RSCace is used for differen caching puposes during initialization of
     * a DistRedundantShare. It is organized as a tree over the several
     * parameters. The deepth of this tree id for every leaf depth. Each node
     * can (and will) hold many Elements in an unorderd map, which gives an
     * O(1) access to the next node. Therefore the RSCache has a O(depth)
     * complexity for adding and searching elements.
     *
     * To access entities in the cache the user has to pass a query to the
     * cache. Such a query is an array of int32_t-values with the length depth.
     *
     * The subclasses RSCacheLeaf and RSCacheNode will build the real tree.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class RSCache {
    public:
        /**
         * get the next best value for the query out of the cache. This is a
         * Elment where the first (depth-1) values hit and the last value is
         * lower or equal to the given value.
         *
         * @param query The query to the searched Element
         *
         * @result The best matching value and the position it was placed.
         */
        virtual std::pair<const int32_t, mpq_class*>* getHighValue(int32_t* query) = 0;

        /**
         * get the next best value for the query out of the cache. This is a
         * Elment where the first (depth-1) values hit and the last value is
         * higher or equal to the given value.
         *
         * @param query The query to the searched Element
         *
         * @result The best matching value and the position it was placed.
         */
        virtual std::pair<const int32_t, mpq_class*>* getLowValue(int32_t* query) = 0;

        /**
         * set the value at the given position.
         *
         * @param query the position to place the value
         * @param value the value to place
         */
        virtual void setValue(int32_t* query, mpq_class &value) = 0;

        /**
         * get the value at the given position.
         *
         * @param query the position the value is placed
         *
         * @return the value at the given position or 0 if there is none.
         */
        virtual mpq_class* getValue(int32_t* query) = 0;

        /**
         * create a new RSCache with the given depth (depth has to be greater
         * or equal to 1).
         *
         * @param depth The depth of the cache
         *
         * @return a new RSCache with the given depth
         */
        static RSCache* createInstance(int32_t depth);

        /**
         * destructor. Destroys the whole cache.
         */
        virtual ~RSCache() {
        }
    };

    /**
     * This class represents a Leaf in a RSCache.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class RSCacheLeaf : public VDRIVE::RSCache {
    public:
        /**
         * @see RSCache::getValue
         */
        virtual mpq_class* getValue(int32_t* query);

        /**
         * see RSCache::getHighValue
         */
        virtual std::pair<const int32_t, mpq_class*>* getHighValue(int32_t* query);

        /**
         * see RSCache::getLowValue
         */
        virtual std::pair<const int32_t, mpq_class*>* getLowValue(int32_t* query);

        /**
         * see RSCache::setValue
         */
        virtual void setValue(int32_t* query, mpq_class &value);

        /**
         * destructor.
         */
        virtual ~RSCacheLeaf();

        /**
         * Create a new Leaf (A SubCache of depth 1). Should not be used by
         * users of a RSCache. The instanciation of a cache sould be done
         * using RSCache::createInstance(int32_t depth).
         *
         * @see RSCache::createInstance
         */
        RSCacheLeaf() {
            cache = new VDRIVE_RSCACHE_LT ();
            highVal = 0;
        }

    private:
        /**
         * Tha values of the caches.
         */
        VDRIVE_RSCACHE_LT *cache;

        /**
         * The highest key stored in this leaf.
         */
        int32_t highVal;

        /**
         * The lowest key stored in this leaf.
         */
        int32_t lowVal;
    };

    /**
     * This class represents a Node in a RSCache.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class RSCacheNode : public VDRIVE::RSCache {
    public:
        /**
         * @see RSCache::getValue
         */
        virtual mpq_class* getValue(int32_t* query);

        /**
         * see RSCache::getHighValue
         */
        virtual std::pair<const int32_t, mpq_class*>* getHighValue(int32_t* query);

        /**
         * see RSCache::getLowValue
         */
        virtual std::pair<const int32_t, mpq_class*>* getLowValue(int32_t* query);

        /**
         * see RSCache::setValue
         */
        virtual void setValue(int32_t* query, mpq_class &value);

        /**
         * destructor.
         */
        virtual ~RSCacheNode();

        /**
         * Create a new node (A SubCache of given depth). Should not be used by
         * users of a RSCache. The instanciation of a cache sould be done
         * using RSCache::createInstance(int32_t depth).
         *
         * @param depth the depth of the SubCache
         *
         * @see RSCache::createInstance
         */
        RSCacheNode(int32_t depth) {
            cache = new VDRIVE_RSCACHE_NT ();
            this->depth = depth;
        }

    private:
        /**
         * the depth of this SubCache
         */
        int32_t depth;

        /**
         * The SubCaches of this SubCache.
         */
        VDRIVE_RSCACHE_NT *cache;
    };
}
#endif	/* _RSCACHE_H */

