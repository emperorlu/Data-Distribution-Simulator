/* 
 * File:   DistShare.h
 * Author: fermat
 *
 * Created on 24. März 2010, 18:12
 */

#ifndef _DISTSHARE_H
#define	_DISTSHARE_H

#include"Distributor.h"
#include "DistNearestNeighbour.h"
#include<set>
#include<gmpxx.h>
#include"DistNearestNeighbour.h"
#include "ImprovedMap.h"

namespace VDRIVE {

    /**
     * DistShare implemts the Share-Stragegy. This strategy is able to
     * distribute non redundant Extents over heterogeneous disks.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class DistShare : public Distributor {
    public:

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a DistNearestNeighbour.
         */
        DistShare(xercesc::DOMElement* data);

        /**
         * generate a new, uninitialized Nearest Neighbor Implementation.
         */
        DistShare(int argc, char** argv);

        /**
         * copy constructor
         *
         * @param orig original DistNearestNeighbour
         */
        DistShare(const DistShare& orig);

        /**
         * Destructor
         */
        virtual ~DistShare();

        /**
         * @see Distributor::placeExtent
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;

        /**
         * @see Distributor::setConfiguration
         *
         * Share does not support copies, so copies is ignored and automatically set to 1.
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
         * @see Distributor::setBaseMessage
         *
         * Overridden, so changing the baseMessage in DistShare also changes
         * the baseMessage in the underlying NearestNeighbour implementations.
         */
        virtual void setBaseMessage(uint8_t* baseMessage);

        /**
         * @see Distributor::getCopies
         *
         * Share does not support copies, so this is always 1.
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
            return std::string("Share");
        }

        /**
         * only used internelly, but has to be public to be used by pThreads.
         *
         * This Method can be called in an own thread to initialize the
         * Nearest Neighbour Implementations multi threaded.
         */
        void __runInitThread(uint32_t threadID);

    private:

        struct NNInitWorkPackage {
            std::list<Disk*> *disks;
            VDRIVE_DNN_MULT *multi;
            uint64_t pos;
        };

        std::list<NNInitWorkPackage*>* workPackages;

        pthread_rwlock_t* workPackageLock;

        pthread_rwlock_t* workPackageResultLock;

        /**
         * Number of copies to be placed.
         */
        int32_t copies;

        /**
         * extentsize as given to setConfiguration
         */
        int32_t extentsize;

        /**
         * disks as given to setDisks (or setConfiguration)
         */
        std::list<Disk*>* disks;

        /**
         * Here we find for each border the corresponding NearestNeighbour.
         */
        std::tr1::unordered_map<uint64_t, DistNearestNeighbour*>* distributors;

        /**
         * sum of the capacity of all disks
         */
        uint64_t systemCapacity;

        /**
         * This map is used to find disks in O(1).
         */
        ImprovedMap *keyFinder;

        /**
         * constant to set the StretchFactor, defaults to 3
         * (enough to cover the whole ring, to less to get a fair distribution).
         */
        uint32_t stretchFactorConst;

        /**
         * if this ia a value greter zero it will be used as Stretchfactor,
         * without any log(n) factor.
         */
        uint32_t staticStretchFactor;

        /**
         * true if NearestNeighbour shall use a static number of copies
         */
        bool staticNNCopies;

        /**
         * Number of copies each NearestNeighbour shall use
         */
        int32_t nnCopies;

        /**
         * Factor of copies that shall be used by each NN
         */
        int32_t nnCopiesFactor;

        /**
         * Number of copies to be placed of each disk in Share
         * (not in NearestNeighbour, defaults to 1)
         */
        int32_t shareCopies;

        /**
         * If this is set to true the copies generated by NearestNeighbour,
         * not by Share. Therefore the Strechfactor will be extended by k.
         */
        bool copiesByNN;

        /**
         * number of copies of each disk to throw on the ring. Share defines
         * this as 1 but the implementation of Christian is working another
         * way.
         */
        virtual int32_t getShareCopies(const std::list<Disk*>* disks);

        /**
         * calculate the factor to stretch each disk. According to Kays PhD
         * thesis this should be
         *
         * \f[3*log(N)\f]
         *
         * @param disks The disks to be stretched
         *
         * @return The factor to stretch the disks.
         */
        virtual double getStretchFactor(const std::list<Disk*>* disks);

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

        /**
         * Find a place to place the extent. The parameters are just driven
         * from placeExtent.
         *
         * @param virtualVolumeId @see Distributor::placeExtent
         *
         * @param blockposition @see Distributor::placeExtent
         *
         * @param index a running index to place the different copies
         *
         * @return a value between 0 and 1 to place the Extent at.
         */
        uint64_t hashFunction(int64_t virtualVolumeId, int64_t blockposition, int64_t index) const;

        /**
         * Each Disk is placed in all NearestNeighbour-Object at the same place.
         * Therefore we have to hash the position used in NearestNeighbour to
         * place copies.
         *
         * @param virtualVolumeId @see Distributor::placeExtent
         *
         * @param blockposition @see Distributor::placeExtent
         *
         * @param index a running index to place the different copies
         *
         * @return a value between 0 and 1 to place the Extent at.
         */
        uint64_t hashFunctionNN(int64_t virtualVolumeId, int64_t blockposition, int64_t index) const;
    };

}
#endif	/* _DISTSHARE_H */

