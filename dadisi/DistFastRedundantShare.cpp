/* 
 * File:   FastRedundantShare.cpp
 * Author: fermat
 * 
 * Created on 27. März 2010, 12:18
 */

#include <list>

#include "DistFastRedundantShare.h"
#include <stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#include<iostream>
#include<ostream>
#include <fstream>
#include<unistd.h>
#include<string.h>
#include <limits.h>
#include<stdint.h>
#include <sstream>
#include <map>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>
#include "helper.h"

using namespace VDRIVE;
using namespace xercesc;

DistFastRedundantShare::DistFastRedundantShare(int argc, char** argv) {
    distributors = 0;
}

DistFastRedundantShare::DistFastRedundantShare(const DistFastRedundantShare& orig) {
}

DistFastRedundantShare::DistFastRedundantShare(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode *n, *m, *l;
    xercesc::DOMElement* disk_pos;
    XMLCh *xmlString, *xmlString2;
    char *sysString, *sysString2;

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

    xmlString = XMLString::transcode("copies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->copies;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("extentsize");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->extentsize = readInt64_t(sysString, 10);
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

    disks = new VDRIVE_FRS_DISKMAP();
    distributors = new VDRIVE_FRS_MMAP();

    for (int32_t i = 1; i <= copies; i++) {
        (*distributors)[i] = new VDRIVE_FRS_KMAP();
    }

    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement* e = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(e->getTagName());
            if (strcmp(sysString, "disks") == 0) {
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        Disk *d = new Disk((xercesc::DOMElement*)m);
                        (*disks)[d->getId()] = d;
                    }
                }
            } else if (strcmp(sysString, "distributors") == 0) {
                xmlString2 = XMLString::transcode("diskPosition");
                xmlString = XMLString::transcode("kPosition");
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        int64_t dpos = 0;
                        int32_t kpos = 0;
                        disk_pos = (xercesc::DOMElement*) m;

                        sysString2 = XMLString::transcode(disk_pos->getAttribute(xmlString));
                        isst.str(sysString2);
                        isst >> kpos;
                        isst.str("");
                        isst.clear();
                        XMLString::release(&sysString2);

                        sysString2 = XMLString::transcode(disk_pos->getAttribute(xmlString2));
                        dpos = readInt64_t(sysString2, 10);
                        XMLString::release(&sysString2);

                        VDRIVE_FRS_KMAP *kmap = (*distributors)[kpos];
                        xercesc::DOMElement* shareElement = 0;
                        for (l = disk_pos->getFirstChild(); (l != 0) && (shareElement == 0); l = l->getNextSibling()) {
                            if (l->getNodeType() == DOMNode::ELEMENT_NODE) {
                                shareElement = (xercesc::DOMElement*)l;
                            }
                        }
                        if (shareElement == 0) {
                            std::cerr << "Could not find a ShareElement in disk position of FastRedundantShare\n";
                        }
                        (*kmap)[dpos] = new DistShare(shareElement);
                    }
                }
                XMLString::release(&xmlString);
                XMLString::release(&xmlString2);
            }
            XMLString::release(&sysString);
        }
    }
}

DistFastRedundantShare::~DistFastRedundantShare() {
    if (distributors != 0) {
        for (VDRIVE_FRS_MMAPit it = distributors->begin(); it != distributors->end(); it++) {
            VDRIVE_FRS_KMAP *kmap = it->second;
            for (VDRIVE_FRS_KMAPit it2 = kmap->begin(); it2 != kmap->end(); it2++) {
                delete it2->second;
            }
            delete kmap;
        }
        delete distributors;
    }
    for (VDRIVE_FRS_DISKMAPit it = disks->begin(); it != disks->end(); it++) {
        delete it->second;
    }
    delete disks;
}

void DistFastRedundantShare::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->extentsize = extentsize;
    this->copies = copies;
    setDisks(disks);
}

void DistFastRedundantShare::setDisks(std::list<Disk*>* disks) {
    this->disks = new VDRIVE_FRS_DISKMAP();
    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        Disk *d = *it;
        (*(this->disks))[d->getId()] = new Disk(*d);
    }
    DistRedundantShare *rs = new DistRedundantShare(0, 0);
    rs->setConfiguration(disks, extentsize, copies);
    initShareStructures(rs);
    delete rs;
}

void DistFastRedundantShare::initShareStructures(DistRedundantShare *rs) {
    mpz_class max_int64 = create_mpz(std::numeric_limits<uint64_t>::max());
    RSDisk **rsDisks = rs->getRSDisks();
    int32_t k = copies;
    int32_t numDisks = rs->getNumDisks();
    int32_t *lastDisk = new int32_t[k];
    int32_t l = k;

    if (distributors != 0) {
        for (VDRIVE_FRS_MMAPit it = distributors->begin(); it != distributors->end(); it++) {
            VDRIVE_FRS_KMAP *kmap = it->second;
            for (VDRIVE_FRS_KMAPit it2 = kmap->begin(); it2 != kmap->end(); it2++) {
                delete it2->second;
            }
            delete kmap;
        }
        delete distributors;
    }

    for (int32_t i = 0; i < numDisks; i++) {
        if (l * rsDisks[i]->getCheckC() >= 1) {
            l--;
            lastDisk[l] = i;
        }
    }

    distributors = new VDRIVE_FRS_MMAP();
    VDRIVE_FRS_KMAP *kmap = new VDRIVE_FRS_KMAP();
    (*kmap)[0] = createShare(rsDisks, 0, lastDisk[k - 1], k);
    (*distributors)[k] = kmap;

    for (int32_t l = k - 1; l > 0; l--) {
        VDRIVE_FRS_KMAP *kmap = new VDRIVE_FRS_KMAP();
        (*distributors)[l] = kmap;
        for (int32_t i = k - l; i <= lastDisk[l - 1]; i++) {
            (*kmap)[rsDisks[i - 1]->getDisk()->getId()] = createShare(rsDisks, i, lastDisk[l - 1], l);
        }
    }

    delete[] lastDisk;
}

DistShare* DistFastRedundantShare::createShare(RSDisk** disks, int32_t first, int32_t last, int32_t k) {
    const int64_t max_int64 = std::numeric_limits<int64_t>::max();
    const mpz_class max_int64_q = create_mpz(std::numeric_limits<int64_t>::max());
    std::list<Disk*>* share_disks = new std::list<Disk*>();
    int64_t rest = max_int64;
    mpq_class factor(1);
    mpq_class ONE(1);

    for (int32_t i = first; i < last; i++) {
        Disk *d = new Disk(*(disks[i]->getDisk()));
        mpq_class capacity = factor * disks[i]->getCheckC() * max_int64_q * k;
        d->setCapacity(read_int64_t(capacity));
        rest -= d->getCapacity();
        factor *= ONE - (k * disks[i]->getCheckC());
        share_disks->push_back(d);
    }

    Disk *d = new Disk(*(disks[last]->getDisk()));
    d->setCapacity(rest);

    // This is just to check, if the invariant holds:
    mpq_class test = factor * disks[last]->getCheckC() * max_int64_q * k;
    int64_t testU = read_int64_t(test);
    if (testU < rest) {
        // This only happens because of error while round the results.
        /*        std::cerr << "===========================================================\n";
                std::cerr << "I found an error while initializing FastRS:\n";
                std::cerr << "while building the Share Function for Disk " << first << " to " << last << "\n";
                std::cerr << "the last should not become more then an areal of\n";
                std::cerr << read_int64_t(test) << " but rest is\n";
                std::cerr << rest << "\n";
                std::cerr << "===========================================================\n";*/
        d->setCapacity(testU);
    }

    share_disks->push_back(d);

    DistShare* share = new DistShare(0, 0);
    uint8_t *bm = getBaseMessage();
    share->setBaseMessage(bm);
    share->setNumThreads(getNumThreads());
    delete[] bm;
    share->setConfiguration(share_disks, extentsize, 1);

    for (std::list<Disk*>::iterator it = share_disks->begin(); it != share_disks->end(); it++) {
        delete *it;
    }
    delete share_disks;

    return share;
}

void DistFastRedundantShare::setBaseMessage(uint8_t* baseMessage) {
    if (distributors != 0) {
        for (VDRIVE_FRS_MMAPit it = distributors->begin(); it != distributors->end(); it++) {
            VDRIVE_FRS_KMAP *kmap = it->second;
            for (VDRIVE_FRS_KMAPit it2 = kmap->begin(); it2 != kmap->end(); it2++) {
                it2->second->setBaseMessage(baseMessage);
            }
        }
    }
    Distributor::setBaseMessage(baseMessage);
}

std::list<Disk*>* DistFastRedundantShare::placeExtent(int64_t virtualVolumeId, int64_t position)  {
    int32_t k = copies;
    std::list<Disk*> *result = new std::list<Disk*>();
    std::list<Disk*> *stepResult;
    VDRIVE_FRS_KMAP *kmap = (*distributors)[k];
    stepResult = (*kmap)[0]->placeExtent(virtualVolumeId, position);
    Disk *d = stepResult->front();
    delete stepResult;
    result->push_back(d);
    for (int32_t l = k - 1; l > 0; l--) {
        kmap = (*distributors)[l];
        DistShare *share = (*kmap)[d->getId()];
        stepResult = share->placeExtent(virtualVolumeId, position);
        d = stepResult->front();
        delete stepResult;
        result->push_back(d);
    }
    // We need to change back the disks, because in share they have a wrong capacity.
    std::list<Disk*> *result2 = new std::list<Disk*>();
    for (std::list<Disk*>::iterator it = result->begin(); it != result->end(); it++) {
        Disk *old = *it;
        Disk *real = (*disks)[old->getId()];
        result2->push_back(new Disk(*real));
        delete old;
    }
    delete result;
    return result2;
}

std::list<Disk*>* DistFastRedundantShare::getDisks() const {
    std::list<Disk*>* result = new std::list<Disk*> ();
    for (VDRIVE_FRS_DISKMAPit it = disks->begin(); it != disks->end(); it++) {
        result->push_back(new Disk(*(it->second)));
    }
    return result;
}

xercesc::DOMElement* DistFastRedundantShare::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE, *distributorsE;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2, *xmlString3, *xmlString4;

    xmlString = XMLString::transcode(DistFastRedundantShare::getXMLRootType().c_str());
    result = doc->createElement(xmlString);
    XMLString::release(&xmlString);

    uint8_t *bm = this->getBaseMessage();
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

    out << this->copies;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("copies");
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
    for (VDRIVE_FRS_DISKMAPit it = disks->begin(); it != disks->end(); it++) {
        Disk *d = it->second;
        disksE->appendChild(d->toXML(doc));
    }
    result->appendChild(disksE);

    xmlString = XMLString::transcode("distributors");
    distributorsE = doc->createElement(xmlString);
    XMLString::release(&xmlString);
    xmlString = XMLString::transcode("distPosition");
    xmlString2 = XMLString::transcode("diskPosition");
    xmlString4 = XMLString::transcode("kPosition");
    for (VDRIVE_FRS_MMAPit it = distributors->begin(); it != distributors->end(); it++) {
        VDRIVE_FRS_KMAP *kmap = it->second;
        int32_t k = it->first;
        for (VDRIVE_FRS_KMAPit it2 = kmap->begin(); it2 != kmap->end(); it2++) {
            DOMElement* pos = doc->createElement(xmlString);

            out << it2->first;
            xmlString3 = XMLString::transcode(out.str().c_str());
            pos->setAttribute(xmlString2, xmlString3);
            XMLString::release(&xmlString3);
            out.str("");
            out.clear();

            out << k;
            xmlString3 = XMLString::transcode(out.str().c_str());
            pos->setAttribute(xmlString4, xmlString3);
            XMLString::release(&xmlString3);
            out.str("");
            out.clear();

            pos->appendChild(it2->second->toXML(doc));
            distributorsE->appendChild(pos);
        }
    }
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    XMLString::release(&xmlString4);
    result->appendChild(distributorsE);
    return result;
}
