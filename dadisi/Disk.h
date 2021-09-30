/* 
 * File:   Disk.h
 * Author: fermat
 *
 * Created on 20. Januar 2010, 10:39
 */

#ifndef _DISK_H
#define	_DISK_H

#define __STDC_LIMIT_MACROS // required for limit macros
#include<stdint.h>
#include<xercesc/dom/DOMElement.hpp>
#include<list>
#include<string>

#include "helper.h"

namespace VDRIVE {

    /**
     * This class represents a Disk for the Distributor. Each Disk consists of
     * an unique ID (in the list of Disks used by this lib), a capacity and a
     * reference to some data, that can be used by the person instantiating a
     * Disk.
     *
     * @author Sascha Effert <fermat@uni-paderborn.de>
     */
    class Disk {
    public:

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a disk.
         */
        Disk(xercesc::DOMElement* data);

        /**
         * instantiates a new Disk with the given values.
         *
         * @param id ID of the disk. Has to be unique all over this library.
         * @param capacity Capacity of the Disk in bytes. (This can also be in
         *                 any other scale, but has to be same for ExtentSize
         *                 of Distributor)
         * @param data Pointer to be used by developer instantiating this disk.
         *             May not be changed by Distributors!
         */
        Disk(int64_t id, int64_t capacity, void* data);

        /**
         * copy constructor
         *
         * @param orig original Disk
         */
        Disk(const Disk& orig);

        /**
         * Destructor
         */
        virtual ~Disk();

        /**
         * Get the ID of the disk. Has to be unique all over this library.
         *
         * @return ID of the disk. Has to be unique all over this library.
         */
        int64_t getId() const;

        /**
         * Set the ID of the disk. Has to be unique all over this library.
         *
         * @param id ID of the disk. Has to be unique all over this library.
         */
        void setId(int64_t id);

        /**
         * Get the Capacity of the Disk in bytes. (This can also be in any
         * other scale, but has to be same for ExtentSize of Distributor)
         *
         * @return Capacity of the Disk in bytes. (This can also be in any
         *         other scale, but has to be same for ExtentSize of
         *         Distributor)
         */
        int64_t getCapacity() const;

        /**
         * Set the Capacity of the Disk in bytes. (This can also be in any
         * other scale, but has to be same for ExtentSize of Distributor)
         *
         * @param capacity Capacity of the Disk in bytes. (This can also be in
         *                any other scale, but has to be same for ExtentSize of
         *                Distributor)
         */
        void setCapacity(int64_t capacity);

        /**
         * Get the Pointer to be used by developer instantiating this disk.
         * May not be changed by Distributors!
         *
         * @return Pointer to be used by developer instantiating this disk.
         *         May not be changed by Distributors!
         */
        void* getData() const;

        /**
         * Set the Pointer to be used by developer instantiating this disk. May not be changed by Distributors!
         *
         * @param data Pointer to be used by developer instantiating this disk.
         *             May not be changed by Distributors!
         */
        void setData(void* data);

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
            return std::string("Disk");
        }

        /**
         * Using this method it is possible to store a list of disks in a file.
         *
         * @param disks The disks to be stored
         * @param filename The name of the file the disks shall be stored in.
         */
        static void storeDiskList(std::list<Disk*>* disks, std::string filename);

        /**
         * Using this method it is possible to read a list of disks out of a
         * file.
         *
         * @param filename The name of the file containing the disks.
         *
         * @return A list with the disks readen out of the file.
         */
        static std::list<Disk*>* loadDiskList(std::string filename);

#ifndef no_sqlite
        /**
         * Using this method it is possible to store a list of disks in a sqlite db file.
         *
         * @param disks The disks to be stored
         * @param filename The name of the file the disks shall be stored in.
         */
        static void storeDiskListDBFile(std::list<Disk*>* disks, std::string filename);

        /**
         * Using this method it is possible to read a list of disks out of a sqlite db
         * file.
         *
         * @param filename The name of the file containing the disks.
         *
         * @return A list with the disks readen out of the file.
         */
        static std::list<Disk*>* loadDiskListDBFile(std::string filename);
#endif
    private:

        /**
         * ID of the disk. Has to be unique all over this library.
         */
        int64_t id;

        /**
         * Capacity of the Disk in bytes. (This can also be in any other
         * scale, but has to be same for ExtentSize of Distributor)
         */
        int64_t capacity;

        /**
         * Pointer to be used by developer instantiating this disk. May not be
         * changed by Distributors!
         */
        void* data;
    };
}
#endif	/* _DISK_H */

