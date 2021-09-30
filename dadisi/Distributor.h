/* 
 * File:   Distributor.h
 * Author: fermat
 *
 * Created on 20. Januar 2010, 10:34
 */

#ifndef _DISTRIBUTOR_H
#define	_DISTRIBUTOR_H

#include <vector>
#include <list>
#include <string>
#include "Disk.h"
#include <ostream>
#include <cstring>
#include <gcrypt.h>


namespace VDRIVE {

    /**
     * A Distributor can be used to Distribute Extents of a Virtual Volume over
     * a number of Disks. This abstract class builds the API of all implemented
     * Distribution Algorithms.
     *
     * @auther Sascha Effert <fermat@uni-paderborn.de>
     */
    class Distributor {
    private:
        /**
         * baseMessage used for HashFunctions
         */
        uint8_t *baseMessage;

        /**
         * number of Threads, that may be used for initialization.
         */
        uint16_t numThreads;

        /**
         * use a internal Hashing algorithm or an external (libgcrypt)
         */
        bool internelHash;

        /**
         * which internal Hash Algorithm shall be used
         */
        int internalHashAlgorithm;

        /**
         * if internalHash is false this indicates the gcrypt Algorithm to be used
         */
        int gcryptHashAlgorithm;

    public:
        /**
         * This value send to createDistributor creates an implementation of
         * Share
         */
        const static int32_t SHARE = 1;
        /**
         * This value send to createDistributor creates an implementation of
         * Nearest Neighbour (Consistant Hashing)
         */
        const static int32_t NEAREST_NEIGHBOUR = 2;
        /**
         * This value send to createDistributor creates an implementation of
         * Redundant Share
         */
        const static int32_t REDUNDANT_SHARE = 3;
        /**
         * This value send to createDistributor creates an implementation of
         * Redundant Share in the O(k) version
         */
        const static int32_t REDUNDANT_SHARE_K = 4;
        /**
         * This value send to createDistributor creates an implementation of
         * Redundant Share
         */
        const static int32_t ROUND_ROBIN = 5;
        /**
         * This value send to createDistributor creates an implementation of
         * RUSHp
         */
        const static int32_t RUSH_P = 6;
        /**
         * This value send to createDistributor creates an implementation of
         * CRUSH
         */
        const static int32_t CRUSH = 7;
        /**
         * This value send to createDistributor creates an implementation of
         * RUSH_R
         */
        const static int32_t RUSH_R = 8;
        /**
         * This value send to createDistributor creates an implementation of
         * RUSH_T
         */
        const static int32_t RUSH_T = 9;

        /**
         * This value send to createDistributor creates an implementation of
         * RAND_SLICE
         */
        const static int32_t RAND_SLICE = 10;

        /**
         * if we ose an own Hash function (internalHash is true, so no
         * libgcrypt is used) than this value means we use our SHA1
         * implementation.
         */
        const static int HASH_SHA1 = 1;

        /**
         * if we ose an own Hash function (internalHash is true, so no
         * libgcrypt is used) than this value means we use our linear congruent
         * implementation.
         */
        const static int HASH_XOR = 2;

        /**
         * if we ose an own Hash function (internalHash is true, so no
         * libgcrypt is used) than this value means we use our rand
         * implementation.
         */
        const static int HASH_RAND = 3;

        /**
         * if we ose an own Hash function (internalHash is true, so no
         * libgcrypt is used) than this value means we use our MT
         * implementation.
         */
        const static int HASH_MT = 4;

        /**
         * used a factor for creation of own Hash using xor. The Hash is build by
         * result = a * message + b
         * The value had to be set in Constructor to use a real big prime number here...
         */
        uint64_t a;

        /**
         * used a sum for creation of own Hash using xor. The Hash is build by
         * result = a * message + b
         */
        uint64_t b;

        /**
         * Destructor
         */
        virtual ~Distributor();

        /**
         * Compute the Disks to be used to place the Extent at position at the
         * given virtualVolumeID. This method has to be implemented by the
         * subclasses of Distributor.
         *
         * The returned list of Disk object as the Disk objects itself belong
         * the caller of this method. A Distributor creates copy of his Disk
         * objects which are send back in this list. Therefore the caller has
         * to delete the list and the Disk objects in the list if it is no more
         * used.
         *
         * @param virtualVolumeId A ID referencing the virtual volume the
         *                        Extent belongs to (therefore several virtual
         *                        Volumes can use the same disks)
         * @param position The position of the Extent on the virtual Volume
         * @return The Disks the Extent has to be placed on. All returned
         *         Objects have to be deleted by caller of this method.
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) = 0;

        /**
         * Set the configuration of the Distributor. This method has to be
         * called exacly once on each Object. It initializes the object, so it
         * can be used to place extents. This method has to be implemented by
         * the subclasses of Distributor.
         *
         * The list of Disks as the Disk objects itself belong the caller of
         * this method. A Distributor creates copy of the Disk objects.
         * Therefore the caller has to delete the list and the Disk objects in
         * the list if it is no more used.
         *
         * @param disks A list with the description of the Disks to distribute
         *              data over.
         * @param extentsize The size of each extent placed on the disk in
         *                   bytes. (This can also be in any other scale, but
         *                   has to be same for capacity of the Disks)
         * @param copies The number of Disks used to place eache Extent. If a
         *               RAID is used upon the Extents this would be the stripe
         *               size.
         * @return A list with the Disks, that shall be used
         */
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) = 0;
        
        virtual void setClusters(std::list<std::list<Disk*>*>* clusters, std::list<uint32_t>* weights)  {};
        /**
         * Change the Disks used by this Distributor.
         *
         * The list of Disks as the Disk objects itself belong the caller of
         * this method. A Distributor creates copy of the Disk objects.
         * Therefore the caller has to delete the list and the Disk objects in
         * the list if it is no more used.
         *
         * @param disks The Disks, that now should be used.
         */
        virtual void setDisks(std::list<Disk*>* disks) = 0;

        /**
         * get (a copy of) the BaseMessage used by the Hashfunctions.
         * 
         * Will return a 64-Byte-Array containing a copy of the base-Values 
         * used to xor Messages again to create Hashes from. The array belongs
         * to the caller of this method and can be modified and must be deleted
         * by him (using delete[]).
         * 
         * @return a copy of the BaseMessage used by the Hashfunctions.
         */
        virtual uint8_t* getBaseMessage() const;

        /**
         * Set the new BaseMessage as described at getBaseMessage. This method
         * will create a copy of this message, so the responsibility for
         * deleting stays at the caller of this method.
         *
         * @param baseMessage The new base Message
         *
         * @see getBaseMessage
         */
        virtual void setBaseMessage(uint8_t* baseMessage);

        /**
         * Creates a Hashvalue by using the bit shift operation of
         * Mersenne Twister. Therefore a 32-Bit version of the given message
         * is created which is used as seed. The algorithm computes the first
         * two randomized values which mersenne twister would give us and
         * combines them to a 64 bit randomized value.
         *
         * @param data message to be used as seed
         * @return a hash values
         */
        uint64_t myMTRand(std::string* data) const;

        /**
         * Creates a Hashvalue by using the rand-function of stdlib.
         * Therefore a 32-Bit version of the given message
         * is created which is used as seed. The algorithm computes the first
         * two randomized values by using rand() and
         * combines them to a 64 bit randomized value.
         *
         * @param data message to be used as seed
         * @return a hash values
         */
        uint64_t myRand(std::string* data) const;

        /**
         * Creates a Hashvalue by using the a linear congruention.
         * Therefore a 64-Bit version of the given message
         * is created which is used as base value. 
         * The hash is then computed by
         * 
         * \f[h(x) = (a\cdot x + b) mod 2^{64}\f]
         * 
         * a and b are initialized in the constructor as 
         * a = 10000000000000000051 and b = 13.
         * 
         * @param data message to be used as seed
         * @return a hash values
         */
        uint64_t myLC(std::string* data) const;

        /**
         * Creates a Hashvalue by using SHA1.
         * Therefore a 64-Byte version of the given message
         * is created which is encrypted by SHA1. The first 8 Byte of the
         * Digest are used as result.
         *
         * @param data message to be used as seed
         * @return a hash values
         */
        uint64_t mySHA1(std::string* data) const;

        /**
         * Creates a Hashvalue using the libgcrypt. The used hash function is
         * specified by gcryptHashAlgorithm.
         *
         * @param data message to be used as seed
         * @return a hash values
         */
        uint64_t gcrypt(std::string* data) const;

        /**
         * This Hashfunction delivers for a given Text a pseudo randomized
         * value between 0 and 1. We used SHA to calculate this value.
         *
         * @param data message to be used as seed
         * @return a hash values
         */
        double hashFunction(std::string *data) const;

        /**
         * This Hashfunction delivers for a given Text a pseudo randomized
         * int64_t value (can be negative).
         * 
         * @param data message to be used as seed
         * @return a hash value
         */
        uint64_t hashFunctionInt64(std::string *data) const;

        /**
         * Get the Disks used by this object as passed to setDisks or setConfiguration.
         *
         * The returned list of Disk object as the Disk objects itself belong
         * the caller of this method. A Distributor creates copy of his Disk
         * objects which are send back in this list. Therefore the caller has
         * to delete the list and the Disk objects in the list if it is no more
         * used.
         *
         * @return The disks used by this object
         */
        virtual std::list<Disk*>* getDisks() const = 0;

        /**
         * Get the Size of the Extents passed to this object as passed to setConfiguration.
         *
         * @return the Size of the Extents passed to this object.
         */
        virtual int64_t getExtentsize() const = 0;

        /**
         * Get the number of copies of each extents to be placed as passed to setConfiguration.
         * 
         * @return the number of copies of each extents to be placed
         */
        virtual int32_t getCopies() const = 0;

        /**
         * Create a new Distributor of the given kind. Using this factory
         * method a using library has not to know about the implementation
         * classes.
         *
         * @param distributor The kind of distributor
         * @param argc number of arguments passed to the distributor
         * @param argv arguments passed to the distributor
         * @return A new Distributor object
         */
        static Distributor* createDistributor(int32_t distributor, int argc, char** argv);

        /**
         * load a configured Distributor from a file.
         *
         * @param filename The name of the file containing the distributor
         * @return the Distributor
         */
        static Distributor* loadDistributor(std::string filename);

        /**
         * build an XML-Version of this object
         *
         * @param doc the document needed to create new XML Elements
         * @return a new Element containing the description of this object.
         */
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const = 0;

        /**
         * save this (configured) Distributor to a file, so it can be loaded later.
         *
         * @param filename The name of the file the distributor shall be saved in.
         */
        void save(std::string filename) const;

        /**
         * get the number of Threads that may be used for Initialization
         * (setDisks)
         *
         * @return the number of Threads that may be used for Initialization
         */
        uint16_t getNumThreads() const {
            return numThreads;
        }

        /**
         * set the number of Threads that may be used for Initialization
         * (setDisks)
         *
         * @param numThreads the number of Threads that may be used for Initialization
         */
        void setNumThreads(uint16_t numThreads) {
            if (numThreads > 0)
                this->numThreads = numThreads;
        }

        /**
         * use the internal Hash Algorithm to create Hashes (SHA1).
         */
        void useInternalHashAlgorithm(int algorithm) {
            internelHash = true;
            internalHashAlgorithm = algorithm;
        }

        /**
         * Use a Hash Algorithm of the lib gcrypt to build hashes.
         *
         * @param algorithm the algorithm to be used. The int represents a hash
         *                  algorithm which has to exists in the used libgcrypt.
         *                  For a list of known hash functions have a look at the
         *                  documentation of your version of libgcrypt.
         */
        bool useGCryptHashAlgorithm(int algorithm) {
            if (gcry_md_test_algo(algorithm) != 0) {
                return false;
            }
            internelHash = false;
            gcryptHashAlgorithm = algorithm;
            return true;
        }

        /**
         * get information about used Hashalgorithm.
         *
         * @param internal true is the internal Hash Algorithm is used, false if
         *                 a libgcrypt Hash algorithm is used.
         * @param algorithm only set if internal is false. Then it holds the
         *                  algorithm used by libgrypt to create Hashes.
         */
        void usedHashAlgorithm(bool* internal, int* gcryptalgorithm, int* intalgorithm) const {
            *internal = internelHash;
            *gcryptalgorithm = gcryptHashAlgorithm;
            *intalgorithm = internalHashAlgorithm;
        }

    protected:
        /**
         * Creates a new Distributor.
         */
        Distributor();
    };
}
#endif	/* _DISTRIBUTOR_H */

