/* 
 * File:   DistRoundRobin.cpp
 * Author: fermat
 * 
 * Created on 14. April 2010, 11:17
 */

#include <list>
#include "DistRoundRobin.h"
#include<string>
#include<sstream>
#include<math.h>
#include <list>
#include<limits>
#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <set>
#include <xercesc/dom/DOM.hpp>
#include "helper.h"
#include<tr1/unordered_map>

using namespace VDRIVE;
using namespace std;
using namespace xercesc;

//#define hashPosition

DistRoundRobin::DistRoundRobin(int argc, char** argv) : Distributor() {
    int argInd = 0;
    numDisks = 0;
    extentsize = 0;
    disks = 0;
    useHash = false;
    useIM = false;

    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-hash") == 0) {
            useHash = true;
        } else if (strcmp(argv[argInd], "-im") == 0) {
            useIM = true;
        } else {
            cout << "unknown option: " << argv[argInd] << "\n";
            throw 1;
        }
    }
}

DistRoundRobin::DistRoundRobin(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode* n;
    xercesc::DOMElement* disk_pos;
    XMLCh* xmlString;
    char* sysString;
    std::set<uint64_t> positions;

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
    isst.str(sysString);
    isst >> this->copies;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("useHash");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        useHash = true;
    } else {
        useHash = false;
    }
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("useIM");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    if (strcmp(sysString, "true") == 0) {
        useIM = true;
    } else {
        useIM = false;
    }
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

    this->disks = new std::tr1::unordered_map<uint64_t, Disk*>();

    xmlString = XMLString::transcode("position");
    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            uint64_t pos = 0;
            disk_pos = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(disk_pos->getAttribute(xmlString));
            pos = (uint64_t) readInt64_t(sysString, 10);
            XMLString::release(&sysString);
            DOMNode *m = disk_pos->getFirstChild();

            while ((m != 0) && (m->getNodeType() != DOMNode::ELEMENT_NODE))
                m = m->getNextSibling();

            if (m == 0) {
                throw 1;
            }

            Disk *d = new Disk((xercesc::DOMElement*)m);
            (*disks)[pos] = d;
            if (useIM)
                positions.insert(pos);
        }
    }
    XMLString::release(&xmlString);

    numDisks = disks->size();

    if (useIM) {
        map = new ImprovedMap(positions.size(), &positions);
    }
}

DistRoundRobin::DistRoundRobin(const DistRoundRobin& orig) {
    this->extentsize = orig.extentsize;
    this->copies = orig.copies;
    this->numDisks = orig.numDisks;
    this->useHash = orig.useHash;
    this->useIM = orig.useIM;
    this->disks = new std::tr1::unordered_map<uint64_t, Disk*>();
    for (std::tr1::unordered_map<uint64_t, Disk*>::iterator it = orig.disks->begin(); it != orig.disks->end(); it++) {
        (*disks)[it->first] = new Disk(*(it->second));
    }
}

DistRoundRobin::~DistRoundRobin() {
    for (std::tr1::unordered_map<uint64_t, Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        delete it->second;
    }
    delete disks;
}

void DistRoundRobin::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->copies = copies;
    this->extentsize = extentsize;
    setDisks(disks);
}

void DistRoundRobin::setDisks(std::list<Disk*>* disks) {
    if (this->disks != 0) {
        for (std::tr1::unordered_map<uint64_t, Disk*>::iterator it = this->disks->begin(); it != this->disks->end(); it++) {
            delete it->second;
        }
        delete this->disks;
    }
    this->disks = new std::tr1::unordered_map<uint64_t, Disk*>();
    this->numDisks = disks->size();
    uint64_t id = 0;
    uint64_t distance = std::numeric_limits<uint64_t>::max() / numDisks;
    if (useIM)
        id = distance / 2;
    std::set<uint64_t> positions;
    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        (*(this->disks))[id] = new Disk(**it);
        if (useIM) {
            positions.insert(id);
            id += distance;
        } else {
            id++;
        }
    }
    if (useIM)
        map = new ImprovedMap(positions.size(), &positions);
}

std::list<Disk*>* DistRoundRobin::placeExtent(int64_t virtualVolumeId, int64_t position)  {
    if (useHash)
        position = hashPosition(position);
    if (useIM) {
        // TODO: ensure a stable distribution if using a ImprovedMap
        // if using a ImprovedMap in Round Robin we can get different
        // copies before and after loading the Distributor because the order
        // of the elements in disks is not stable.
        position = map->findClosest(position);
        std::tr1::unordered_map<uint64_t, Disk*>::iterator it = disks->find(position);
        std::list<Disk*>* result = new std::list<Disk*>();
        for (int i = 0; i < copies; i++) {
            Disk *d = it->second;
            result->push_back(new Disk(*d));
            it++;
            if (it == disks->end()) {
                it = disks->begin();
            }
        }
        return result;
    } else {
        uint64_t firstDisk = ((position + virtualVolumeId) * copies) % numDisks;
        std::list<Disk*>* result = new std::list<Disk*>();
        for (int32_t i = 0; i < copies; i++) {
            result->push_back(new Disk(*(*disks)[(firstDisk + i) % numDisks]));
        }
        return result;
    }
}

int64_t DistRoundRobin::hashPosition(int64_t position) const {
    std::stringstream out;
    out << position;
    string s = out.str();
    return hashFunctionInt64(&s);
}

std::list<Disk*>* DistRoundRobin::getDisks() const {
    std::list<Disk*>* result = new std::list<Disk*>();
    for (std::tr1::unordered_map<uint64_t, Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        result->push_back(new Disk(*(it->second)));
    }
    return result;
}

xercesc::DOMElement* DistRoundRobin::toXML(xercesc::DOMDocument* doc) const {
    DOMElement* result;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2, *xmlString3;

    xmlString = XMLString::transcode(DistRoundRobin::getXMLRootType().c_str());
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

    if (useHash) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("useHash");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    if (useIM) {
        xmlString = XMLString::transcode("true");
    } else {
        xmlString = XMLString::transcode("false");
    }
    xmlString2 = XMLString::transcode("useIM");
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

    xmlString = XMLString::transcode("diskPosition");
    xmlString2 = XMLString::transcode("position");
    for (std::tr1::unordered_map<uint64_t, Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        DOMElement* pos = doc->createElement(xmlString);
        out << it->first;
        xmlString3 = XMLString::transcode(out.str().c_str());
        pos->setAttribute(xmlString2, xmlString3);
        XMLString::release(&xmlString3);
        out.str("");
        out.clear();
        pos->appendChild(it->second->toXML(doc));
        result->appendChild(pos);
    }
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    return result;
}
