/* 
 * File:   Distributor.cpp
 * Author: fermat
 * 
 * Created on 20. Januar 2010, 10:34
 */
//#define no_gcrypt

#include "Distributor.h"
#include "DistNearestNeighbour.h"
#include "DistRedundantShare.h"
#include "DistFastRedundantShare.h"
#include "DistShare.h"
#ifndef no_gcrypt
#include <gcrypt.h>
#endif
#include <limits>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include "SHA.h"
#include "DistRoundRobin.h"
#include "DistCRUSH.h"
#include "DistRUSHp.h"
#include "DistRUSHr.h"
#include "DistRUSHt.h"
#include "DistRandSlice.h"
#include <cstring>
#include <errno.h>

using namespace VDRIVE;
using namespace xercesc;
using namespace std;

#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

//uint8_t* Distributor::baseMessage = new uint8_t[64];
//memset(Distributor::baseMessage, 0, 64);
//int blablub = Distributor::__init();

#ifndef no_gcrypt
GCRY_THREAD_OPTION_PTHREAD_IMPL;
gcry_error_t my_gcry_error = gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
#endif

Distributor::Distributor() {
    baseMessage = new uint8_t[64];
    memset(baseMessage, 0, 64);
    numThreads = 1;
    internelHash = true;
    gcryptHashAlgorithm = 0;
    internalHashAlgorithm = HASH_SHA1;
    a = 5000000000000000000;
    a += 5000000000000000051;
    b = 13;
}

Distributor::~Distributor() {
    delete[] baseMessage;
}

Distributor* Distributor::createDistributor(int32_t distributor, int argc, char** argv) {
    Distributor* dist = 0;
    if (distributor == SHARE) {
        dist = new DistShare(argc, argv);
    } else if (distributor == NEAREST_NEIGHBOUR) {
        dist = new DistNearestNeighbour(argc, argv);
    } else if (distributor == REDUNDANT_SHARE_K) {
        dist = new DistFastRedundantShare(argc, argv);
    } else if (distributor == REDUNDANT_SHARE) {
        dist = new DistRedundantShare(argc, argv);
    } else if (distributor == ROUND_ROBIN) {
        dist = new DistRoundRobin(argc, argv);
    } else if (distributor == RUSH_P) {
        dist = new DistRUSHp(argc, argv);
    } else if (distributor == CRUSH) {
        dist = new DistCRUSH(argc, argv);
    } else if (distributor == RUSH_R) {
        dist = new DistRUSHr(argc, argv);
    } else if (distributor == RUSH_T) {
        dist = new DistRUSHt(argc, argv);
    } else if (distributor == RAND_SLICE){
        dist = new DistRandSlice(argc, argv);
    } else {
        throw 1;
    }
    return dist;
}

void Distributor::setBaseMessage(uint8_t* baseMessage) {
    uint8_t *newMessage = new uint8_t[64];
    memcpy(newMessage, baseMessage, 64);
    uint8_t *oldBaseMessage = this->baseMessage;
    this->baseMessage = newMessage;
    delete[] oldBaseMessage;
}

uint8_t* Distributor::getBaseMessage() const {
    uint8_t *newMessage = new uint8_t[64];
    memcpy(newMessage, baseMessage, 64);
    return newMessage;
}

double Distributor::hashFunction(std::string* data) const {
    uint64_t result = hashFunctionInt64(data);

    if (result < 0) {
        result *= -1;
    }
    double end = result * 1.0 / std::numeric_limits<uint64_t>::max();
    return end;
}

uint64_t Distributor::myMTRand(std::string* data) const {
    uint8_t* message = (uint8_t*) data->c_str();
    uint16_t messageSize = data->size();
    uint8_t *newMessage = new uint8_t[4];
    memcpy(newMessage, baseMessage, 4);
    uint8_t* origMessage = message;
    uint64_t result = 0;
    uint32_t seed;
    uint32_t seed2;
    for (uint32_t i = 0; i < messageSize; i++) {
        uint32_t index = i % 4;
        newMessage[index] ^= origMessage[i];
    }
    uint32_t* tmp = (uint32_t*) newMessage;
    seed = tmp[0];
    seed2 = (1812433253UL * (seed ^ (seed >> 30)) + 1);

    seed ^= (seed >> 11);
    seed ^= (seed << 7) & 0x9d2c5680UL;
    seed ^= (seed << 15) & 0xefc60000UL;
    seed ^= (seed >> 18);

    seed2 ^= (seed2 >> 11);
    seed2 ^= (seed2 << 7) & 0x9d2c5680UL;
    seed2 ^= (seed2 << 15) & 0xefc60000UL;
    seed2 ^= (seed2 >> 18);

    result = seed;
    result <<= 32;
    result |= seed2;

    delete[] newMessage;

    return result;
}

uint64_t Distributor::myRand(std::string* data) const {
    uint8_t* message = (uint8_t*) data->c_str();
    uint16_t messageSize = data->size();
    uint8_t *newMessage = new uint8_t[4];
    memcpy(newMessage, baseMessage, 4);
    uint8_t* origMessage = message;
    uint64_t result = 0;
    uint32_t seed;
    for (uint32_t i = 0; i < messageSize; i++) {
        uint32_t index = i % 4;
        newMessage[index] ^= origMessage[i];
    }
    uint32_t* tmp = (uint32_t*) newMessage;
    seed = tmp[0];
    srand(seed);
    result = rand();
    result <<= 16;
    result = rand();
    result <<= 16;
    result = rand();
    result <<= 16;
    result = rand();
    delete[] newMessage;
    return result;
}

uint64_t Distributor::myLC(std::string* data) const {
    uint8_t* message = (uint8_t*) data->c_str();
    uint16_t messageSize = data->size();
    uint8_t *newMessage = new uint8_t[8];
    memcpy(newMessage, baseMessage, 8);
    uint8_t* origMessage = message;
    uint64_t result;
    for (uint32_t i = 0; i < messageSize; i++) {
        uint32_t index = i % 8;
        newMessage[index] ^= origMessage[i];
    }
    uint64_t* tmp = (uint64_t*) newMessage;
    result = tmp[0];
    result *= a;
    result += b;
    delete[] newMessage;
    return result;
}

uint64_t Distributor::mySHA1(std::string* data) const {
    uint8_t* message = (uint8_t*) data->c_str();
    uint16_t messageSize = data->size();
    uint8_t *newMessage = new uint8_t[64];
    memcpy(newMessage, baseMessage, 64);
    uint8_t* origMessage = message;
    uint64_t result;
    for (uint32_t i = 0; i < messageSize; i++) {
        uint32_t index = i % 64;
        newMessage[index] ^= origMessage[i];
    }
    if (SHA1(newMessage, &result) != shSuccess) {
        std::cerr << "Christians SHA implementation resulted in an error\n";
        abort();
    }
    delete[] newMessage;
    return result;
}

uint64_t Distributor::gcrypt(std::string* data) const {
    char* buffer = 0;
    char* run = 0;
    uint64_t result = 0;
    uint64_t mask = 0xFF;

    buffer = new char[gcry_md_get_algo_dlen(this->gcryptHashAlgorithm)];

    gcry_md_hash_buffer(this->gcryptHashAlgorithm, buffer, data->c_str(), data->size());

    run = buffer;
    for (int i = 0; i < 8; i++) {
        char j = *run;
        uint64_t tmp = j & 0xFF;
        tmp = tmp << (8 * i);
        tmp = tmp & mask;
        result = result | tmp;
        mask = mask << 8;
        run++;
    }
    delete[] buffer;
    return result;
}

uint64_t Distributor::hashFunctionInt64(std::string* data) const {
    if (internelHash) {
        switch (internalHashAlgorithm) {
            case HASH_SHA1:
                return mySHA1(data);
            case HASH_XOR:
                return myLC(data);
            case HASH_RAND:
                return myRand(data);
            case HASH_MT:
                return myMTRand(data);
        }
        throw 1;
    } else {
#ifndef no_gcrypt
        return gcrypt(data);
#else
        throw "GCrypt not supported in this compilation";
#endif
    }
}

void Distributor::save(std::string filename) const {
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
        xmlString = XMLString::transcode("distributor");
        DOMDocument* doc = impl->createDocument(0, xmlString, 0);
        XMLString::release(&xmlString);
        DOMElement* rootElem = doc->getDocumentElement();
        DOMElement* xml = this->toXML(doc);
        rootElem->appendChild(xml);
        DOMLSSerializer *theSerializer = ((DOMImplementationLS*) impl)->createLSSerializer();
        DOMConfiguration* serializerConfig = theSerializer->getDomConfig();
        if (serializerConfig->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
            serializerConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
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

Distributor* Distributor::loadDistributor(std::string filename) {
    XMLCh* xmlString;
    Distributor* dist;

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
        xmlString = XMLString::transcode("Core");
        DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(xmlString);
        XMLString::release(&xmlString);
        DOMLSParser *parser =
                ((DOMImplementationLS*) impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
        DOMDocument *doc = 0;
        DOMElement *rootElem, *distElem;
        DOMNode *n;
        char* distName;
        parser->resetDocumentPool();
        xmlString = XMLString::transcode(filename.c_str());
        doc = parser->parseURI(xmlString);
        XMLString::release(&xmlString);
        rootElem = doc->getDocumentElement();

        n = rootElem->getFirstChild();

        while ((n != 0) && (n->getNodeType() != DOMNode::ELEMENT_NODE))
            n = n->getNextSibling();

        if (n != 0) {
            distElem = (DOMElement*) n;
            distName = XMLString::transcode(distElem->getTagName());
            cout << distName << endl;
            if (strcmp(distName, DistRedundantShare::getXMLRootType().c_str()) == 0) {
                dist = new DistRedundantShare(distElem);
            } else if (strcmp(distName, DistNearestNeighbour::getXMLRootType().c_str()) == 0) {
                dist = new DistNearestNeighbour(distElem);
            } else if (strcmp(distName, DistShare::getXMLRootType().c_str()) == 0) {
                dist = new DistShare(distElem);
            } else if (strcmp(distName, DistFastRedundantShare::getXMLRootType().c_str()) == 0) {
                dist = new DistFastRedundantShare(distElem);
            } else if (strcmp(distName, DistRoundRobin::getXMLRootType().c_str()) == 0) {
                dist = new DistRoundRobin(distElem);
            } else if (strcmp(distName, DistCRUSH::getXMLRootType().c_str()) == 0) {
                dist = new DistCRUSH(distElem);
            } else if (strcmp(distName, DistRUSHp::getXMLRootType().c_str()) == 0) {
                dist = new DistRUSHp(distElem);
            } else if (strcmp(distName, DistRUSHr::getXMLRootType().c_str()) == 0) {
                dist = new DistRUSHr(distElem);
            } else if (strcmp(distName, DistRUSHt::getXMLRootType().c_str()) == 0) {
                dist = new DistRUSHt(distElem);
            } else {
                printf("Unknown Distributor: %s\n", distName);
                dist = 0;
            }
            XMLString::release(&distName);
        } else {
            dist = 0;
            cerr << "Distributor got in loaded File something that is no DOMElement\n";
            abort();
        }
        delete parser;
    } catch (const XMLException &toCatch) {
        char* sysString = XMLString::transcode(toCatch.getMessage());
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
                << "  Exception message:"
                << sysString << XERCES_STD_QUALIFIER endl;
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
    return dist;
}
