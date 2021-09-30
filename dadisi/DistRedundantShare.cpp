/* 
 * File:   DistRedundantShare.cpp
 * Author: fermat
 * 
 * Created on 20. Januar 2010, 17:39
 */

#define __STDC_FORMAT_MACROS

#include <cstring>
#include <gmpxx.h>
#include <stdlib.h>
#include <set>
#include <sstream>
#include <stdio.h>
#include <xercesc/util/XMLString.hpp>
#include "DistRedundantShare.h"
#include <sys/time.h>
#include <inttypes.h>
#include "helper.h"
#include <iostream>
#include <limits>

using namespace VDRIVE;
using namespace std;
using namespace xercesc;

mpq_class ZERO(0), ONE(1);

DistRedundantShare::DistRedundantShare(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode *n, *m;
    xercesc::DOMElement* disk_pos;
    XMLCh* xmlString;
    char* sysString;

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

    xmlString = XMLString::transcode("capacity");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->capacity;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("copies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->copies;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("numDisks");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->numDisks;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("extentsize");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->extentsize;
    isst.str("");
    isst.clear();
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

    xmlString = XMLString::transcode("revertIDOrder");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        this->revertIDOrder = true;
    } else {
        this->revertIDOrder = false;
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

    disks = new RSDisk*[numDisks];

    xmlString = XMLString::transcode("position");
    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            int32_t pos = 0;
            disk_pos = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(disk_pos->getAttribute(xmlString));
            isst.str(sysString);
            isst >> pos;
            isst.str("");
            isst.clear();
            XMLString::release(&sysString);
            m = disk_pos->getFirstChild();
            while ((m != 0) && (m->getNodeType() != DOMNode::ELEMENT_NODE)) {
                m = m->getNextSibling();
            }
            if (m == 0) {
                std::cerr << "RS read a position " << pos << " without a Disk\n";
                abort();
            }
            disks[pos] = new RSDisk((xercesc::DOMElement*)m);
        }
    }
    XMLString::release(&xmlString);

    this->hashValues = 0;
    this->errorValues = 0;
    this->errorDisks = 0;
    initValueCaches();
}

DistRedundantShare::DistRedundantShare(int argc, char** argv) {
    this->capacity = 0;
    this->copies = 0;
    //this->debugfile = 0;
    this->disks = 0;
    this->extentsize = 1;
    this->numDisks = 0;
    this->revertIDOrder = false;
    int argInd;
    this->hashValues = 0;
    this->errorValues = 0;
    this->errorDisks = 0;

#ifdef USE_P_CACHE
    this->p_cache = 0;
#endif
    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-decID") == 0) {
            this->revertIDOrder = true;
        } else {
            std::cout << "unknown option: " << argv[argInd] << "\n";
            throw 1;
        }
    }
}

DistRedundantShare::DistRedundantShare(const DistRedundantShare& orig) {
}

DistRedundantShare::~DistRedundantShare() {
    clear();
    if (this->hashValues != 0) {
        for (int i = 0; i < numDisks; i++) {
            delete[] this->hashValues[i];
        }
        delete[] this->hashValues;
        this->hashValues = 0;
    }
    if (this->errorValues != 0) {
        delete[] this->errorValues;
        this->errorValues = 0;
    }
    if (errorDisks != 0)
        delete[] errorDisks;
}

mpq_class DistRedundantShare::P(int32_t a, int32_t b, int32_t l) {
    //debugfile << getTimeMS() << ";" << "P;" << a << ";" << b << ";" << l << "\n";
    if (a > b) {
        return ZERO;
    }
    if (a == b) {
        return ONE;
    }
#ifdef USE_P_CACHE
    pair<const int32_t, mpq_class*>* bestP = 0;
    bestP = getBestP(a, b, l);
    if ((bestP != 0) && (bestP->first == a)) {
        return *(bestP->second);
    }
#endif
    mpq_class lbr(l);
    if (b > 0) {
        mpq_class diskBefore = disks[b - 1]->getCheckC() * lbr;
        if (diskBefore > 1) {
#ifdef USE_P_CACHE
            setP(a, b, l, new mpq_class(0));
#endif
            return ZERO;
        }
    }

    mpq_class result = ONE;
    //I changed the direction of the loop, so I can use the P Cache more efficient.
    int32_t start = b - 1;
#ifdef USE_P_CACHE
    if (bestP != 0) {
        start = bestP->first;
        start--;
        result = *(bestP->second);
    }
#endif
    for (int v = start; v >= a; v--) {
        result = result * (ONE - (lbr * disks[v]->getCheckC()));
        if (result < 0) {
            return ZERO;
        }
#ifdef USE_P_CACHE
        setP(v, b, l, new mpq_class(result));
#endif
    }
    return result;
}

mpq_class DistRedundantShare::r_inner(int32_t o, int32_t m, int32_t j, int32_t i) {
    debugfile << getTimeMS() << ";" << "rinner;" << o << ";" << m << ";" << j << ";" << i << "\n";
#ifdef USE_R_CACHE
    mpq_class* cacheHit = getR(o, m, j, i);
    if (cacheHit != 0)
        return *cacheHit;
#endif
    if (o == m) {
        mpq_class result(m);
        result = result * disks[i]->getCheckC();
        if (result > 1) {
            result = ONE;
        }
        result = result * P(j, i, m);
#ifdef USE_R_CACHE
        setR(o, m, j, i, result);
#endif
        return result;
    }

    mpq_class result = ZERO;
    for (int v = j; v <= i - m + o; v++) {
        mpq_class stepResult = disks[v]->getCheckC();
        stepResult = stepResult * P(j, v, m);
        stepResult = stepResult * r_inner(o, m - 1, v + 1, i);
        result = result + stepResult;
    }
    result = result * m;
#ifdef USE_R_CACHE
    setR(o, m, j, i, result);
#endif
    return result;
}

mpq_class DistRedundantShare::r(int32_t o, int32_t m, int32_t j, int32_t i) {
    debugfile << getTimeMS() << ";" << "r;" << o << ";" << m << ";" << j << ";" << i << "\n";
    if (o > m) {
        return ZERO;
    }
    if (j > i) {
        return ZERO;
    }
    if (j > i - m + o) {
        return ZERO;
    }
    if ((disks[i - 1]->getCheckC() * o) > 1) {
        return ZERO;
    }
    return r_inner(o, m, j, i);
}

mpq_class DistRedundantShare::r2_inner(int32_t o, int32_t m, int32_t j, int32_t i) {
    debugfile << getTimeMS() << ";" << "r2inner;" << o << ";" << m << ";" << j << ";" << i << "\n";
#ifdef USE_R2_CACHE
    mpq_class* cacheHit = getR2(o, m, j, i);
    if (cacheHit != 0)
        return *cacheHit;
#endif
    if (o == m) {
        mpq_class result(m);
        result = result * disks[i]->getCheckC();
        if (result > 1) {
            result = ONE;
        }
        result = result * P(j, i, m);
#ifdef USE_R2_CACHE
        setR2(o, m, j, i, result);
#endif
        return result;
    }
    mpq_class result = ZERO;
    for (int v = j; v <= i - m + o - 1; v++) {
        mpq_class stepResult = disks[v]->getCheckC();
        stepResult = stepResult * P(j, v, m);
        stepResult = stepResult * r2_inner(o, m - 1, v + 1, i);
        result = result + stepResult;
    }
    result = result * m;
#ifdef USE_R2_CACHE
    setR2(o, m, j, i, result);
#endif
    return result;
}

mpq_class DistRedundantShare::r2(int32_t o, int32_t m, int32_t j, int32_t i) {
    debugfile << getTimeMS() << ";" << "r2inner;" << o << ";" << m << ";" << j << ";" << i << "\n";
    if (o > m) {
        return ZERO;
    }
    if (j > i) {
        return ZERO;
    }
    if (j > i - m + o - 1) {
        return ZERO;
    }
    if ((disks[i - 1]->getCheckC() * o) > 1) {
        return ZERO;
    }
    return r2_inner(o, m, j, i);
}

mpq_class DistRedundantShare::ki(int32_t i, int32_t l) {
    debugfile << getTimeMS() << ";" << "ki;" << i << ";" << l << "\n";
    mpq_class result = create_mpq(copies * disks[i]->getAlignedCapacity(), disks[0]->getCPosition());
    for (int o = copies; o > 0; o--) {
        if (o == (l - 1)) {
            result = result - r2(o, copies, 0, i);
        } else {
            result = result - r(o, copies, 0, i);
        }
    }
    return result;
}

mpq_class DistRedundantShare::cStar(int32_t i, int32_t l) {
    debugfile << getTimeMS() << ";" << "cStar;" << i << ";" << l << "\n";
    mpq_class tmp(1, l - 1);
    tmp = tmp * ki(i, l);
    tmp = tmp / r(l, copies, 0, i - 1);
    mpq_class upper = create_mpq_n(disks[i + 1]->getCPosition());
    upper = upper * tmp;
    mpq_class lower = ONE - tmp;
    return (upper / lower);
}

uint64_t DistRedundantShare::alignDisks(set<RSDisk*, bool (*) (RSDisk*, RSDisk*)>* disks, int32_t copies) {
    uint64_t capacity = 0;
    int32_t i = copies - disks->size();
    for (set<RSDisk*>::reverse_iterator it = disks->rbegin(); it != disks->rend(); ++it) {
        RSDisk* disk = *it;
        if ((i <= 0) || ((uint32_t) disk->getCapacity() < capacity / (uint32_t) i)) {
            disk->setAlignedCapacity(disk->getCapacity());
        } else {
            disk->setAlignedCapacity(capacity / i);
        }
        capacity += disk->getAlignedCapacity();
        disk->setCPosition(capacity);
        disk->setCheckC(create_mpq(disk->getAlignedCapacity(), capacity));
        i++;
    }
    return capacity;
}

void DistRedundantShare::setDisks(std::list<Disk*>* newPDisks) {
    if (newPDisks == 0) {
        return;
    }
    if (newPDisks->size() < (uint32_t) copies) {
        return;
    }
    clear();

    bool(*fn_pt)(RSDisk*, RSDisk*);
    if (this->revertIDOrder) {
        fn_pt = rsDiskCompareRevertID;
    } else {
        fn_pt = rsDiskCompare;
    }
    set < RSDisk*, bool (*) (RSDisk*, RSDisk*)>* rsDisks = new set < RSDisk*, bool (*) (RSDisk*, RSDisk*)> (fn_pt);
    char* text = new char[50];
    for (list<Disk*>::iterator it = newPDisks->begin(); it != newPDisks->end(); ++it) {
        Disk* disk = *it;
        RSDisk* rsDisk = new RSDisk(disk);
        sprintf(text, "%" PRId64 "", disk->getId());
        string s(text);
        rsDisk->setRsId(s);
        rsDisks->insert(rsDisk);
    }
    delete[] text;

    this->capacity = alignDisks(rsDisks, copies);

    disks = new RSDisk*[rsDisks->size()];
    numDisks = rsDisks->size();
    int l = copies;
    int* errors = new int[copies];
    for (int i = 0; i < copies; i++) {
        errors[i] = -1;
    }

    int i = 0;
    for (set<RSDisk*>::iterator it = rsDisks->begin(); it != rsDisks->end(); ++it) {
        RSDisk* disk = *it;
        disks[i] = disk;
        mpq_class result = disk->getCheckC() * create_mpq_n(l);
        if (result == 1) {
            l--;
            errors[l] = -1;
        } else if (result > 1) {
            disk->setKError(l - 1);
            l--;
            errors[l] = i;
        }
        i++;
    }
    delete rsDisks;

    debugfile.open("/home/fermat/RSDebug.csv");
#ifdef USE_P_CACHE
    clearP();
#endif

#ifdef USE_R_CACHE
    initR();
#endif

#ifdef USE_R2_CACHE
    initR2();
#endif

    for (int i = 1; i < copies; i++) {
        //cout << "errors[" << i << "] = " << errors[i] << "\n";
        if (errors[i] != -1) {
            mpq_class cNew = cStar(errors[i], i + 1);
            mpq_class cap;
            if (errors[i] < numDisks - 1) {
                cap = cNew + create_mpq_n(disks[errors[i] + 1]->getCPosition());
            } else {
                cap = cNew;
            }
            mpq_class checkNew = cNew / cap;
            disks[errors[i]]->setCheckCStar(checkNew * i);
        }
    }
    delete[] errors;
#ifdef USE_P_CACHE
    clearP();
#endif

#ifdef USE_R_CACHE
    clearR();
#endif

#ifdef USE_R2_CACHE
    clearR2();
#endif

    debugfile.close();
    initValueCaches();
    //printConfiguration();
}

void DistRedundantShare::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    //cout << "Configuring RedundantShare with extentsize " << extentsize <<
    //        " copies " << copies << " and " << disks->size() << " disks\n";
    this->copies = copies;
    setDisks(disks);
}

std::list<Disk*>* DistRedundantShare::placeExtent(int64_t virtualVolumeId, int64_t position) {
    list<Disk*>* usedDisks = new list<Disk*>();
    int32_t i = 0;
    std::string s;
    std::stringstream buf;
    uint64_t dice;
    RSDisk* disk;
    uint64_t value;
    Disk* backDisk;
    for (int32_t toPlace = copies; toPlace > 0; toPlace--) {
        disk = disks[i];
        if (disk->getKError() == toPlace) {
            value = errorValues[toPlace - 1];
        } else {
            value = hashValues[i][toPlace - 1];
        }
        buf << "RS::" << disk->getId() << "::" << position;
        s = buf.str();
        dice = hashFunctionInt64(&s);
        buf.str("");
        while (dice > value) {
            i++;
            disk = disks[i];
            value = hashValues[i][toPlace - 1];
            buf << "RS::" << disk->getId() << "::" << position;
            s = buf.str();
            dice = hashFunctionInt64(&s);
            buf.str("");
        }
        //i = placeCopy(position, toPlace, i);
        backDisk = new Disk(*(disk->getDisk()));
        usedDisks->push_back(backDisk);
        i++;
    }
    return usedDisks;
}

int32_t DistRedundantShare::placeCopy(int64_t position, int32_t copy, int32_t firstDisk) const {
    RSDisk* disk = disks[firstDisk];
    uint64_t value;
    if (disk->getKError() == copy) {
        value = errorValues[copy - 1];
    } else {
        value = hashValues[firstDisk][copy - 1];
    }
    std::stringstream buf;
    buf << "RS::" << firstDisk << "::" << position;
    std::string s = buf.str();
    uint64_t dice = hashFunctionInt64(&s);
    buf.str("");
    int diskPos = firstDisk;
    while (dice > value) {
        diskPos++;
        value = hashValues[diskPos][copy - 1];
        buf << "RS::" << diskPos << "::" << position;
        s = buf.str();
        dice = hashFunctionInt64(&s);
        buf.str("");
    }
    return diskPos;
}

void DistRedundantShare::clear() {
    for (int i = 0; i < numDisks; i++) {
        delete disks[i];
    }
    delete[] disks;
}

void DistRedundantShare::printConfiguration() {
    for (int i = 0; i < numDisks; i++) {
        RSDisk* disk = disks[i];
        cout << i << ";" << disk->getCPosition() << ";" << disk->getCapacity() << ";" << disk->getAlignedCapacity() << ";" << disk->getKError() << ";";
        for (int k = 1; k <= copies; k++) {
            double val = k * disk->getCheckCDouble();
            cout << val << ";";
        }
        for (int k = 1; k <= copies; k++) {
            if (disk->getKError() >= 0) {
                double val = k * disk->getCheckCStarDouble();
                cout << val << ";";
            } else {
                cout << ";";
            }
        }
        cout << "\n";
    }
}

void DistRedundantShare::initValueCaches() {
    mpz_class maxUnit64 = create_mpz(std::numeric_limits<uint64_t>::max());
    hashValues = new uint64_t*[numDisks];
    errorValues = new uint64_t[copies];
    errorDisks = new int64_t[copies];
    //cout << "max: " << std::numeric_limits<uint64_t>::max() << "\n";
    for (int32_t disk = 0; disk < numDisks; disk++) {
        RSDisk* rsdisk = disks[disk];
        mpq_class baseValue = rsdisk->getCheckC() * maxUnit64;
        hashValues[disk] = new uint64_t[copies];
        if (rsdisk->getKError() > 0) {
            errorDisks[rsdisk->getKError()] = disk;
            mpq_class value = rsdisk->getCheckCStar() * maxUnit64;
            mpz_class zValue = value.get_num() / value.get_den();
            errorValues[rsdisk->getKError() - 1] = read_uint64_t(zValue);
            //cout << "errorValues[" << rsdisk->getKError() << "] = " << errorValues[rsdisk->getKError() - 1] << "\n";
        }
        for (int32_t copy = 0; copy < copies; copy++) {
            mpq_class value = baseValue * (copy + 1);
            mpz_class zValue = value.get_num() / value.get_den();
            if (zValue >= maxUnit64) {
                //cout << "Value: " << value.get_str(10) << "\n";
                hashValues[disk][copy] = std::numeric_limits<uint64_t>::max();
            } else {
                hashValues[disk][copy] = read_uint64_t(zValue);
            }
            //cout << "hashValues[" << disk << "][" << copy << "] = " << hashValues[disk][copy] << "\n";
        }
    }
}

xercesc::DOMElement* DistRedundantShare::toXML(xercesc::DOMDocument* doc) const {
    DOMElement* result;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2, *xmlString3;

    xmlString = XMLString::transcode(DistRedundantShare::getXMLRootType().c_str());
    result = doc->createElement(xmlString);
    XMLString::release(&xmlString);

    uint8_t *bm = getBaseMessage();
    out << (uint16_t) bm[0];
    for (int i = 1; i < 64; i++) {
        out << " " << (uint16_t) bm[i];
    }
    delete[] bm;
    std::cout << "BM: " << out.str() << "\n";
    std::cout << "BM: " << out.str().c_str() << "\n";
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("baseMessage");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->capacity;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("capacity");
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

    out << this->numDisks;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("numDisks");
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

    if (this->revertIDOrder) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("revertIDOrder");
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

    xmlString = XMLString::transcode("diskPosition");
    xmlString2 = XMLString::transcode("position");
    for (int32_t i = 0; i < numDisks; i++) {
        DOMElement* pos = doc->createElement(xmlString);
        out << i;
        xmlString3 = XMLString::transcode(out.str().c_str());
        pos->setAttribute(xmlString2, xmlString3);
        XMLString::release(&xmlString3);
        out.str("");
        out.clear();
        pos->appendChild(disks[i]->toXML(doc));
        result->appendChild(pos);
    }
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    return result;
}

std::list<Disk*>* DistRedundantShare::getDisks() const {
    std::list<Disk*>* result = new std::list<Disk*>();
    for (int i = 0; i < numDisks; i++) {
        Disk* origDisk = disks[i]->getDisk();
        Disk* clone = new Disk(*origDisk);
        result->push_back(clone);
    }
    return result;
}

#ifdef USE_P_CACHE

void DistRedundantShare::clearP() {
    if (p_cache != 0) {
        for (map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>::iterator it1 = (*p_cache).begin();
                it1 != (*p_cache).end(); ++it1) {
            pair<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*> elem1 = *it1;
            std::map<int32_t, std::map<int32_t, mpq_class*>*>* map1 = elem1.second;
            for (map<int32_t, std::map<int32_t, mpq_class*>*>::iterator it2 = map1->begin();
                    it2 != map1->end(); ++it2) {
                pair<int32_t, std::map<int32_t, mpq_class*>*> elem2 = *it2;
                std::map<int32_t, mpq_class*>* map2 = elem2.second;
                for (map<int32_t, mpq_class*>::iterator it3 = map2->begin(); it3 != map2->end(); ++it3) {
                    pair<int32_t, mpq_class*> elem3 = *it3;
                    mpq_class* dat = elem3.second;
                    delete(dat);
                }
                delete(map2);
            }
            delete(map1);
        }
        delete(p_cache);
    }
}

void DistRedundantShare::setP(int32_t a, int32_t b, int32_t l, mpq_class* val) {
    if (p_cache == 0) {
        p_cache = new std::map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>();
    }
    std::map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>::iterator l_it = p_cache->find(l);
    std::map<int32_t, std::map<int32_t, mpq_class*>*>* l_map = 0;
    if (l_it == p_cache->end()) {
        l_map = new std::map<int32_t, std::map<int32_t, mpq_class*>*>();
        p_cache->insert(make_pair(l, l_map));
    } else {
        l_map = l_it->second;
    }
    std::map<int32_t, std::map<int32_t, mpq_class*>*>::iterator b_it = l_map->find(b);
    std::map<int32_t, mpq_class*>* b_map = 0;
    if (b_it == l_map->end()) {
        b_map = new std::map<int32_t, mpq_class*>();
        l_map->insert(make_pair(b, b_map));
    } else {
        b_map = b_it->second;
    }
    std::map<int32_t, mpq_class*>::iterator a_it = b_map->find(a);
    if (a_it == b_map->end()) {
        b_map->insert(make_pair(a, val));
    } else {
        if (a_it->second == val) {
            //debugfile << " But the val is same...\n";
        } else {
            delete(a_it->second);
            a_it->second = val;
        }
    }
}

mpq_class* DistRedundantShare::getP(int32_t a, int32_t b, int32_t l) {
    if (p_cache == 0)
        return 0;
    std::map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>::iterator l_it = p_cache->find(l);
    if (l_it == p_cache->end()) {
        return 0;
    }
    std::map<int32_t, std::map<int32_t, mpq_class*>*>::iterator b_it = l_it->second->find(b);
    if (b_it == l_it->second->end()) {
        return 0;
    }
    std::map<int32_t, mpq_class*>::iterator a_it = b_it->second->find(l);
    if (a_it == b_it->second->end()) {
        return 0;
    }
    return a_it->second;
}

pair<const int32_t, mpq_class*>* DistRedundantShare::getBestP(int32_t a, int32_t b, int32_t l) {
    if (p_cache == 0)
        return 0;
    std::map<int32_t, std::map<int32_t, std::map<int32_t, mpq_class*>*>*>::iterator l_it = p_cache->find(l);
    if (l_it == p_cache->end()) {
        return 0;
    }
    std::map<int32_t, std::map<int32_t, mpq_class*>*>::iterator b_it = l_it->second->find(b);
    if (b_it == l_it->second->end()) {
        return 0;
    }
    std::map<int32_t, mpq_class*>::iterator a_it = b_it->second->find(l);
    if (a_it == b_it->second->end()) {
        a_it = b_it->second->lower_bound(l);
    }
    if (a_it == b_it->second->end()) {
        return 0;
    }
    //pair<int32_t, mpq_class*>* result = new pair<int32_t, mpq_class*>(*a_it);
    return &(*a_it);
}
#endif

bool VDRIVE::rsDiskCompare(VDRIVE::RSDisk* ld, VDRIVE::RSDisk* rd) {
    if (ld == rd)
        return false;
    if (ld == 0)
        return false;
    if (rd == 0)
        return true;
    if (ld->getCapacity() > rd->getCapacity())
        return true;
    if (ld->getCapacity() < rd->getCapacity())
        return false;
    return (ld->getId() < rd->getId());
}

bool VDRIVE::rsDiskCompareRevertID(VDRIVE::RSDisk* ld, VDRIVE::RSDisk* rd) {
    if (ld == rd)
        return false;
    if (ld == 0)
        return false;
    if (rd == 0)
        return true;
    if (ld->getCapacity() > rd->getCapacity())
        return true;
    if (ld->getCapacity() < rd->getCapacity())
        return false;
    return (ld->getId() > rd->getId());
}
