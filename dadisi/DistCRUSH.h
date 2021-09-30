/* 
 * File:   DistCRUSH.h
 * Author: ywkang
 *
 * Created on March 16, 2010, 7:42 PM
 */

#ifndef _DISTCRUSH_H
#define	_DISTCRUSH_H

#include "Distributor.h"

#include <set>
#include <map>
#include <list>
#include <iostream>

#include "crushwrapper.h"
#include <xercesc/dom/DOMElement.hpp>

using namespace std;
using namespace xercesc;


namespace VDRIVE {
    class DistCRUSH : public Distributor {
    public:
        DistCRUSH(int argc, char** argv);
        DistCRUSH(const DistCRUSH& orig);
        DistCRUSH(xercesc::DOMElement* data);
        static std::string getXMLRootType() {
            return std::string("Crush");
        }
        virtual ~DistCRUSH();
    private:
        CrushWrapper crush;
        int copies;
        int ruleno;
        int failuredomains;
        int extentsize;
        list<Disk*> alldisks;
        vector<uint32_t> disk_weights;

        struct layer_t {
          string name;
          int buckettype;
          int size;
        };

        vector<layer_t> layers;
        
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position);
        //virtual void setClusters(list<list<Disk*>*>* clusters, list<uint32_t>* weights);
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies);
        virtual void setDisks(std::list<Disk*>* disks);
        virtual std::list<Disk*>* getDisks() const;
        virtual int64_t getExtentsize() const;
        virtual int32_t getCopies() const;
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const;
        virtual void buildCrushMap(vector<layer_t> layers, std::list<Disk*>* disks);
        double hash(int64_t key1, int64_t key2, int64_t key3) const;
        

    };
}


#endif	/* _DISTCRUSH_H */

