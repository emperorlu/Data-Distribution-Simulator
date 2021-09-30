/* 
 * File:   DistShare.cpp
 * Author: fermat
 * 
 * Created on 24. März 2010, 18:12
 */

#include "DistShare.h"
#include "DistShare_pThreads.h"
#include "ShareDisk.h"
#include<math.h>
#include <list>
#include <set>
#include <gmpxx.h>
#include <limits>
#include <map>
#include<sstream>
#include<ostream>
#include<iostream>
#include<string>
#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>
#include <tr1/unordered_map>
#include "helper.h"

using namespace VDRIVE;
using namespace xercesc;

static std::tr1::unordered_map<uint64_t, DistShare*> VDRIVE_DistShare_distributors;
static pthread_rwlock_t VDRIVE_DistShare_lock;
static int VDRIVE_DistShare_staticError = pthread_rwlock_init(&VDRIVE_DistShare_lock, 0);
static uint64_t VDRIVE_DistShare_nextID = 0;

void* VDRIVE_DistShare_runTest(void* args) {
    VDRIVE_DistShare_runTest_parm *parm = (VDRIVE_DistShare_runTest_parm*) args;
    uint64_t id = parm->dist_id;
    VDRIVE_DistShare_distributors[id]->__runInitThread(parm->thread_id);
    return args;
}

DistShare::DistShare(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode *n, *m;
    xercesc::DOMElement* disk_pos;
    XMLCh* xmlString;
    char *sysString, *sysString2;
    std::set<uint64_t> borders;

    xmlString = XMLString::transcode("baseMessage");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    uint8_t *bm = new uint8_t[64];
    uint16_t tmp;
    for (int i = 0; i < 64; i++) {
        isst >> tmp;
        bm[i] = tmp;
    }
    Distributor::setBaseMessage(bm);
    delete[] bm;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("extentsize");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->extentsize = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("copies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->copies = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("stretchFactorConst");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->stretchFactorConst = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("staticStretchFactor");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->staticStretchFactor = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("shareCopies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->shareCopies = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("nnCopies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->nnCopies;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("nnCopiesFactor");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->nnCopiesFactor;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("staticNNCopies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        staticNNCopies = true;
    } else {
        staticNNCopies = false;
    }
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("copiesByNN");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        this->copiesByNN = true;
    } else {
        this->copiesByNN = false;
    }
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("systemCapacity");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->systemCapacity = readInt64_t(sysString, 10);
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    bool internalHash;
    int gcryptAlgorithm;
    xmlString = XMLString::transcode("internalHash");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        internalHash = true;
    } else {
        internalHash = false;
    }
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("gcryptAlgorithm");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> gcryptAlgorithm;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    int internalHashAlgorithm;
    xmlString = XMLString::transcode("internalHashAlgorithm");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> internalHashAlgorithm;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    if (internalHash) {
        useGCryptHashAlgorithm(gcryptAlgorithm);
        useInternalHashAlgorithm(internalHashAlgorithm);
    } else {
        useInternalHashAlgorithm(internalHashAlgorithm);
        useGCryptHashAlgorithm(gcryptAlgorithm);
    }

    disks = new std::list<Disk*>();
    distributors = new std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>();

    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement* e = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(e->getTagName());
            if (strcmp(sysString, "disks") == 0) {
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        Disk *d = new Disk((xercesc::DOMElement*)m);
                        disks->push_back(d);
                    }
                }
            } else if (strcmp(sysString, "distributors") == 0) {
                xmlString = XMLString::transcode("position");
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        int64_t pos = 0;
                        disk_pos = (xercesc::DOMElement*) m;
                        sysString2 = XMLString::transcode(disk_pos->getTagName());
                        if (!strcmp(sysString2, "distPosition") == 0) {
                            std::cerr << "Found Element " << sysString2 << " but expected distPosition\n";
                            throw 1;
                        }
                        XMLString::release(&sysString2);
                        sysString2 = XMLString::transcode(disk_pos->getAttribute(xmlString));
                        try {
                            pos = readInt64_t(sysString2, 10);
                        } catch (int error) {
                            std::cerr << "String did not contain a number: " << sysString2;
                            throw error;
                        }
                        XMLString::release(&sysString2);
                        DOMNode *x = disk_pos->getFirstChild();
                        while ((x != 0) && (x->getNodeType() != DOMNode::ELEMENT_NODE))
                            x = x->getNextSibling();
                        if (x == 0) {
                            std::cerr << "Share has DistPosition without DistNN\n";
                            throw 1;
                        }
                        (*distributors)[pos] = new DistNearestNeighbour((xercesc::DOMElement*)x);
                        borders.insert(pos);
                    }
                }
                XMLString::release(&xmlString);
            }
            XMLString::release(&sysString);
        }
    }
    keyFinder = new ImprovedMap(disks->size(), &borders);
}

DistShare::DistShare(int argc, char** argv) {
    int argInd = 0;
    this->copies = 1;
    this->staticNNCopies = false;
    this->nnCopies = 0;
    this->nnCopiesFactor = 400;
    this->stretchFactorConst = 3;
    this->disks = 0;
    this->distributors = 0;
    this->shareCopies = 1;
    this->copiesByNN = false;
    this->staticStretchFactor = 0;
    if (VDRIVE_DistShare_staticError != 0) {
        std::cerr << "Warning: global pThread Initialization failed with error " << VDRIVE_DistShare_staticError << "\n";
    }

    // TODO: Implement that Share can get nnCopies and nnCopiesFactor for NearestNeighbour from User

    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-sf") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -sf needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-sf has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative stretch factor?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for sf is 2147483647";
            }
            this->stretchFactorConst = (uint32_t) val;
        } else if (strcmp(argv[argInd], "-ssf") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -ssf needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-ssf has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative stretch factor?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for sf is 2147483647";
            }
            this->staticStretchFactor = (uint32_t) val;
        } else if (strcmp(argv[argInd], "-sc") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -sc needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-sc has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative number copies of Disks in Share?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for sc is 2147483647";
            }
            this->shareCopies = (int32_t) val;
        } else if (strcmp(argv[argInd], "-nf") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -nf needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-nf has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative factor?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for nf is 2147483647";
            }
            this->nnCopiesFactor = (uint32_t) val;
        } else if (strcmp(argv[argInd], "-nc") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -nc needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-nc has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative number of copies?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for nc is 2147483647";
            }
            this->nnCopies = (uint32_t) val;
            this->staticNNCopies = true;
        } else if (strcmp(argv[argInd], "-cnn") == 0) {
            this->copiesByNN = true;
        } else {
            std::cout << "unknown option: " << argv[argInd] << "\n";
            throw 1;
        }
    }
}

DistShare::DistShare(const DistShare& orig) {
}

DistShare::~DistShare() {
    if (distributors != 0) {
        for (std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>::iterator it = distributors->begin(); it != distributors->end(); it++) {
            delete it->second;
        }
        delete distributors;
    }
    if (this->disks != 0) {
        for (std::list<Disk*>::iterator it = this->disks->begin(); it != this->disks->end(); it++) {
            delete *it;
        }
        delete this->disks;
    }
    delete keyFinder;
}

std::list<Disk*>* DistShare::getDisks() const {
    if (this->disks == 0)
        return 0;
    std::list<Disk*> *resDisks = new std::list<Disk*>();
    for (std::list<Disk*>::iterator it = this->disks->begin(); it != this->disks->end(); it++) {
        Disk *d = *it;
        resDisks->push_back(new Disk(*d));
    }
    return resDisks;
}

std::list<Disk*>* DistShare::placeExtent(int64_t virtualVolumeId, int64_t position)  {

    // TODO: Ändern, so dass k=1 und K>1 den gleichen Code verwenden.
    
    if ((copiesByNN)) {
        uint64_t pos = hashFunction(virtualVolumeId, position, 0);
        pos = keyFinder->findClosest(pos);
        if (distributors->count(pos) == 0) {
            std::cerr << "DistShare.placeExtent: keyfinder returned value " << pos << " but there is no distributor\n";
        }
        std::list<Disk*>* result = (*distributors)[pos]->placeExtent(virtualVolumeId, position);
        if (result->size() == 0) {
            std::cerr << "NN gave back an empty list\n";
        }
        if (result->size() != (uint32_t) copies) {
            std::cerr << "NN sould deliver " << copies << " Disks but gave only " << result->size() << "\n";
        }
        return result;
    } else {
        std::list<Disk*>* result = new std::list<Disk*>();
        std::set<int64_t> ids;
        int64_t i = 0;
        while (result->size() < (uint32_t) copies) {
            std::list<Disk*>* nnResult;
            uint64_t pos = hashFunction(virtualVolumeId, position, i);
            i++;
            pos = keyFinder->findClosest(pos);
            if (distributors->count(pos) == 0) {
                std::cerr << "DistShare.placeExtent: keyfinder returned value " << pos << " but there is no distributor\n";
            }
            uint64_t nnPos = hashFunctionNN(virtualVolumeId, position, i);
            nnResult = (*distributors)[pos]->placeExtent(virtualVolumeId, nnPos);
            Disk* nnDisk = nnResult->front();
            delete nnResult;
            if (ids.count(nnDisk->getId()) == 0) {
                ids.insert(nnDisk->getId());
                result->push_back(nnDisk);
            } else {
                delete nnDisk;
            }
        }
        return result;
    }
}

void DistShare::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->extentsize = extentsize;
    this->copies = copies;
    setDisks(disks);
}

void DistShare::setDisks(std::list<Disk*>* disks) {
    std::set<uint64_t> borders;
    this->systemCapacity = 0;
    if (this->disks != 0)
        delete this->disks;
    if (this->distributors != 0)
        delete this->distributors;
    this->disks = 0;
    this->distributors = 0;
    std::list<ShareDisk*> *sDisks = new std::list<ShareDisk*>();
    std::list<Disk*> *origDisks = new std::list<Disk*>();
    int32_t shareCopies = getShareCopies(disks);
    double stretchFactor = getStretchFactor(disks);
    if (copiesByNN)
        stretchFactor *= copies;
    this->disks = origDisks;

    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        Disk *d = new Disk(**it);
        this->systemCapacity += d->getCapacity();
        origDisks->push_back(d);
    }

    mpz_class sysCapMPZ = create_mpz(this->systemCapacity);
    mpz_class max_int64 = create_mpz(std::numeric_limits<uint64_t>::max());
    uint32_t max_int32 = std::numeric_limits<uint32_t>::max();
    std::istringstream isst;
    bool allFilled = false;

    while (!allFilled) {
        borders.clear();
        for (std::list<Disk*>::iterator it = origDisks->begin(); it != origDisks->end(); it++) {
            Disk* disk = *it;
            mpz_class length = create_mpz(disk->getCapacity()) * max_int64;
            length *= stretchFactor;
            length /= sysCapMPZ;
            int32_t rounds = 0;
            if (length > max_int64) {
                mpz_class mpRounds = length / max_int64;
                rounds = mpRounds.get_ui();
            }
            for (int32_t i = 0; i < shareCopies; i++) {
                int64_t start = hashFunction(disk, i);
                mpz_class tmp = length + create_mpz(start);
                tmp %= max_int64;
                uint64_t end;
                if (tmp > max_int32) {
                    end = readInt64_t(tmp.get_str(10).c_str(), 10);
                } else {
                    end = tmp.get_ui();
                }
                ShareDisk *sd = new ShareDisk(disk->getId(), start, end, rounds, i);
                borders.insert(start);
                borders.insert(end);
                sDisks->push_back(sd);
            }
        }

        if (copiesByNN) {
            allFilled = true;
            for (std::set<uint64_t>::iterator it = borders.begin(); (it != borders.end()) && (allFilled); it++) {
                uint64_t p = *it - 1;
                std::set<int64_t> ids;
                for (std::list<ShareDisk*>::iterator sdIt = sDisks->begin(); (sdIt != sDisks->end()) && (ids.size() < (uint32_t) copies); sdIt++) {
                    if (((*sdIt)->countCoversPoint(p) > 0) && (ids.count((*sdIt)->getID()) == 0))
                        ids.insert((*sdIt)->getID());
                }
                if (ids.size() < (uint32_t) copies)
                    allFilled = false;
            }
        } else {
            allFilled = true;
            for (std::set<uint64_t>::iterator it = borders.begin(); (it != borders.end()) && (allFilled); it++) {
                uint64_t p = *it - 1;
                bool pFilled = false;
                for (std::list<ShareDisk*>::iterator sdIt = sDisks->begin(); (sdIt != sDisks->end()) && (!pFilled); sdIt++) {
                    if ((*sdIt)->countCoversPoint(p) > 0)
                        pFilled = true;
                }
                if (!pFilled)
                    allFilled = false;
            }
        }

        if (!allFilled) {
            stretchFactor *= 2;
            for (std::list<ShareDisk*>::iterator it = sDisks->begin(); it != sDisks->end(); it++) {
                delete *it;
            }
            sDisks->clear();
        }
    }

    distributors = new std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>();

    if ((getNumThreads() < 2) || (VDRIVE_DistShare_staticError != 0)) {
        std::cout << "Initializing share without Threads\n";
        std::cout << "borders has a length of " << borders.size() << "\n";
        //Initializing without Threads.
        uint8_t *bm = getBaseMessage();
        for (std::set<uint64_t>::iterator it = borders.begin(); it != borders.end(); it++) {
            uint64_t p = *it - 1;
            VDRIVE_DNN_MULT *multi = new VDRIVE_DNN_MULT();
            for (std::list<ShareDisk*>::iterator sdIt = sDisks->begin(); sdIt != sDisks->end(); sdIt++) {
                (*multi)[(*sdIt)->getID()] += (*sdIt)->countCoversPoint(p);
            }
            DistNearestNeighbour *dnn = new DistNearestNeighbour(staticNNCopies, nnCopies, nnCopiesFactor);
            dnn->setBaseMessage(bm);
            if (copiesByNN) {
                dnn->setConfiguration(0, extentsize, copies);
            } else {
                dnn->setConfiguration(0, extentsize, 1);
            }
            dnn->setDisksMulti(origDisks, multi);
            (*distributors)[*it] = dnn;
            delete multi;
        }
        delete[] bm;
    } else {
        //Initializing with Threads.
        int error;
        pthread_t threads[getNumThreads()];
        uint64_t distNum;

        workPackages = new std::list<NNInitWorkPackage*>();

        for (std::set<uint64_t>::iterator it = borders.begin(); it != borders.end(); it++) {
            NNInitWorkPackage *wp = new NNInitWorkPackage();
            wp->pos = *it;
            uint64_t p = *it - 1;
            VDRIVE_DNN_MULT *multi = new VDRIVE_DNN_MULT();
            for (std::list<ShareDisk*>::iterator sdIt = sDisks->begin(); sdIt != sDisks->end(); sdIt++) {
                (*multi)[(*sdIt)->getID()] += (*sdIt)->countCoversPoint(p);
            }
            wp->disks = origDisks;
            wp->multi = multi;
            workPackages->push_back(wp);
        }

        workPackageResultLock = new pthread_rwlock_t();
        workPackageLock = new pthread_rwlock_t();

        error = pthread_rwlock_init(workPackageLock, 0);
        if (error != 0) {
            std::cerr << "Error " << error << " while initializing workPackageLock\n";
            throw 1;
        }
        error = pthread_rwlock_init(workPackageResultLock, 0);
        if (error != 0) {
            std::cerr << "Error " << error << " while initializing workPackageResultLock\n";
            throw 1;
        }

        error = pthread_rwlock_wrlock(&VDRIVE_DistShare_lock);
        if (error != 0) {
            std::cerr << "Error " << error << " while get VD lock\n";
            throw 1;
        }

        distNum = VDRIVE_DistShare_nextID++;

        error = pthread_rwlock_unlock(&VDRIVE_DistShare_lock);
        if (error != 0) {
            std::cerr << "Error " << error << " while release VD lock\n";
            throw 1;
        }

        VDRIVE_DistShare_distributors[distNum] = this;

        // Start the Threads
        for (uint16_t i = 0; i < getNumThreads(); i++) {
            VDRIVE_DistShare_runTest_parm *parm = new VDRIVE_DistShare_runTest_parm();
            parm->dist_id = distNum;
            parm->thread_id = i;
            if ((error = pthread_create(&threads[i], 0, VDRIVE_DistShare_runTest, parm))) {
                std::cerr << "Error " << error << " while trying to start thread " << i << "\n";
                throw 1;
            }
        }

        // Wait for all Threads to be done
        for (int32_t i = 0; i < getNumThreads(); i++) {
            VDRIVE_DistShare_runTest_parm *parm;
            if ((error = pthread_join(threads[i], (void**) & parm))) {
                std::cerr << "Error " << error << " joining Thread " << i << "\n";
                throw 1;
            }
            delete parm;
        }

        pthread_rwlock_destroy(workPackageLock);
        pthread_rwlock_destroy(workPackageResultLock);
        delete workPackageLock;
        delete workPackageResultLock;
        delete workPackages;
    }

    for (std::list<ShareDisk*>::iterator sdIt = sDisks->begin(); sdIt != sDisks->end(); sdIt++) {
        delete *sdIt;
    }
    delete sDisks;

    keyFinder = new ImprovedMap(disks->size(), &borders);
}

void DistShare::__runInitThread(uint32_t threadID) {
    int error;
    NNInitWorkPackage *workPackage;
    bool terminate = false;
    uint8_t *bm = getBaseMessage();

    while (!terminate) {
        error = pthread_rwlock_wrlock(workPackageLock);
        if (error != 0) {
            std::cerr << "Error: DistShase.__runInitThread could not get wrlock " <<
                    "because of error " << error << "\n";
            throw 1;
        }

        if (workPackages->empty()) {
            terminate = true;
        } else {
            workPackage = workPackages->front();
            workPackages->pop_front();
        }

        error = pthread_rwlock_unlock(workPackageLock);
        if (error != 0) {
            std::cerr << "Error: DistShase.__runInitThread could not release wrlock " <<
                    "because of error " << error << "\n";
            throw 1;
        }

        if (!terminate) {
            DistNearestNeighbour *dnn = new DistNearestNeighbour(staticNNCopies, nnCopies, nnCopiesFactor);
            dnn->setBaseMessage(bm);
            if (copiesByNN) {
                dnn->setConfiguration(0, extentsize, copies);
            } else {
                dnn->setConfiguration(0, extentsize, 1);
            }
            dnn->setDisksMulti(workPackage->disks, workPackage->multi);

            error = pthread_rwlock_wrlock(workPackageResultLock);
            if (error != 0) {
                std::cerr << "Error: DistShase.__runInitThread could not get wrlock " <<
                        "for result because of error " << error << "\n";
                throw 1;
            }

            (*distributors)[workPackage->pos] = dnn;

            error = pthread_rwlock_unlock(workPackageResultLock);
            if (error != 0) {
                std::cerr << "Error: DistShase.__runInitThread could not release wrlock " <<
                        "for result because of error " << error << "\n";
                throw 1;
            }

            delete workPackage->multi;
            delete workPackage;
        }
    }
    delete[] bm;
}

void DistShare::setBaseMessage(uint8_t* baseMessage) {
    if (distributors != 0) {
        for (std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>::iterator it = distributors->begin(); it != distributors->end(); it++) {
            it->second->setBaseMessage(baseMessage);
        }
    }
    Distributor::setBaseMessage(baseMessage);
}

uint64_t DistShare::hashFunction(Disk* disk, int64_t iteration) const {
    std::stringstream buf;
    buf << "Share::" << disk->getId() << "::" << iteration;
    std::string s = buf.str();
    return Distributor::hashFunctionInt64(&s);
}

uint64_t DistShare::hashFunction(int64_t virtualVolumeId, int64_t blockposition, int64_t index) const {
    std::stringstream buf;
    buf << "Share::" << virtualVolumeId << "::" << blockposition << "::" << index;
    std::string s = buf.str();
    return Distributor::hashFunctionInt64(&s);
}

uint64_t DistShare::hashFunctionNN(int64_t virtualVolumeId, int64_t blockposition, int64_t index) const {
    std::stringstream buf;
    buf << "Share::NextData::" << virtualVolumeId << "::" << blockposition << "::" << index;
    std::string s = buf.str();
    return Distributor::hashFunctionInt64(&s);
}

xercesc::DOMElement* DistShare::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE, *distributorsE;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2, *xmlString3;

    xmlString = XMLString::transcode(DistShare::getXMLRootType().c_str());
    result = doc->createElement(xmlString);
    XMLString::release(&xmlString);

    uint8_t *bm = getBaseMessage();
    out << (uint16_t) bm[0];
    for (int i = 1; i < 64; i++) {
        out << " " << (uint16_t) bm[i];
    }
    delete[] bm;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("baseMessage");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->systemCapacity;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("systemCapacity");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->extentsize;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("extentsize");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->copies;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("copies");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->stretchFactorConst;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("stretchFactorConst");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->staticStretchFactor;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("staticStretchFactor");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->shareCopies;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("shareCopies");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->nnCopies;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("nnCopies");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->nnCopiesFactor;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("nnCopiesFactor");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    if (staticNNCopies) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("staticNNCopies");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    if (copiesByNN) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("copiesByNN");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    bool internalHash;
    int gcryptAlgorithm;
    int internalHashAlgorithm;
    usedHashAlgorithm(&internalHash, &gcryptAlgorithm, &internalHashAlgorithm);

    out << internalHashAlgorithm;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("internalHashAlgorithm");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    if (internalHash) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("internalHash");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    out << gcryptAlgorithm;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("gcryptAlgorithm");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    xmlString = XMLString::transcode("disks");
    disksE = doc->createElement(xmlString);
    XMLString::release(&xmlString);
    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        disksE->appendChild((*it)->toXML(doc));
    }
    result->appendChild(disksE);

    xmlString = XMLString::transcode("distributors");
    distributorsE = doc->createElement(xmlString);
    XMLString::release(&xmlString);
    xmlString = XMLString::transcode("distPosition");
    xmlString2 = XMLString::transcode("position");
    for (std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>::iterator it = distributors->begin(); it != distributors->end(); it++) {
        DOMElement* pos = doc->createElement(xmlString);
        out << it->first;
        xmlString3 = XMLString::transcode(out.str().c_str());
        pos->setAttribute(xmlString2, xmlString3);
        XMLString::release(&xmlString3);
        out.str("");
        out.clear();
        pos->appendChild(it->second->toXML(doc));
        distributorsE->appendChild(pos);

    }
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    result->appendChild(distributorsE);
    return result;
}

double DistShare::getStretchFactor(const std::list<Disk*>* disks) {
    if (staticStretchFactor > 0)
        return staticStretchFactor;
    if (disks->size() < 2)
        return 1.0;
    return this->stretchFactorConst * 1.0 * log2(1.0 * disks->size());
}

int32_t DistShare::getShareCopies(const std::list<Disk*>* disks) {
    return shareCopies;
}
