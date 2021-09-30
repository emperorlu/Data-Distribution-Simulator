/* 
 * File:   DistRandSlice.h
 * Author: amiranda
 *
 * Created on 8, November 2010, 12:24
 */

#ifndef _DISTRANDSLICE_H
#define _DISTRANDSLICE_H


#include <tr1/unordered_map> //FIXME?
#define __STDC_LIMIT_MACROS// required for UINT64_MAX macro
#include <stdint.h>
#include "Distributor.h"
#include "flat_segment_tree.hpp"


#define DEBUG
#ifdef DEBUG
#   define DUMP_COPIES
//#   define DUMP_DISKS
//#   define DUMP_PARTITIONS
//#   define DUMP_FREE_SPACE_COLLECTION
//#   define DUMP_FREE_SPACE_ASSIMILATION
//#   define DUMP_FREE_SPACE
//#   define DUMP_INTERVALS
//#   define DUMP_INTERVALS_VERBOSE
#endif


namespace VDRIVE {

    /**
     * XXX: Place comment describing the distribution algorithm here
     *
     *
     *
     *
     */
    class DistRandSlice : public Distributor {
    public:
        
        // constants declaring the min and max values for the 
        // virtual allocation space.
        // right now, it spans from 0 to 2^64-1 blocks/files/whatever
        static const uint64_t VSPACE_MIN = 0;
        static const uint64_t VSPACE_MAX = UINT64_MAX;
    

        /**
         * Create a new instance from the data in the given XML Element.
         *
         * @param data An XML-Element containing the description of a DistRandSlice.
         */
        DistRandSlice(xercesc::DOMElement* data);

        /**
         * generate a new, uninitialized Rand Slice Implementation.
         */
        DistRandSlice(int argc, char** argv);

        /**
         * copy constructor
         *
         * @param orig original DistRandSlice
         */
        DistRandSlice(const DistRandSlice& orig);

        /**
         * Destructor
         */
        virtual ~DistRandSlice();

        /**
         * @see Distributor::placeExtent
         */
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position);

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
            return m_extentsize;
        }

        /**
         * @see Distributor::getCopies
         */
        virtual int32_t getCopies() const {
            return m_copies;
        }

		/**
		 * @return the number of partitions currently in the system
		 */
		uint64_t getNumPartitions() const {
			return m_num_partitions;
		}

		/**
		 * @return the number of intervals currently in the system
		 */
		uint64_t getNumIntervals() const;

        /**
         * Get the Root-Type of XML-Elements representing this class.
         *
         * @return the Root-Type of XML-Elements representing this class.
         */
        static std::string getXMLRootType() {
            return std::string("RandSlice");
        }

    private:

        void cleanup(void);
        
		void create_partitions(std::list<Disk*>* disks);
        
		void add_partitions(std::list<Disk*>* disks);
        
		void redistribute(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions,
                          const std::list< std::pair<uint64_t, uint64_t> >& new_partitions);

		void collect_free_space(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions,
								std::list< std::pair<uint64_t, uint64_t> >& free_space);

		void collect_free_space_even_odd(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions,
								         std::list< std::pair<uint64_t, uint64_t> >& free_space);

		void reuse_free_space(std::list< std::pair<uint64_t, uint64_t> >& free_space,
							  const std::list< std::pair<uint64_t, uint64_t> >& new_partitions);

		void reuse_free_space_sort(std::list< std::pair<uint64_t, uint64_t> >& free_space,
							       const std::list< std::pair<uint64_t, uint64_t> >& new_partitions);


#if defined DEBUG
        void dump_intervals(void);
        void dump_free_space(const std::list< std::pair<uint64_t, uint64_t> >& l) const;
        void verify_partitions(void);
        std::vector<uint64_t> compute_interval_sizes(void);
#endif

        /**
         * ExtenSize as given to setConfiguration.
         */
        int64_t m_extentsize;

        /**
         * number of copies to be distributed.
         */
        int32_t m_copies;

        /**
         * the disks as given to setDisks (or setConfiguration)
         */
        std::tr1::unordered_map<uint64_t, Disk*>* m_disks;

        /**
         * number of Disks contained by this Distributor.
         */
        uint64_t m_num_disks;

        /**
         * partition info: id -> capacity
         */
        std::tr1::unordered_map<uint64_t, uint64_t>* m_partitions;

        /**
         * number of partitions
         */
        uint64_t m_num_partitions;

        /**
         * interval tree for searches
         */
        typedef ::mdds::flat_segment_tree<uint64_t, uint64_t> flat_segment_tree;
        flat_segment_tree* m_interval_tree;

        /**
         * capacity of the system
         */
        uint64_t m_capacity;


        /**
         * use Even-Odd strategy when collecting free space
         */
        bool m_use_even_odd_collection;

        /**
         * sort free intervals decreasingly when assimilating free space
         */
        bool m_use_sorted_assimilation;
        

    };
}
#endif /* _DISTRANDSLICE_H */

