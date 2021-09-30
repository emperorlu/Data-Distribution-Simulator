/* 
 * File:   FastRedundantShare.h
 * Author: fermat
 *
 * Created on 27. März 2010, 12:18
 */

#ifndef _FASTREDUNDANTSHARE_H
#define	_FASTREDUNDANTSHARE_H

#include"DistRedundantShare.h"
#include "DistShare.h"
#include<limits>
#include<string>
#include<sstream>
#include<iostream>
#include"helper.h"

#define VDRIVE_FRS_KMAP std::tr1::unordered_map<int64_t, DistShare*>
#define VDRIVE_FRS_KMAPit VDRIVE_FRS_KMAP::iterator
#define VDRIVE_FRS_MMAP std::tr1::unordered_map<int32_t, VDRIVE_FRS_KMAP *>
#define VDRIVE_FRS_MMAPit VDRIVE_FRS_MMAP::iterator

#define VDRIVE_FRS_DISKMAP std::tr1::unordered_map<int64_t, Disk*>
#define VDRIVE_FRS_DISKMAPit VDRIVE_FRS_DISKMAP::iterator

namespace VDRIVE {

    /**
     * O(k) implementation of RedundantShare. Consumes much memory.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class DistFastRedundantShare : public Distributor {
    public:
        /**
         * constructor building an uninitialized Implementation.
         */
        DistFastRedundantShare(int argc, char** argv);
        DistFastRedundantShare(xercesc::DOMElement* data);
        DistFastRedundantShare(const DistFastRedundantShare& orig);
        virtual ~DistFastRedundantShare();

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
         * Get the Root-Type of XML-Elements representing this class.
         *
         * @return the Root-Type of XML-Elements representing this class.
         */
        static std::string getXMLRootType() {
            return std::string("FastRedundantShare");
        }

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
         * @see Distributor::setBaseMessage
         *
         * Overridden, so changing the baseMessage here also changes
         * the baseMessage in the underlying Share implementations.
         */
        virtual void setBaseMessage(uint8_t* baseMessage);
        
    private:
        /**
         * here you can find the Share-Implementations needed to distribute
         * each copy.
         */
        VDRIVE_FRS_MMAP *distributors;

        /**
         * The number of copies returned by each call of placeExtent.
         */
        int32_t copies;

        /**
         * The size of each extent passed to this Distributor
         */
        int64_t extentsize;

        /**
         * disks by their ID
         */
        VDRIVE_FRS_DISKMAP *disks;

        /**
         * initialises distributors by building Share-Implementations to place
         * each copy in O(1).
         */
        void initShareStructures(DistRedundantShare *rs);
        
        /**
         * create a single Share-Strategy for the given copy and the given
         * area of the given disks.
         *
         * @param disks Initialized RSDisks (from DistRedundantShare)
         * @param first first Disk to put in Share
         * @param last last disk to put in Share
         * @param k The copy to be placed.
         *
         * @return A Share implementation placing a copy k in O(1) over the
         *         disks
         */
        DistShare* createShare(RSDisk** disks, int32_t first, int32_t last, int32_t k);

    };
}
#endif	/* _FASTREDUNDANTSHARE_H */

