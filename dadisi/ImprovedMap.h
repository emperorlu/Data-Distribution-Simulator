/* 
 * File:   ImprovedMap.h
 * Author: fermat
 *
 * Created on 29. März 2010, 11:04
 */

#ifndef _IMPROVEDMAP_H
#define	_IMPROVEDMAP_H

#include<stdint.h>
#include<set>

namespace VDRIVE {

    /**
     * ImprovedMap can be used to calculate fast the closest next smaller
     * Value from a set of value to an average Value. if there is no smaller
     * Value it delivers biggest Value. Therefore it cn be used to find next
     * Values on a Ring in O(1).
     */
    class ImprovedMap {
    public:
        /**
         * Initiate a new Map with the given number of Elements and the given
         * keys.
         *
         * @param initSlices number of slices to use (should be O(#keys) in
         * most cases
         * @param initKeys Keys to be stored in this Map
         */
        ImprovedMap(uint64_t initSlices, std::set<uint64_t> *initKeys);

        /**
         * copy constructur
         *
         * @param orig the ImprovedMap to be copied.
         **/
        ImprovedMap(const ImprovedMap& orig);

        /**
         * Destructor
         **/
        virtual ~ImprovedMap();

        /**
         * find the closest Element to the given value.
         *
         * @param key Element to be placed
         **/
        uint64_t findClosest(uint64_t key);
    private:
        uint16_t bitShift;
        uint64_t slices;
        std::set<uint64_t> **keys;
        uint64_t lastKey;
    };
}
#endif	/* _IMPROVEDMAP_H */

