/* 
 * File:   DistNearestNeighbour.h
 * Author: fermat
 *
 * Created on 20. Januar 2010, 11:32
 */

#ifndef _DISTNEARESTNEIGHBOUR_H
#define	_DISTNEARESTNEIGHBOUR_H

#include"Distributor.h"
#include"ImprovedMap.h"
#include<set>
#include<map>
#include<tr1/unordered_map>

#define VDRIVE_DNN_DDT std::tr1::unordered_map<uint64_t, Disk*>
#define VDRIVE_DNN_DDTit std::tr1::unordered_map<uint64_t, Disk*>::iterator
#define VDRIVE_DNN_MULT std::tr1::unordered_map<uint64_t, int32_t>
#define VDRIVE_DNN_MULTit std::tr1::unordered_map<uint64_t, int32_t>::iterator

namespace VDRIVE {

    /**
     * Implementation of the Nearest Neighbour strategy (same as Consistent
     * Hashing)
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class DistNearestNeighbour : public Distributor {
    public:

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a DistNearestNeighbour.
         */
        DistNearestNeighbour(xercesc::DOMElement* data);

        /**
         * generate a new, uninitialized Nearest Neighbor Implementation.
         */
        DistNearestNeighbour(int argc, char** argv);

        /**
         * generate a new, uninitialized Nearest Neigbour Implementation which
         * will place nnCopies of each Disk on the ring.
         *
         * @param nnCopies The number of times each disk shall be placed on the ring.
         */
        DistNearestNeighbour(bool staticNNCopies, int32_t nnCopies, int32_t nnCopiesFactor);

        /**
         * copy constructor
         *
         * @param orig original DistNearestNeighbour
         */
        DistNearestNeighbour(const DistNearestNeighbour& orig);

        /**
         * Destructor
         */
        virtual ~DistNearestNeighbour();

        /**
         * @see Distributor::placeExtent
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;

        /**
         * This function is used by Share to initialize a NearestNeighbour
         * where each Disk may be inserted several times if it surrounds the
         * ring.
         */
        virtual void setDisksMulti(std::list<Disk*>* disks, VDRIVE_DNN_MULT *multiplicity);

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
            return std::string("NearestNeighbour");
        }

    private:
        /**
         * ExtenSize as given to setConfiguration.
         */
        int64_t extentsize;

        /**
         * Number of copies to be distributed
         */
        int32_t copies;

        /**
         * Number of copies to created from each disk. This has nothing to to
         * with the number of disks to be returned by getExtent.
         */
        int32_t nnCopies;

        /**
         * Factor of copies to be placed of each disk if not using a static factor
         */
        int32_t nnCopiesFactor;

        /**
         * cut each Disk by this Factor into virtual disks and place therefore
         * more copies of the disks in the intervall. 0 means do not cut.
         */
        uint64_t cutFactor;

        /**
         * the disks as given to setDisks (or setConfiguration)
         */
        std::list<Disk*>* disks;

        /**
         * This map is used to find disks in O(1).
         */
        ImprovedMap *keyFinder;

        /**
         * All keys, where a copy is placed.
         */
        std::set<uint64_t> *keys;

        /**
         * map to find the placed disk for a key out of keys.
         *
         * @see keys
         */
        VDRIVE_DNN_DDT *diskDistribution;

        /**
         * if this Flag is set (by calling the right Constructor) Nearest
         * Neigbour does not decide itself how many copies of each disks are created.
         */
        bool staticNNCopies;

        /**
         * Find a place to place the extent. The parameters are just driven
         * from placeExtent.
         *
         * @param virtualVolumeId @see Distributor::placeExtent
         *
         * @param blockposition @see Distributor::placeExtent
         *
         * @return a value between 0 and 1 to place the Extent at.
         */
        uint64_t hashFunction(int64_t virtualVolumeId, int64_t blockposition, int64_t index) const;

        /**
         * Place a copy of a disk.
         *
         * @param The Disk to be placed
         *
         * @param the copy to be placed (needed to generate unique placements)
         *
         * @return a value between 0 and 1 to place the Disk at.
         */
        uint64_t hashFunction(Disk* disk, int64_t iteration) const;

    protected:
        /**
         * Get the number of copies of each Disk to be placed. To get a failure
         * of epsilon you need to place
         *
         * \f[\frac{1}{\epsilon^2}\cdot \mbox{log}_2(n)\f]
         *
         * copies of each disk, where n is the number of disks. This method may
         * be overwritten by drived classes to use other values. If the object
         * was created with a static nnCount, this one will be given back.
         *
         * @param disks the disks that shall be placed
         *
         * @return number of copies of each Disk to be placed.
         */
        virtual int32_t getNNCopies(std::list<Disk*>* disks) const;
    };
}
#endif	/* _DISTNEARESTNEIGHBOUR_H */

