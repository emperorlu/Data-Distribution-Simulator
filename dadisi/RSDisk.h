/* 
 * File:   RSDisk.h
 * Author: fermat
 *
 * Created on 20. Januar 2010, 16:42
 */

#ifndef _RSDISK_H
#define	_RSDISK_H

#include<cstring>
#include<gmpxx.h>
#include"Disk.h"
#include <xercesc/dom/DOMDocument.hpp>

namespace VDRIVE {

    /**
     * RSDisk is a used by RedundantShare to hold some precomputed data about a
     * Disk. Most important is checkC used to place the Extents.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class RSDisk {
    public:

        /**
         * Generate a new uninitialized RSDisk representing the given Disk.
         *
         * @param disk The disk to be represented by this RSDisk.
         */
        RSDisk(Disk* disk);

        /**
         * copy constructor
         *
         * @param orig original RSDisk
         */
        RSDisk(const RSDisk& orig);

        /**
         * Destructor
         */
        virtual ~RSDisk();

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a RSDisk.
         */
        RSDisk(xercesc::DOMElement* data);

        /**
         * Get the ID of the underlying Disk
         *
         * @return ID of the underlying Disk
         */
        int64_t getId() {
            return disk->getId();
        }

        /**
         * Get the capacity of the underlying Disk
         *
         * @return capacity of the underlying Disk
         */
        int64_t getCapacity() {
            return disk->getCapacity();
        }

        /**
         * Get the underlying Disk
         *
         * @return underlying Disk
         */
        Disk* getDisk() const {
            return disk;
        }

        /**
         * Get the real useable capacity.
         *
         * @return real useable capacity.
         */
        int64_t getAlignedCapacity() const {
            return alignedCapacity;
        }

        /**
         * Set the real useable capacity.
         *
         * @param alignedCapacity real useable capacity.
         */
        void setAlignedCapacity(int64_t alignedCapacity) {
            this->alignedCapacity = alignedCapacity;
        }

        /**
         * Get the size of the "Rest" of Disks
         *
         * @return size of the "Rest" of Disks
         */
        int64_t getCPosition() const {
            return cPosition;
        }

        /**
         * Set the size of the "Rest" of Disks
         *
         * @param cPosition size of the "Rest" of Disks
         */
        void setCPosition(int64_t cPosition) {
            this->cPosition = cPosition;
        }

        /**
         * Get the aligned size of the disk devided by "Rest" of Disks
         *
         * @return aligned size of the disk devided by "Rest" of Disks
         */
        mpq_class getCheckC() const {
            return checkC;
        }

        /**
         * Set the aligned size of the disk devided by "Rest" of Disks
         *
         * @param checkC aligned size of the disk devided by "Rest" of Disks
         */
        void setCheckC(mpq_class checkC) {
            this->checkC = checkC;
            this->checkCDouble = checkC.get_d();
        }

        /**
         * Get the precomputed nearest double to checkC
         *
         * @return precomputed nearest double to checkC
         */
        double getCheckCDouble() const {
            return checkCDouble;
        }

        /**
         * Get the checkC for the kError copy.
         *
         * @return checkC for the kError copy.
         */
        mpq_class getCheckCStar() const {
            return checkCStar;
        }

        /**
         * Set the checkC for the kError copy.
         *
         * @param checkCStar checkC for the kError copy.
         */
        void setCheckCStar(mpq_class checkCStar) {
            this->checkCStar = checkCStar;
            this->checkCStarDouble = checkCStar.get_d();
        }

        /**
         * Get the precomputed nearest double to checkCStar
         *
         * @return precomputed nearest double to checkCStar
         */
        double getCheckCStarDouble() const {
            return checkCStarDouble;
        }

        /**
         * Get the k, this Disk has a error correcting checkCStar for (-1 for none)
         *
         * @return k, this Disk has a error correcting checkCStar for (-1 for none)
         */
        int32_t getKError() const {
            return kError;
        }

        /**
         * Set the k, this Disk has a error correcting checkCStar for (-1 for none)
         *
         * @param kError k, this Disk has a error correcting checkCStar for (-1 for none)
         */
        void setKError(int32_t kError) {
            this->kError = kError;
        }

        /**
         * Get the the id of the RSDisk. (Is it needed?)
         *
         * @return the id of the RSDisk. (Is it needed?)
         */
        std::string getRsId() const {
            return rsId;
        }

        /**
         * Set the the id of the RSDisk. (Is it needed?)
         *
         * @param rsId the id of the RSDisk. (Is it needed?)
         */
        void setRsId(std::string rsId) {
            this->rsId = rsId;
        }

        /**
         * build an XML-Version of this object
         *
         * @param doc the document needed to create new XML Elements
         * @return a new Element containing the description of this object.
         */
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc);

        /**
         * Get the Root-Type of XML-Elements representing this class.
         *
         * @return the Root-Type of XML-Elements representing this class.
         */
        static std::string getXMLRootType() {
            return std::string("RSDisk");
        }
    private:
        /**
         * The original Disk as passed to RedundantShare.
         */
        Disk* disk;

        /**
         * the id of the RSDisk. (Is it needed?)
         */
        std::string rsId;

        /**
         * aligned size of the disk devided by "Rest" of Disks
         */
        mpq_class checkC;

        /**
         * A Disk can have for one k an error, this k is represented here
         * (-1 if no error)
         */
        int32_t kError;

        /**
         * checkC for the kError copy.
         */
        mpq_class checkCStar;

        /**
         * precomputed nearest double to checkC
         */
        double checkCDouble;

        /**
         * precomputed nearest double to checkCStar
         */
        double checkCStarDouble;

        /**
         * size of the "Rest" of Disks
         */
        int64_t cPosition;

        /**
         * real useable capacity.
         */
        int64_t alignedCapacity;
    };
}



#endif	/* _RSDISK_H */

