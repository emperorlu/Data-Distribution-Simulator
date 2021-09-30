/* 
 * File:   RSCache.cpp
 * Author: fermat
 * 
 * Created on 11. März 2010, 15:36
 */

#include "RSCache.h"
#include  <iostream> 

using namespace VDRIVE;

RSCacheLeaf::~RSCacheLeaf() {
//    std::cout << "Delete Leaf\n";
    for (VDRIVE_RSCACHE_LTit it = cache->begin(); it != cache->end(); it++) {
        mpq_class *val = it->second;
        delete val;
    }
    delete cache;
}

void RSCacheLeaf::setValue(int32_t* query, mpq_class &value) {
    int32_t key = query[0];
//    std::cout << key << "\n";
    //(*cache)[key] = new mpq_class(value);
    VDRIVE_RSCACHE_LTit it = cache->find(key);
    if (it == cache->end()) {
        mpq_class *myValue = new mpq_class(value);
        std::pair<int32_t, mpq_class*> pair = std::make_pair(key, myValue);
        cache->insert(pair);
    } else {
        std::cout << "Adding existing value...\n";
    }
    if (key > highVal) {
        highVal = key;
    }
    if (key < lowVal) {
        lowVal = key;
    }
}

std::pair<const int32_t, mpq_class*>* RSCacheLeaf::getHighValue(int32_t* query) {
    int32_t key = query[0];
    if (cache->size() == 0) {
        return 0;
    }
    if (key > highVal)
        key = highVal;
    return &(*(cache->find(key)));
}

std::pair<const int32_t, mpq_class*>* RSCacheLeaf::getLowValue(int32_t* query) {
    int32_t key = query[0];
    if (cache->size() == 0) {
        return 0;
    }
    if (key < lowVal)
        key = lowVal;
    return &(*(cache->find(key)));
}

mpq_class* RSCacheLeaf::getValue(int32_t* query) {
    int32_t key = query[0];
    VDRIVE_RSCACHE_LTit it = cache->find(key);
    if (it == cache->end()) {
        return 0;
    } else {
        return it->second;
    }
}

RSCacheNode::~RSCacheNode() {
    for (VDRIVE_RSCACHE_NTit it = cache->begin(); it != cache->end(); it++) {
        delete it->second;
    }
    delete cache;
}

void RSCacheNode::setValue(int32_t* query, mpq_class &value) {
    int32_t key = query[0];
//    std::cout << key << ":";
    query++;
    RSCache* son = (*cache)[key];
    if (son == 0) {
        son = RSCache::createInstance(depth - 1);
        (*cache)[key] = son;
    }
    son->setValue(query, value);
}

std::pair<const int32_t, mpq_class*>* RSCacheNode::getHighValue(int32_t* query) {
    int32_t key = query[0];

    query++;
    VDRIVE_RSCACHE_NTit it = cache->find(key);
    if (it == cache->end()) {
        return 0;
    } else {
        return it->second->getHighValue(query);
    }
}

std::pair<const int32_t, mpq_class*>* RSCacheNode::getLowValue(int32_t* query) {
    int32_t key = query[0];

    query++;
    VDRIVE_RSCACHE_NTit it = cache->find(key);
    if (it == cache->end()) {
        return 0;
    } else {
        return it->second->getLowValue(query);
    }
}

mpq_class* RSCacheNode::getValue(int32_t* query) {
    int32_t key = query[0];

    query++;
    VDRIVE_RSCACHE_NTit it = cache->find(key);
    if (it == cache->end()) {
        return 0;
    } else {
        return it->second->getValue(query);
    }
}

RSCache* RSCache::createInstance(int32_t depth) {
    if (depth == 0) {
        return 0;
    }
    if (depth == 1) {
        return new RSCacheLeaf();
    }
    return new RSCacheNode(depth);
}
