/* 
 * File:   DistRoundRobin.h
 * Author: fermat
 *
 * Created on 14. April 2010, 11:17
 */

#ifndef _DISTROUNDROBIN_H
#define	_DISTROUNDROBIN_H

#include "Distributor.h"
#include "ImprovedMap.h"
#include<tr1/unordered_map>

namespace VDRIVE {

    /**
     * Round Robin is an very simple distribution: The disks are arranged in
     * an 0..(numDisks-1) interval and each extend is placed beginning at disk
     * ((position + virtualVolumeId) * copies) % numDisks at the following
     * copies disks. This algorithm is mostly for testing and is absolutly not
     * adaptive. :-)
     */
    class DistRoundRobin : public Distributor {
    public:
        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a DistRoundRobin.
         */
        DistRoundRobin(xercesc::DOMElement* data);

        /**
         * generate a new, uninitialized Round Robin Implementation.
         */
        DistRoundRobin(int argc, char** argv);

        /**
         * copy constructor
         *
         * @param orig original DistRoundRobin
         */
        DistRoundRobin(const DistRoundRobin& orig);

        /**
         * Destructor
         */
        virtual ~DistRoundRobin();

        /**
         * @see Distributor::placeExtent
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;

        /**
         * @see Distributor::setConfiguration
         */
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies);

        /**
         * @see Distributor::setDisks
         */
        virtual void setDisks(std::list<Disk*>* disks);

        /**
         * @see Distributor::toXML
         */
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const;

         /**
         * @see Distributor::getDisks
         */
        virtual std::list<Disk*>* getDisks() const;

        /**
         * @see Distributor::getExtentsize
         */
        virtual int64_t getExtentsize() const {
            return extentsize;
        }

        /**
         * @see Distributor::getCopies
         */
        virtual int32_t getCopies() const {
            return copies;
        }

        /**
         * Get the Root-Type of XML-Elements representing this class.
         *
         * @return the Root-Type of XML-Elements representing this class.
         */
        static std::string getXMLRootType() {
            return std::string("RoundRobin");
        }
    private:
        /**
         * ExtenSize as given to setConfiguration.
         */
        int64_t extentsize;

        /**
         * number of copies to be distributed.
         */
        int32_t copies;

        /**
         * shall the Positions in placeExtent been hashed or not?
         */
        bool useHash;

        /**
         * shall the Positions be stored in an IM? 
         */
        bool useIM;

        /**
         * map to be used if useIM is true
         */
        ImprovedMap* map;
        
        /**
         * the disks as given to setDisks (or setConfiguration)
         */
        std::tr1::unordered_map<uint64_t, Disk*>* disks;

        /**
         * number of Disks hold by this Instance.
         */
        uint64_t numDisks;

        /**
         * hash Position to somewhere else
         */
        int64_t hashPosition(int64_t position) const;
    };
}
#endif	/* _DISTROUNDROBIN_H */

