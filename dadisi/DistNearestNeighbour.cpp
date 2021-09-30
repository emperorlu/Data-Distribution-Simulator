/* 
 * File:   DistNearestNeighbour.cpp
 * Author: fermat
 * 
 * Created on 20. Januar 2010, 11:32
 */

#include "DistNearestNeighbour.h"
#include "ImprovedMap.h"
#include<string>
#include<sstream>
#include<math.h>
#include <list>
#include<limits>
#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>
#include "helper.h"

//#define printAreas

using namespace VDRIVE;
using namespace std;
using namespace xercesc;

DistNearestNeighbour::DistNearestNeighbour(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode* n;
    xercesc::DOMElement* disk_pos;
    XMLCh* xmlString;
    char* sysString;
    keys = new std::set<uint64_t > ();

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

    xmlString = XMLString::transcode("nnCopies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->nnCopies;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("cutFactor");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->cutFactor;
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

    std::tr1::unordered_map<int64_t, Disk*>* disks_by_id =
            new std::tr1::unordered_map<int64_t, Disk*>();
    this->disks = new std::list<Disk*>();
    diskDistribution = new VDRIVE_DNN_DDT();
    //std::set<uint64_t> *keys = new std::set<uint64_t>();

    xmlString = XMLString::transcode("position");
    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            int64_t pos = 0;
            disk_pos = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(disk_pos->getAttribute(xmlString));
            pos = readInt64_t(sysString, 10);
            XMLString::release(&sysString);
            DOMNode *m = disk_pos->getFirstChild();

            while ((m != 0) && (m->getNodeType() != DOMNode::ELEMENT_NODE))
                m = m->getNextSibling();

            if (m == 0) {
                throw 1;
            }

            Disk *d = new Disk((xercesc::DOMElement*)m);
            if (disks_by_id->count(d->getId()) == 0) {
                this->disks->push_back(d);
                (*disks_by_id)[d->getId()] = d;
            } else {
                int64_t id = d->getId();
                delete d;
                d = (*disks_by_id)[id];
            }
            (*diskDistribution)[pos] = d;
            keys->insert(pos);
        }
    }
    XMLString::release(&xmlString);

    delete disks_by_id;

    keyFinder = new ImprovedMap(nnCopies, keys);

#ifdef printAreas
    std::list<Disk*>* myDisks = this->disks;
    map<Disk*, uint64_t> areas;
    for (list<Disk*>::iterator it = myDisks->begin(); it != myDisks->end();
            ++it) {
        Disk* disk = *it;
        areas.insert(make_pair(disk, 0));
    }
    uint64_t before = 0;
    for (set<uint64_t>::iterator it = (*keys).begin(); it != (*keys).end();
            ++it) {
        uint64_t value = *it;
        uint64_t area = value - before;
        before = value;
        VDRIVE_DNN_DDTit it2 = diskDistribution->find(value);
        Disk* d = 0;
        if (it2 != diskDistribution->end()) {
            d = it2->second;
        } else {
            std::cerr << "Could not find disk for key " << value << "\n";
            throw "Error in implementation";
        }
        map<Disk*, uint64_t>::iterator it3 = areas.find(d);
        uint64_t oldValue = it3->second;
        it3->second = oldValue + area;
    }

    uint64_t firstKey = (*(keys->begin()));
    Disk* disk = (*diskDistribution)[firstKey];
    uint64_t oldValue = areas[disk];
    uint64_t value = std::numeric_limits<uint64_t>::max() - before;
    areas[disk] = oldValue + value;

    uint64_t sum = 0;
    for (map<Disk*, uint64_t>::iterator it = areas.begin(); it != areas.end();
            ++it) {
        pair<Disk*, uint64_t> elem = *it;
        double tmp = elem.second * 1.0 / std::numeric_limits<uint64_t>::max();
        sum += elem.second;
        cout << elem.first->getId() << ";" << tmp << "\n";
    }
    cout << "Together: " << sum << "\n";
    if (sum != std::numeric_limits<uint64_t>::max()) {
        cout << "Sum is to small by " <<
                (std::numeric_limits<uint64_t>::max() - sum) << "\n";
    }
#endif
}

DistNearestNeighbour::DistNearestNeighbour(int argc, char** argv) :
Distributor() {
    this->staticNNCopies = false;
    this->nnCopies = 0;
    this->nnCopiesFactor = 400;
    this->diskDistribution = 0;
    this->disks = 0;
    int argInd;
    this->copies = 1;
    this->cutFactor = 0;

    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-cut") == 0) {
            long val;
            char* end;
            argInd++;
            if (argInd == argc) {
                throw "Option -cut needs a number";
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                throw "-cut has to be followed by a number";
            }
            if (val < 0) {
                throw "Negative cut factor?!?";
            }
            if (val > 2147483647) {
                throw "Maximum value for cut is 2147483647";
            }
            this->cutFactor = (uint32_t) val;
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
        } else {
            cout << "unknown option: " << argv[argInd] << "\n";
            throw 1;
        }
    }
    this->keys = new std::set<uint64_t > ();
}

DistNearestNeighbour::DistNearestNeighbour(bool staticNNCopies, int32_t nnCopies, int32_t nnCopiesFactor) : Distributor() {
    this->staticNNCopies = staticNNCopies;
    this->nnCopies = nnCopies;
    this->nnCopiesFactor = nnCopiesFactor;
    this->diskDistribution = 0;
    this->disks = 0;
    this->keys = new std::set<uint64_t > ();
    this->copies = 1;
}

DistNearestNeighbour::DistNearestNeighbour(const DistNearestNeighbour& orig) :
Distributor(orig) {
    this->nnCopies = orig.nnCopies;
    this->diskDistribution = orig.diskDistribution;
    this->disks = orig.disks;
    this->extentsize = orig.extentsize;
    this->copies = orig.copies;
    this->nnCopiesFactor = orig.nnCopiesFactor;
    keys = new std::set<uint64_t > ();
}

DistNearestNeighbour::~DistNearestNeighbour() {
    if (diskDistribution != 0)
        delete diskDistribution;
    if (this->disks != 0) {
        for (list<Disk*>::iterator it = this->disks->begin();
                it != this->disks->end(); it++) {
            Disk *d = *it;
            delete d;
        }
        delete this->disks;
    }
    delete keyFinder;
    if (keys != 0)
        delete keys;
}

std::list<Disk*>* DistNearestNeighbour::getDisks() const {
    if (this->disks == 0)
        return 0;
    list<Disk*> *resDisks = new list<Disk*>();
    for (list<Disk*>::iterator it = this->disks->begin();
            it != this->disks->end(); it++) {
        Disk *d = *it;
        resDisks->push_back(new Disk(*d));
    }
    return resDisks;
}

std::list<Disk*>* DistNearestNeighbour::placeExtent(int64_t virtualVolumeId,
        int64_t position)  {
    std::list<Disk*>* result = new std::list<Disk*>();
    uint64_t i = 0;
    while (result->size() < (uint32_t) copies) {
        uint64_t pos = hashFunction(virtualVolumeId, position, i);
        pos = keyFinder->findClosest(pos);
        //std::set<uint64_t>::iterator it = keys->lower_bound(pos);
        //it--;
        //if (*it == 0) {
        //    cout << "Beginn\n";
        //    pos = (*(keys->rbegin()));
        //} else {
        //    cout << "Not Beginn\n";
        //    pos = *it;
        //}
        if (diskDistribution->count(pos) == 0) {
            cout << "No Disk for position " << pos << "\n";
        }
        Disk *d = (*diskDistribution)[pos];
        int found = 0;
        for (list<Disk*>::iterator it = result->begin(); (it != result->end()) && (found == 0); it++) {
            Disk* existingDisk = *it;
            if (d->getId() == existingDisk->getId()) {
                found = 1;
            }
        }
        if (found == 0) {
            result->push_back(new Disk(*d));
        }
        i++;
    }
    return result;

    /*    uint64_t pos = hashFunction(virtualVolumeId, position);
        for (set<uint64_t>::iterator it = keys.begin(); it != keys.end(); ++it) {
            uint64_t border = *it;
            if (pos < border) {
                Disk* disk = diskDistribution->find(border)->second;
                result->push_back(new Disk(*disk));
                return result;
            }
        }
        set<uint64_t>::iterator it = keys.begin();
        if (it == keys.end()) {
            cerr << "NO KEYS!!!!!!\n";
        }
        uint64_t border = *it;
        Disk* disk = diskDistribution->find(border)->second;
        //Disk* disk = diskDistribution->begin()->second;
        result->push_back(new Disk(*disk));
        return result;
     */
}

void DistNearestNeighbour::setConfiguration(std::list<Disk*>* disks,
        int64_t extentsize, int32_t copies) {
    this->extentsize = extentsize;
    this->copies = copies;
    setDisks(disks);
}

void DistNearestNeighbour::setDisks(std::list<Disk*>* disks) {
    if (disks == 0)
        return;
    VDRIVE_DNN_MULT *multiplicity = new VDRIVE_DNN_MULT();
    if (this->cutFactor == 0) {
        for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end();
                it++) {
            (*multiplicity)[(*it)->getId()] = 1;
        }
    } else {
        for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end();
                it++) {
            int32_t multi = (*it)->getCapacity() / this->cutFactor;
            if (((*it)->getCapacity() % this->cutFactor) != 0)
                multi++;
            (*multiplicity)[(*it)->getId()] = multi;
        }
    }

    setDisksMulti(disks, multiplicity);
    delete multiplicity;
}

void DistNearestNeighbour::setDisksMulti(std::list<Disk*>* disks,
        VDRIVE_DNN_MULT *multiplicity) {
    //std::set<uint64_t> *keys = new std::set<uint64_t>;
    if (keys == 0) {
        keys = new std::set<uint64_t > ();
    } else {
        keys->clear();
    }
    if (diskDistribution != 0)
        delete diskDistribution;
    if (this->disks != 0) {
        for (list<Disk*>::iterator it = this->disks->begin();
                it != this->disks->end(); it++) {
            delete *it;
        }
        delete this->disks;
    }

    nnCopies = getNNCopies(disks);
    diskDistribution = new VDRIVE_DNN_DDT();
    std::list<Disk*>* myDisks = new std::list<Disk*>;
    this->disks = myDisks;

    //cout << "Will place " << copies << " copies of each disk\n";
    for (list<Disk*>::iterator it = disks->begin(); it != disks->end(); ++it) {
        if ((*multiplicity)[(*it)->getId()] > 0) {
            Disk* disk = new Disk(**it);
            myDisks->push_back(disk);
            int64_t c = nnCopies * (*multiplicity)[disk->getId()];
            for (int64_t i = 0; i < c; i++) {
                uint64_t value = hashFunction(disk, i);
                if (diskDistribution->find(value) == diskDistribution->end()) {
                    diskDistribution->insert(make_pair(value, disk));
                } else {
                    //cout << "Hit!\n";
                }
            }
        }
    }
    for (VDRIVE_DNN_DDTit it = diskDistribution->begin();
            it != diskDistribution->end(); ++it) {
        uint64_t key = it->first;
        (*keys).insert(key);
    }
    if (myDisks->empty()) {
        std::cerr <<
                "Warning: You initialized Nearest Neighbour without " <<
                "giving it a disk!!!\n";
    }

    keyFinder = new ImprovedMap(nnCopies, keys);

#ifdef printAreas
    map<Disk*, uint64_t> areas;
    for (list<Disk*>::iterator it = myDisks->begin(); it != myDisks->end();
            ++it) {
        Disk* disk = *it;
        areas.insert(make_pair(disk, 0));
    }
    uint64_t before = 0;
    for (set<uint64_t>::iterator it = (*keys).begin(); it != (*keys).end();
            ++it) {
        uint64_t value = *it;
        uint64_t area = value - before;
        before = value;
        VDRIVE_DNN_DDTit it2 = diskDistribution->find(value);
        Disk* d = 0;
        if (it2 != diskDistribution->end()) {
            d = it2->second;
        } else {
            std::cerr << "Could not find disk for key " << value << "\n";
            throw "Error in implementation";
        }
        map<Disk*, uint64_t>::iterator it3 = areas.find(d);
        uint64_t oldValue = it3->second;
        it3->second = oldValue + area;
    }

    uint64_t firstKey = (*(keys->begin()));
    Disk* disk = (*diskDistribution)[firstKey];
    uint64_t oldValue = areas[disk];
    uint64_t value = std::numeric_limits<uint64_t>::max() - before;
    areas[disk] = oldValue + value;

    uint64_t sum = 0;
    for (map<Disk*, uint64_t>::iterator it = areas.begin(); it != areas.end();
            ++it) {
        pair<Disk*, uint64_t> elem = *it;
        double tmp = elem.second * 1.0 / std::numeric_limits<uint64_t>::max();
        sum += elem.second;
        cout << elem.first->getId() << ";" << tmp << "\n";
    }
    cout << "Together: " << sum << "\n";
    if (sum != std::numeric_limits<uint64_t>::max()) {
        cout << "Sum is to small by " <<
                (std::numeric_limits<uint64_t>::max() - sum) << "\n";
    }
#endif
    keys->insert(0);
    //delete keys;
}

uint64_t DistNearestNeighbour::hashFunction(int64_t virtualVolumeId,
        int64_t blockposition, int64_t index) const {
    std::stringstream buf;
    buf << "NearestNeighbour::" << virtualVolumeId << "::" << blockposition << "::" << index;
    std::string s = buf.str();
    return Distributor::hashFunctionInt64(&s);
}

uint64_t DistNearestNeighbour::hashFunction(Disk* disk,
        int64_t iteration) const {
    std::stringstream buf;
    buf << "NearestNeighbour::" << disk->getId() << "::" << iteration;
    std::string s = buf.str();
    return Distributor::hashFunctionInt64(&s);
}

int32_t DistNearestNeighbour::getNNCopies(std::list<Disk*>* disks) const {
    if (staticNNCopies) {
        return nnCopies;
    }
    if (disks == 0)
        return 0;
    if (disks->size() < 2) {
        return 1;
    }
    //unsigned int number = disks.size();
    return (int32_t) nnCopiesFactor * (int32_t) ceil(log2(disks->size() * 1.0));
}

xercesc::DOMElement* DistNearestNeighbour::toXML(DOMDocument* doc) const {
    DOMElement* result;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2, *xmlString3;

    xmlString =
            XMLString::transcode(DistNearestNeighbour::getXMLRootType().c_str());
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

    out << this->nnCopies;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("nnCopies");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->cutFactor;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("cutFactor");
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
    for (VDRIVE_DNN_DDTit it = diskDistribution->begin();
            it != diskDistribution->end(); it++) {
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
