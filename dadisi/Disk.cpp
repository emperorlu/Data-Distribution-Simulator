/* 
 * File:   Disk.cpp
 * Author: fermat
 * 
 * Created on 20. Januar 2010, 10:39
 */

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <sqlite3.h>
#include <cstdio>

#include "Disk.h"
#include "helper.h"

using namespace VDRIVE;
using namespace xercesc;
using namespace std;

Disk::Disk(int64_t id, int64_t capacity, void* data) {
    this->id = id;
    this->capacity = capacity;
    this->data = data;
}

Disk::Disk(const Disk& orig) {
    this->id = orig.id;
    this->capacity = orig.capacity;
    this->data = orig.data;
}

Disk::Disk(xercesc::DOMElement* data) {
    std::istringstream isst;
    XMLCh* xmlString;
    char* sysString;

    xmlString = XMLString::transcode("id");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->id;
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

    //TODO private_data
}

Disk::~Disk() {
}

void* Disk::getData() const {
    return this->data;
}

void Disk::setData(void* data) {
    this->data = data;
}

int64_t Disk::getId() const {
    return this->id;
}

void Disk::setId(int64_t id) {
    this->id = id;
}

int64_t Disk::getCapacity() const {
    return this->capacity;
}

void Disk::setCapacity(int64_t capacity) {
    this->capacity = capacity;
}

xercesc::DOMElement* Disk::toXML(xercesc::DOMDocument* doc) {
    xercesc::DOMElement* result;
    std::stringstream out;
    XMLCh *xmlString, *xmlString2;

    xmlString = XMLString::transcode(Disk::getXMLRootType().c_str());
    result = doc->createElement(xmlString);
    XMLString::release(&xmlString);

    out << this->id;
    xmlString = XMLString::transcode(out.str().c_str());
    xmlString2 = XMLString::transcode("id");
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

    //TODO: Do anything with privateData

    return result;
}

#ifndef no_sqlite

void Disk::storeDiskListDBFile(std::list<Disk*>* disks, std::string filename) {
    sqlite3 *db;
    std::stringstream out;

    db = createDB(filename);

    execQuery(db, string("insert into meta values('filetype', 'disklist')"));
    execQuery(db, string("insert into meta values('table', 'disklist')"));

    execQuery(db, string("create table disklist_meta (key text, value text)"));
    execQuery(db, string("insert into meta values('tabletype', 'disklist')"));
    out << "insert into disklist_meta values ('count', " << disks->size() << ")";
    execQuery(db, string(out.str()));
    out.str("");

    execQuery(db, string("create table disklist (id integer, capacity integer)"));
    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        out << "insert into disklist values (" << (*it)->getId() << ", " << (*it)->getCapacity() << ")";
        execQuery(db, string(out.str()));
        out.str("");
    }

    sqlite3_close(db);
}

std::list<Disk*> *Disk::loadDiskListDBFile(std::string filename) {
    sqlite3 *db;
    sqlite3_stmt *query;
    int error;
    std::list<Disk*> *result = new std::list<Disk*>();

    db = loadDB(filename);

    error = sqlite3_prepare(db, "select id,capacity from disklist", -1, &query, 0);
    if (error) {
        fprintf(stderr, "Could not read disklist: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(query);
        throw "Could not read disklist";
    }

    error = sqlite3_step(query);
    while (error == SQLITE_ROW) {
        Disk *d = new Disk(sqlite3_column_int64(query, 0), sqlite3_column_int64(query, 1), 0);
        result->push_back(d);
        error = sqlite3_step(query);
    }

    if (error != SQLITE_DONE) {
        fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(query);
        throw "Error executing query";
    }

    sqlite3_finalize(query);

    return result;
}
#endif

void Disk::storeDiskList(std::list<Disk*>* disks, std::string filename) {
    XMLCh* xmlString;
    try {
        XMLPlatformUtils::Initialize();
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
        abort();
    }

    try {
        xmlString = XMLString::transcode("Core");
        DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(xmlString);
        XMLString::release(&xmlString);
        xmlString = XMLString::transcode("disklist");
        DOMDocument* doc = impl->createDocument(0, xmlString, 0);
        XMLString::release(&xmlString);
        DOMElement* rootElem = doc->getDocumentElement();
        for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
            Disk* disk = *it;
            DOMElement* xml = disk->toXML(doc);
            rootElem->appendChild(xml);
        }
        DOMLSSerializer *theSerializer = ((DOMImplementationLS*) impl)->createLSSerializer();
        DOMLSOutput *theOutput = ((DOMImplementationLS*) impl)->createLSOutput();
        xmlString = XMLString::transcode(filename.c_str());
        LocalFileFormatTarget *myFormTarget = new LocalFileFormatTarget(xmlString);
        XMLString::release(&xmlString);
        theOutput->setByteStream(myFormTarget);
        theSerializer->write(doc, theOutput);
        delete theSerializer;
        delete theOutput;
        delete myFormTarget;
        delete doc;
        //delete xml;
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
        XMLPlatformUtils::Terminate();
        abort();
    } catch (const DOMException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "DOMError during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
        XMLPlatformUtils::Terminate();
        abort();
    }

    try {
        XMLPlatformUtils::Terminate();
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
        XMLPlatformUtils::Terminate();
        abort();
    }
}

std::list<Disk*>* Disk::loadDiskList(std::string filename) {
    xercesc::DOMNode *n;
    XMLCh* xmlString;
    std::list<Disk*>* disks = 0;
    char *sysString;

    try {
        XMLPlatformUtils::Initialize();
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
        return 0;
    }

    try {
        disks = new std::list<Disk*>();
        xmlString = XMLString::transcode("Core");
        DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(xmlString);
        XMLString::release(&xmlString);
        DOMLSParser *parser = ((DOMImplementationLS*) impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
        DOMDocument *doc = 0;
        DOMElement *rootElem, *diskElem;
        parser->resetDocumentPool();
        xmlString = XMLString::transcode(filename.c_str());
        doc = parser->parseURI(xmlString);
        XMLString::release(&xmlString);
        rootElem = doc->getDocumentElement();
        n = rootElem->getFirstChild();
        while (n != 0) {
            if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
                diskElem = (xercesc::DOMElement*) n;
                sysString = XMLString::transcode(diskElem->getTagName());
                if (strcmp(Disk::getXMLRootType().c_str(), sysString) == 0) {
                    Disk *disk = new Disk(diskElem);
                    disks->push_back(disk);
                }
                XMLString::release(&sysString);
            }
            n = n->getNextSibling();
        }
        delete parser;
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        if (disks != 0) {
            delete disks;
        }
        XMLString::release(&sysString);
        XMLPlatformUtils::Terminate();
        return 0;
    }

    try {
        XMLPlatformUtils::Terminate();
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
        XMLString::release(&sysString);
    }
    return disks;
}
