/* 
 * File:   ImprovedMap.cpp
 * Author: fermat
 * 
 * Created on 29. März 2010, 11:04
 */

#include <fstream>
#include <string>
#include <set>
#include<math.h>
#include<ostream>
#include <iostream>
#include <ios>
#include <limits>

#include "ImprovedMap.h"

using namespace VDRIVE;

ImprovedMap::ImprovedMap(uint64_t initSlices,
        std::set<uint64_t> *initKeys) {
    uint64_t sliceSize;
    // if initSices is 0 i will take the number of elements as initSlices
    if (initSlices == 0) {
        initSlices = initKeys->size();
        if (initSlices == 0)
            initSlices = 1;
    }
    if (initSlices == 1) {
        // Bitschift of 64 does not work as expected, therefore I Initiate this manually
        bitShift = 0;
        this->slices = 1;
        bitShift = 64;
        sliceSize = std::numeric_limits<uint64_t>::max();
    } else {
        // I will adjust the number slices, so that it is a power of two.
        // This way it is easier to calculate the slide.
        int16_t keyShift = (uint16_t) ceil(log2(initSlices * 1.0));
        this->slices = 1;
        this->slices <<= keyShift;
        bitShift = 64 - keyShift;
        sliceSize = 1;
        sliceSize <<= bitShift;
    }

    // Now we put in every slice a set containing the keys falling in the slice.
    // Each slice holds als lowest Element the biggest Element of the last
    // slice (if there is no key which fall exactly on the border).
    // To close the ring I add a virtual Element 0 at the begging (if there is
    // no Element with this value). Later, when searching the right Element,
    // this will be replaced with lastKey.
    keys = new std::set<uint64_t>*[this->slices];
    std::set<uint64_t>::iterator key = initKeys->begin();
    std::set<uint64_t>::iterator previousKey;
    // Until the first Placement we have to add 0 Values at the beginning of each slice.
    bool firstPlaced = false;
    // The Ring has only to be closed of there is no key for zero.
    if (*key == 0) {
        this->lastKey = 0;
    } else {
        std::set<uint64_t>::reverse_iterator lastKey =
                initKeys->rbegin();
        this->lastKey = *lastKey;
    }

    for (uint64_t slice = 0; slice < this->slices; slice++) {
        uint64_t sliceStart = slice * sliceSize;
        uint64_t sliceEnd = sliceStart + sliceSize;
        std::set<uint64_t> *sliceSet = new std::set<uint64_t > ();
        keys[slice] = sliceSet;
        if ((key == initKeys->end()) || (*key > sliceStart)) {
            if (firstPlaced) {
                sliceSet->insert(*previousKey);
            } else {
                sliceSet->insert(0);
            }
        }
        if (slice == (this->slices - 1)) {
            // We have reached the last Element, therefore sliceEnd had an
            // overrun. So now we will all all key not used before in this
            // slice.
            while (key != initKeys->end()) {
                sliceSet->insert(*key);
                previousKey = key;
                key++;
                firstPlaced = true;
            }
        } else {
            while ((key != initKeys->end()) && (*key >= sliceStart) &&
                    (*key < sliceEnd)) {
                sliceSet->insert(*key);
                previousKey = key;
                key++;
                firstPlaced = true;
            }
        }
    }
}

ImprovedMap::ImprovedMap(const ImprovedMap& orig) {
}

ImprovedMap::~ImprovedMap() {
    for (uint64_t slice = 0; slice < this->slices; slice++) {
        delete keys[slice];
    }
    delete[] keys;
}

uint64_t ImprovedMap::findClosest(uint64_t key) {
    uint64_t sliceKey = key;
    sliceKey >>= bitShift;
    if (bitShift == 64) {
        sliceKey = 0;
    }
    std::set<uint64_t> *sliceSet = keys[sliceKey];
    if (sliceSet->size() == 0) {
        return lastKey;
    }
    std::set<uint64_t>::iterator upKey = sliceSet->lower_bound(key);
    if ((upKey == sliceSet->end()) || (*upKey > key))
        upKey--;
    if (*upKey == 0) {
        return lastKey;
    }
    return *upKey;
}

