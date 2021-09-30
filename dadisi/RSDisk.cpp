/* 
 * File:   RSDisk.cpp
 * Author: fermat
 * 
 * Created on 20. Januar 2010, 16:42
 */

#include "RSDisk.h"
#include <sstream>
#include <string>
#include <ostream>
#include <xercesc/util/XMLString.hpp>
#include <iostream>

using namespace VDRIVE;
using namespace xercesc;

RSDisk::RSDisk(xercesc::DOMElement* data) {
    std::istringstream isst;
    XMLCh* xmlString;
    char* sysString;
    std::string *name;
    xercesc::DOMNode *m;

    sysString = XMLString::transcode(data->getTagName());
    name = new std::string(sysString);
    if (name->compare(getXMLRootType()) != 0) {
        std::cerr << "RSDisk should have RootType " << getXMLRootType() << " but got " << *name << "\n";
        abort();
    }
    XMLString::release(&sysString);
    delete name;

    xmlString = XMLString::transcode("rsId");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->rsId;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("cPosition");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->cPosition;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("kError");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->kError;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("checkC");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->setCheckC(mpq_class(sysString));
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("checkCStar");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    this->setCheckCStar(mpq_class(sysString));
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    m = data->getFirstChild();
    while ((m != 0) && (m->getNodeType() != DOMNode::ELEMENT_NODE)) {
        m = m->getNextSibling();
    }
    if (m == 0) {
        std::cerr << "RS " << this->rsId << " had no Disk\n";
        abort();
    }
    xercesc::DOMElement* diskE = (xercesc::DOMElement*) m;
    this->disk = new Disk(diskE);
}

RSDisk::RSDisk(Disk* disk) {
    this->disk = new Disk(*disk);
    this->alignedCapacity = disk->getCapacity();
    this->cPosition = 0;
    this->checkC = 0;
    this->checkCDouble = 0.0;
    this->checkCStar = 0;
    this->checkCStarDouble = 0.0;
    this->kError = -1;
    this->rsId = disk->getId();
}

RSDisk::RSDisk(const RSDisk& orig) {
    this->disk = new Disk(*(orig.disk));
    this->alignedCapacity = orig.alignedCapacity;
    this->cPosition = orig.cPosition;
    this->checkC = orig.checkC;
    this->checkCDouble = orig.checkCDouble;
    this->checkCStar = orig.checkCStar;
    this->checkCStarDouble = orig.checkCStarDouble;
    this->kError = orig.kError;
    this->rsId = orig.rsId;
}

RSDisk::~RSDisk() {
    delete this->disk;
}

xercesc::DOMElement* RSDisk::toXML(xercesc::DOMDocument* doc) {
    xercesc::DOMElement* result;
    std::stringstream out;
    XMLCh* xmlString;
    XMLCh* xmlString2;

    xmlString = XMLString::transcode(RSDisk::getXMLRootType().c_str());
    result = doc->createElement(xmlString);
    XMLString::release(&xmlString);
    xercesc::DOMElement* diskE = this->disk->toXML(doc);
    result->appendChild(diskE);

    out << this->rsId;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("rsId");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->cPosition;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("cPosition");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    out << this->kError;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("kError");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);
    out.str("");
    out.clear();

    xmlString = XMLString::transcode(this->checkC.get_str().c_str());
    xmlString2 = XMLString::transcode("checkC");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    xmlString = XMLString::transcode(this->checkCStar.get_str().c_str());
    xmlString2 = XMLString::transcode("checkCStar");
    result->setAttribute(xmlString2, xmlString);
    XMLString::release(&xmlString);
    XMLString::release(&xmlString2);

    return result;
}
