/*
 * File:    DistRandSlice.cpp
 * Author: amiranda
 *
 * Created on 8, November 2010, 12:41
 */

#include <list>
#include <string>
#include <stdexcept>
#include <cmath>
#include <iomanip>

#include "DistRandSlice.h"

using namespace VDRIVE;
using namespace std;
using namespace xercesc;


DistRandSlice::DistRandSlice(int argc, char** argv) : 
    Distributor() {

    m_extentsize = 0;
    m_copies = 0;
    m_disks = NULL;
    m_num_disks = 0;
    m_partitions = NULL;
    m_num_partitions = 0;
    m_interval_tree = NULL;
    m_capacity = 0;
    m_use_even_odd_collection = false;
    m_use_sorted_assimilation = false;

    for(int i=0; i < argc; ++i){
        if(argv[i][0] != '-'){
            throw std::logic_error("unkown argument");
        }
        else if(!strcmp(argv[i], "-even-odd")){
            m_use_even_odd_collection = true;
        }
        else if(!strcmp(argv[i], "-sort-free")){
            m_use_sorted_assimilation = true;
        }
        else{
            throw std::logic_error("unkown argument");
        }
    }
}

DistRandSlice::DistRandSlice(xercesc::DOMElement* data) {
    // FIXME: implement this feature
    throw std::logic_error("feature not implemented");
}

DistRandSlice::DistRandSlice(const DistRandSlice& orig) {

    m_extentsize = orig.m_extentsize;
    m_copies = orig.m_copies;

	if(orig.m_disks != NULL){
		m_disks = new std::tr1::unordered_map<uint64_t, Disk*>();

		for(std::tr1::unordered_map<uint64_t, Disk*>::const_iterator it = orig.m_disks->begin();
			it != orig.m_disks->end();
			++it){

			(*m_disks)[it->first] = new Disk(*(it->second));
		}

		m_num_disks = orig.m_num_disks;
	}
	else{
		m_disks = NULL;
		m_num_disks = 0;
	}


	if(orig.m_partitions != NULL){
		m_partitions = new std::tr1::unordered_map<uint64_t, uint64_t>(*orig.m_partitions);
		m_num_partitions = orig.m_num_partitions;
	}
	else{
		m_partitions = NULL;
		m_num_partitions = 0;
	}


	if(orig.m_interval_tree != NULL){
		// XXX: the copy constructor for flat_segment_tree does not copy the internal
		// tree structure, only the leaf nodes. therefore we need to rebuild the
		// tree using the provided build_tree function :P
		m_interval_tree = new flat_segment_tree(*orig.m_interval_tree);
		m_interval_tree->build_tree();
	}
	else{
		m_interval_tree = NULL;
	}

    m_capacity = orig.m_capacity;
    m_use_even_odd_collection = orig.m_use_even_odd_collection;
    m_use_sorted_assimilation = orig.m_use_sorted_assimilation;
}

DistRandSlice::~DistRandSlice() {
    cleanup();
}

void 
DistRandSlice::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {

    if(extentsize != 1)
        throw std::logic_error("unsupported extentsize value; supported values: 1");

    /*if(copies != 1)
        throw std::logic_error("unsupported copies value; supported values: 1");*/

    m_extentsize = extentsize;
    m_copies = copies;

    setDisks(disks);

}

void 
DistRandSlice::setDisks(std::list<Disk*>* disks) {

    if(this->m_disks == NULL){

        // create partitions for initial set of disks
        create_partitions(disks);

#if defined DEBUG        
        verify_partitions();
#endif
    }
    else{

        // add partitions for additional disks
        add_partitions(disks);

#if defined DEBUG        
        verify_partitions();
#endif
    }
}


/**
  * 64 bit Integer Hash Function
  * found at http://www.concentric.net/~Ttwang/tech/inthash.htm by Thomas Wang
  */
static inline uint64_t
hash(int64_t i){

	i  = (~i) + (i << 21); // i = (key << 21) - key - 1;
	i ^= (i >> 24);
	i  = (i + (i << 3)) + (i << 8);	// key * 265
	i ^= (i >> 14);
	i  = (i + (i << 2)) + (i << 4); // key * 21
	i ^= (i >> 28);
	i += (i << 31);

	return i;
}

std::list<Disk*>* 
DistRandSlice::placeExtent(int64_t virtualVolumeId, int64_t position){

    // XXX: for tests we want to avoid id clashing with copies so
	int64_t offset = 200;
    if(position > INT64_MAX/offset-1)
        abort();
    position*=offset;

	std::list<Disk*>* result = new std::list<Disk*>();
    std::vector<uint64_t> selected_partitions;

    uint64_t idx = 0;

#if defined DEBUG && defined DUMP_COPIES
    uint64_t retries = 0;
#endif

    while(result->size() < (uint32_t)m_copies){
        // XXX: at the moment, we ignore the virtualVolumeId
        uint64_t key = hash(position + idx);

        // lookup interval containing the key and retrieve the partition ID
        uint64_t partition_id = 0;
        m_interval_tree->search_tree(key, partition_id);

        // check if this partition has already been selected
        bool found = false;
        for(uint i=0; i<selected_partitions.size() && !found; ++i){
            if(partition_id == selected_partitions[i])
                found = true;
        }

        if(found){
#if defined DEBUG && defined DUMP_COPIES
            ++retries;
#endif
            ++idx;
            assert(idx <= (uint64_t)offset); //XXX
            continue;
        }

        // Find the corresponding disk and return it
        // XXX: ATM, partition_id == disk_id
        uint64_t disk_id = partition_id;
        assert(m_disks->count(disk_id) != 0);
        Disk* d = (*m_disks)[disk_id];

        result->push_back(new Disk(*d));
        selected_partitions.push_back(partition_id);
        ++idx;
    }

#if defined DEBUG && defined DUMP_COPIES
//    std::cout << "  DEBUG: Extent placed with " << retries << " retries " << std::endl;
#endif

    return result;
}

std::list<Disk*>* 
DistRandSlice::getDisks() const {

    std::list<Disk*>* l = new std::list<Disk*>();

    for(std::tr1::unordered_map<uint64_t, Disk*>::const_iterator it = m_disks->begin();
        it != m_disks->end();
        ++it){

        l->push_back(new Disk(*(it->second)));
    }

    return l;
}

xercesc::DOMElement* 
DistRandSlice::toXML(xercesc::DOMDocument* doc) const {
    // FIXME: implement this feature
    throw std::logic_error("feature not implemented");
    return NULL;
}

uint64_t
DistRandSlice::getNumIntervals(void) const{
	uint64_t n = 0;

	// FIXME: this loop wouldn't be needed if the creator of flat_segment_tree
	// had provided an easy way to get interval count T_T
	for(flat_segment_tree::const_iterator it = m_interval_tree->begin();
		it != m_interval_tree->end();
		++it){

		++n;
	}

	return n;
}


/* private functions */
void
DistRandSlice::cleanup(void){

    delete m_interval_tree;

    delete m_partitions;

	if(m_disks != NULL){
		for(std::tr1::unordered_map<uint64_t, Disk*>::iterator it = m_disks->begin();
			it != m_disks->end();
			++it){

			delete it->second;
		}
		delete m_disks;
	}

    // we don't know if this instance will ever be used again.
    // better be safe than sorry
    m_interval_tree = (__typeof__(m_interval_tree))0xdeadbeef;
    m_disks = (__typeof__(m_disks)) 0xdeadbeef;
    m_copies = m_extentsize = m_num_disks = m_num_partitions = m_capacity = 0;
}

void
DistRandSlice::create_partitions(std::list<Disk*>* disks){

    m_disks = new std::tr1::unordered_map<uint64_t, Disk*>();

    for(std::list<Disk*>::const_iterator it = disks->begin();
        it != disks->end();
        ++it){

        Disk* d = *it;

        (*(m_disks))[d->getId()] = new Disk(*d);
        m_capacity += (uint64_t)d->getCapacity();
    }

    m_num_disks = disks->size();

    assert(m_capacity <= VSPACE_MAX);

#if defined DEBUG && defined DUMP_DISKS
    std::cout << "  DEBUG: Total capacity: " << m_capacity << std::endl;

    for(std::list<Disk*>::const_iterator it = disks->begin();
        it != disks->end();
        ++it){

        Disk* d = *it;
        double p = d->getCapacity()*1.0/m_capacity;
        uint64_t nb = (uint64_t)floor(p*VSPACE_MAX);

        std::cout << "  DEBUG:   disk_" << d->getId() << "\t\t" 
                                        << d->getCapacity() << "\t\t" 
                                        << setw(10) << setprecision(7) << p*100.0 
                                        << "%\t\t(" << nb << " blocks)\n";
    }

#endif

    // create the partition table
    assert(m_partitions == NULL);
    m_partitions = new std::tr1::unordered_map<uint64_t, uint64_t>();

    // initialize the interval tree
    assert(m_interval_tree == NULL);
    m_interval_tree = new flat_segment_tree(VSPACE_MIN, VSPACE_MAX, disks->front()->getId() ); //XXX ATM partition_id == disk_id

    // compute the proportional capacities for each disk and use them to
    // partition the virtual block space

    uint64_t last_interval_end = VSPACE_MIN;

    for(std::list<Disk*>::const_iterator it = disks->begin();
        it != disks->end();
        ++it){

        double p = (*it)->getCapacity()*1.0/m_capacity;

        // compute interval limits
        uint64_t i0 = last_interval_end;
        uint64_t i1 = (uint64_t)floor(i0 + p*VSPACE_MAX);

        // to account for rounding errors, the last interval MUST end at
        // VSPACE_MAX
        if(*it == disks->back() || i1 > VSPACE_MAX){
            i1 = VSPACE_MAX;
        }

        last_interval_end = i1;
        
        //XXX: ATM partition_id == disk_id
        uint64_t part_id = (*it)->getId();

        (*m_partitions)[part_id] = i1-i0;
        m_interval_tree->insert_front(i0, i1, part_id ); 
    }

	m_num_partitions = m_partitions->size();

    // build tree to speed up searches
    // XXX: as required by the particular implementation of 
    // mdds::flat_segment_tree. bleh.
    m_interval_tree->build_tree();

#if defined DEBUG && defined DUMP_INTERVALS
    dump_intervals();
#endif
    
}

void
DistRandSlice::add_partitions(std::list<Disk*>* new_disks){

    uint64_t capacity_added, new_capacity;
    capacity_added = new_capacity = 0;

    // compute the capacity added by new_disks
    for(std::list<Disk*>::const_iterator it = new_disks->begin();
        it != new_disks->end();
        ++it){

        capacity_added += (*it)->getCapacity();
    }

    // compute the new proportions for the old partitions and how much must be
    // stolen from each
    new_capacity = m_capacity + capacity_added;

#if defined DEBUG && defined DUMP_DISKS
std::cout << "  DEBUG: Total capacity (new): " << new_capacity << std::endl;

    for(std::list<Disk*>::const_iterator it = new_disks->begin();
        it != new_disks->end();
        ++it){

        Disk* d = *it;
        double p = d->getCapacity()*1.0/new_capacity;
        uint64_t nb = (uint64_t)floor(p*VSPACE_MAX);

        std::cout << "  DEBUG:   disk_" << d->getId() << "\t\t" 
                                        << d->getCapacity() << "\t\t" 
                                        << setw(10) << setprecision(7) << p*100.0 
                                        << "%\t\t(" << nb << " blocks)\n";
    }

    std::cout << "\n";

#endif

    // partition_id -> number of blocks to redistribute
	std::tr1::unordered_map<uint64_t, uint64_t> old_partitions;
    uint64_t num_blocks_to_remove = 0;

    for(std::tr1::unordered_map<uint64_t, uint64_t>::const_iterator it = m_partitions->begin();
        it != m_partitions->end();
        ++it){

        uint64_t part_id = it->first;
        uint64_t old_part_capacity = it->second;

        // get disk capacity and compute the new partition capacity according to
        // it and the new capacity of the system
        uint64_t disk_capacity = m_disks->find(part_id)->second->getCapacity();
         
        double p = disk_capacity*1.0/new_capacity;

        uint64_t new_part_capacity = (uint64_t)floor(p*VSPACE_MAX);

		old_partitions[it->first] = old_part_capacity - new_part_capacity;

        num_blocks_to_remove += old_part_capacity - new_part_capacity;

#if defined DEBUG && defined DUMP_PARTITIONS
std::cout << "  DEBUG: partition_" << part_id << "\t\told capacity: " << setw(20) << old_part_capacity
                                   << "\t\tloses : "  << setw(20) << old_part_capacity - new_part_capacity << "\n";
#endif

        // !!! CAUTION: m_partitions map is not used again in computations, therefore it is
        //              safe to update partition sizes here. If this ever
        //              changes, take it into account
        (*m_partitions)[part_id] = new_part_capacity;

    }


#if defined DEBUG && defined DUMP_PARTITIONS
std::cout << "  DEBUG:\n";
std::cout << "  DEBUG: total blocks lost: " << num_blocks_to_remove << "\n";
std::cout << "  DEBUG:\n";
#endif

    // compute the proportions corresponding to the new partitions
    std::list< std::pair<uint64_t, uint64_t> > new_partitions;
    uint64_t num_blocks_to_add = 0;

    for(std::list<Disk*>::const_iterator it = new_disks->begin();
        it != new_disks->end();
        ++it){

        uint64_t part_id = (*it)->getId();
        double p = (*it)->getCapacity()*1.0/new_capacity;
        uint64_t new_part_capacity = (uint64_t)floor(p*VSPACE_MAX);
        num_blocks_to_add += new_part_capacity;

        //XXX ATM partition_id == disk_id
        new_partitions.push_back(std::make_pair(part_id, new_part_capacity));
        
#if defined DEBUG && defined DUMP_PARTITIONS
std::cout << "  DEBUG: partition_" << part_id << "\t\tnew capacity: " << setw(20) << new_part_capacity << "\n";
#endif

    }

#if defined DEBUG && defined DUMP_PARTITIONS
std::cout << "  DEBUG:\n";
std::cout << "  DEBUG: total blocks gained : " << num_blocks_to_add << "\n";
std::cout << "  DEBUG:\n";
#endif

    // XXX: ideally, num_blocks_to_add should equal num_blocks_to_remove, but
    // due to rounding errors, this is not always the case. Add/remove the
    // difference to/from the last partition.
    //
    // NOTE: We could distribute the remainder proportionally among the new
    // partitions, but this way is faster and the remainder is neglectable
    if(num_blocks_to_remove != num_blocks_to_add){
        uint64_t r = num_blocks_to_remove - num_blocks_to_add;
        new_partitions.back().second += r;
    }

    redistribute(old_partitions, new_partitions);

    // update members
    // TODO: we could probably add the new disks and the new partitions in an
    // earlier stage and mark them as dirty or something
    for(std::list<Disk*>::const_iterator it = new_disks->begin();
        it != new_disks->end();
        ++it){

        Disk* d = *it;

        (*(m_disks))[d->getId()] = new Disk(*d);
    }

    m_num_disks += new_disks->size();

    for(std::list< std::pair<uint64_t, uint64_t> >::const_iterator it = new_partitions.begin();
        it != new_partitions.end();
        ++it){

        uint64_t part_id = it->first;
        uint64_t part_size = it->second;

        (*m_partitions)[part_id] = part_size;
    }

    m_num_partitions += new_partitions.size();
    m_capacity = new_capacity;

#if defined DEBUG && defined DUMP_INTERVALS
    dump_intervals();
#endif

}

void
DistRandSlice::redistribute(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions, ///XXX rename to old_partitions
                            const std::list< std::pair<uint64_t, uint64_t> >& new_partitions){

    // for each existing partition, create a list of its intervals and try to shrink
    // the overall partition size while trying to reduce the number of
    // non-contiguous intervals as much as possible

    // compute the free space
    std::list< std::pair<uint64_t, uint64_t> > free_space;

    if(m_use_even_odd_collection){
        collect_free_space_even_odd(old_partitions, free_space);
    }
    else{
        collect_free_space(old_partitions, free_space);
    }

#if defined DEBUG && defined DUMP_FREE_SPACE
    dump_free_space(free_space);
#endif

    if(m_use_sorted_assimilation){
        reuse_free_space_sort(free_space, new_partitions);
    }
    else{
        reuse_free_space(free_space, new_partitions);
    }

}

void
DistRandSlice::collect_free_space(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions,
							      std::list< std::pair<uint64_t, uint64_t> >& free_space){

    uint64_t last_low = m_interval_tree->begin()->first;
    uint64_t last_pid = m_interval_tree->begin()->second;

    bool done = false;
    flat_segment_tree::const_iterator it = m_interval_tree->begin();

    while(!done){

        // XXX: since the iterator for flat_segment_tree does not return the
        // last limit of the interval, we must do an extra round ...
        if(it == m_interval_tree->end())
            done = true;

        if(it->first != last_low){
            uint64_t i_low, i_high, i_pid;

            i_low = last_low;
            i_high = it->first;
            i_pid = last_pid;

            uint64_t i_sz = i_high - i_low; // interval size
            assert(i_low != i_high);

            uint64_t i_psz = old_partitions.find(i_pid)->second; // number of blocks to remove from partition

            if(i_psz != 0){
                if(i_sz < i_psz){
                    // add interval completely but check if it is a new interval or
                    // an extension of the last one we added
                    if(!free_space.empty() && free_space.back().second == i_low){
                        free_space.back().second = i_high;
                    }
                    else{
                        free_space.push_back(std::make_pair(i_low, i_high));
                    }
                    old_partitions[i_pid] -= i_sz;
                }
                else{
                    // add interval partially but check if it is a new interval or
                    // an extension of the last one we added
                    if(!free_space.empty() && free_space.back().second == i_low){
                        free_space.back().second = i_low + i_psz;
                    }
                    else{
                        free_space.push_back(std::make_pair(i_low, i_low + i_psz));
                    }
                    old_partitions[i_pid] -= i_psz;
                }
            }

            last_low = it->first;
            last_pid = it->second;
        }

        if(!done)
            ++it;
    }
}


void
DistRandSlice::collect_free_space_even_odd(std::tr1::unordered_map<uint64_t, uint64_t>& old_partitions,
							               std::list< std::pair<uint64_t, uint64_t> >& free_space){

    bool last_was_complete = false;
    bool even = false;
    uint64_t last_low = m_interval_tree->begin()->first;
    uint64_t last_pid = m_interval_tree->begin()->second;

    bool done = false;
    flat_segment_tree::const_iterator it = m_interval_tree->begin();

    while(!done){

        // XXX: since the iterator for flat_segment_tree does not return the
        // last limit of the interval, we must do an extra round ...
        if(it == m_interval_tree->end())
            done = true;

        if(it->first != last_low){
            uint64_t i_low, i_high, i_pid;

            i_low = last_low;
            i_high = it->first;
            i_pid = last_pid;

            uint64_t i_sz = i_high - i_low; // interval size
            assert(i_low != i_high);

            uint64_t i_psz = old_partitions.find(i_pid)->second; // number of blocks to remove from partition

            if(i_psz != 0){
                if(i_sz < i_psz){

#if defined DUMP_FREE_SPACE_COLLECTION
                    std::cout << "   i_sz: " << i_sz <<" vs. i_psz: " << i_psz << " => COMPLETE\n";
#endif
                    // add interval completely but check if it is a new interval or
                    // an extension of the last one we added
                    if(!free_space.empty() && free_space.back().second == i_low){
//std::cout << "extend" << (!even ? " even\n" : " odd\n");
                        free_space.back().second = i_high;
                    }
                    else{
                        free_space.push_back(std::make_pair(i_low, i_high));
                    }

                    old_partitions[i_pid] -= i_sz;

                    if(last_was_complete)
                        even = false;

                    last_was_complete = true;
                }
                else{

#if defined DUMP_FREE_SPACE_COLLECTION
                    std::cout << "   i_sz: " << i_sz <<" vs. i_psz: " << i_psz << " => PARTIAL\n";
#endif
                    // add interval partially but check if it is a new interval or
                    // an extension of the last one we added
                    if(!free_space.empty() && free_space.back().second == i_low){
//std::cout << "extend" << (!even ? " even\n" : " odd\n");
                        free_space.back().second = i_low + i_psz;
                    }
                    else{
                        if(even){
//std::cout << "push even\n";
                            free_space.push_back(std::make_pair(i_high-i_psz, i_high));
                        }
                        else{
//std::cout << "push odd\n";
                            free_space.push_back(std::make_pair(i_low, i_low + i_psz));
                        }

                    }

                    old_partitions[i_pid] -= i_psz;

                    last_was_complete = false;
                    even = !even;
                }
            }

            last_low = it->first;
            last_pid = it->second;
        }

        if(!done)
            ++it;
    }
}


void
DistRandSlice::reuse_free_space(std::list< std::pair<uint64_t, uint64_t> >& free_space,
							    const std::list< std::pair<uint64_t, uint64_t> >& new_partitions){

    // assign free intervals to new partitions splitting as needed
    for(std::list< std::pair<uint64_t, uint64_t> >::const_iterator it = new_partitions.begin();
        it != new_partitions.end();
        ++it){

        uint64_t pid   = it->first;
        uint64_t psize = it->second;

//std::cout << "*** partition_" << pid << " size (new): " << psize << "\n";

        for(std::list< std::pair<uint64_t, uint64_t> >::iterator it2 = free_space.begin();
            it2 != free_space.end() && psize > 0;
            ++it2){

            uint64_t isize = it2->second - it2->first;

            if(isize == 0)
                continue;

            if(isize > psize){
                // assimilate partially
                
#if defined DUMP_FREE_SPACE_ASSIMILATION
                std::cout << "   isize: " << isize <<" vs. psize: " << psize << " => PARTIAL\n";
#endif
                m_interval_tree->insert_front(it2->first, it2->first+psize, pid);
                it2->first += psize;

                psize = 0;
            }
            else{
                // assimilate completely

#if defined DUMP_FREE_SPACE_ASSIMILATION
                std::cout << "   isize: " << isize <<" vs. psize: " << psize << " => COMPLETE\n";
#endif
                m_interval_tree->insert_front(it2->first, it2->second, pid);
                it2->first = 0;
                it2->second = 0;

                psize -= isize;
            }


//std::cout << "*** partition_" << pid << " psize: " << psize << "\n";
        }

#if defined DEBUG && defined DUMP_FREE_SPACE
        dump_free_space(free_space);
#endif

        
    }

    m_interval_tree->build_tree();
	
}

static bool 
cmp_free_intervals(std::pair<uint64_t, uint64_t> i1, std::pair<uint64_t, uint64_t> i2){

    uint64_t sz1 = i1.second - i1.first;
    uint64_t sz2 = i2.second - i2.first;

    return sz1 > sz2;
}

static bool 
cmp_partitions(std::pair<uint64_t, uint64_t> p1, std::pair<uint64_t, uint64_t> p2){

    return p1.second > p2.second;
}

void
DistRandSlice::reuse_free_space_sort(std::list< std::pair<uint64_t, uint64_t> >& free_space,
							         const std::list< std::pair<uint64_t, uint64_t> >& new_partitions){


    // sort both lists decreasingly to try to make new 
    // intervals as big as possible
    free_space.sort(cmp_free_intervals);
    const_cast<std::list< std::pair<uint64_t, uint64_t> >&>(new_partitions).sort(cmp_partitions);

    // assign free intervals to new partitions splitting as needed
    for(std::list< std::pair<uint64_t, uint64_t> >::const_iterator it = new_partitions.begin();
        it != new_partitions.end();
        ++it){

        uint64_t pid   = it->first;
        uint64_t psize = it->second;

//std::cout << "*** partition_" << pid << " size (new): " << psize << "\n";

        for(std::list< std::pair<uint64_t, uint64_t> >::iterator it2 = free_space.begin();
            it2 != free_space.end() && psize > 0;
            ++it2){

            uint64_t isize = it2->second - it2->first;

            if(isize == 0)
                continue;

            if(isize > psize){
                // assimilate partially
                
#if defined DUMP_FREE_SPACE_ASSIMILATION
                std::cout << "   isize: " << isize <<" vs. psize: " << psize << " => PARTIAL\n";
#endif
                m_interval_tree->insert_front(it2->first, it2->first+psize, pid);
                it2->first += psize;

                psize = 0;
            }
            else{
                // assimilate completely

#if defined DUMP_FREE_SPACE_ASSIMILATION
                std::cout << "   isize: " << isize <<" vs. psize: " << psize << " => COMPLETE\n";
#endif
                m_interval_tree->insert_front(it2->first, it2->second, pid);
                it2->first = 0;
                it2->second = 0;

                psize -= isize;
            }


//std::cout << "*** partition_" << pid << " psize: " << psize << "\n";
        }

#if defined DEBUG && defined DUMP_FREE_SPACE
        dump_free_space(free_space);
#endif

        
    }

    m_interval_tree->build_tree();
	
}


#if defined DEBUG
void
DistRandSlice::dump_intervals(void){

    uint64_t num_intervals = 0;

    uint64_t last_low = m_interval_tree->begin()->first;
    uint64_t last_pid = m_interval_tree->begin()->second;

    bool done = false;
    flat_segment_tree::const_iterator it = m_interval_tree->begin();

#if defined DUMP_INTERVALS_VERBOSE
    std::cout << "  DEBUG:\n\n\n";
    std::cout << "\n\nDEBUG: Dumping intervals\n";
#endif

    while(!done){

        // XXX: since the iterator for flat_segment_tree does not return the
        // last limit of the interval, we must do an extra round ...
        if(it == m_interval_tree->end())
            done = true;

        if(it->first != last_low){
            uint64_t i_low, i_high, i_pid;

            i_low = last_low;
            i_high = it->first;
            i_pid = last_pid;

#if defined DUMP_INTERVALS_VERBOSE
            std::cout << "  DEBUG:" << setw(20) << setfill('0') << i_low << ";" 
                                    << setw(20) << setfill('0') << i_high << ";" 
                                    << setw(6)  << setfill('0') << i_pid <<  ";"
                                    << setw(20) << setfill('0') << i_high - i_low << "\n";
#endif            

            ++num_intervals;

            last_low = it->first;
            last_pid = it->second;
        }

        if(!done)
            ++it;
    }

    std::cout << "\n  DEBUG: " << num_intervals << " intervals\n";

#if defined DUMP_INTERVALS_VERBOSE
    std::cout << "  DEBUG:\n\n\n";
#endif            

}

void
DistRandSlice::dump_free_space(const std::list< std::pair<uint64_t, uint64_t> >& l) const{

    uint64_t tsz = 0;

    std::cout << "\n\nDEBUG: free space stolen:\n";
    for(std::list< std::pair<uint64_t, uint64_t> >::const_iterator it = l.begin();
        it != l.end();
        ++it){

        uint64_t lo = it->first;
        uint64_t hi = it->second;
        uint64_t sz = hi - lo;

        tsz += sz;

        std::cout << "  DEBUG: { " << setw(20) << lo << "," << setw(20) << hi << " }\n";
    }

    std::cout << "Total free space: " << tsz << "\n";
}

void
DistRandSlice::verify_partitions(void){

    std::vector<uint64_t> v = compute_interval_sizes();

    for(std::tr1::unordered_map<uint64_t, Disk*>::const_iterator  it = m_disks->begin();
        it != m_disks->end();
        ++it){

        Disk* d = it->second;
        double dp = d->getCapacity()*1.0/m_capacity;

        // verify partition
        uint64_t part_id = d->getId();
        uint64_t part_capacity = (*m_partitions)[part_id];
        assert(part_capacity != 0);
        double pp = part_capacity*1.0/VSPACE_MAX;

        // verify intervals
        uint64_t interval_capacity = v[part_id];
        assert(interval_capacity != 0);
        double ip = interval_capacity*1.0/VSPACE_MAX;

        // we can't use == here :P
        double e = fabs(dp - pp);

        if(e > 1.0e-14){
            cerr.precision(100);
            cerr << "1: error in double comparison: " << dp << " vs. " << pp <<  " e=" << e << "\n";
            abort();
        }

        e = fabs(pp - ip);

        if(e > 1.0e-14){
            cerr.precision(100);
            cerr << "2: error in double comparison: " << pp << " vs. " << ip <<  " e=" << e << "\n";
            abort();
        }
    }
}


std::vector<uint64_t>
DistRandSlice::compute_interval_sizes(void){

    std::vector<uint64_t> result(m_partitions->size(), 0);

    uint64_t last_low = m_interval_tree->begin()->first;
    uint64_t last_pid = m_interval_tree->begin()->second;

    bool done = false;
    flat_segment_tree::const_iterator it = m_interval_tree->begin();

    while(!done){

        // XXX: since the iterator for flat_segment_tree does not return the
        // last limit of the interval, we must do an extra round ...
        if(it == m_interval_tree->end())
            done = true;

        if(it->first != last_low){
            uint64_t i_low, i_high, i_pid;

            i_low = last_low;
            i_high = it->first;
            i_pid = last_pid;

            result[i_pid] += (i_high - i_low);


            last_low = it->first;
            last_pid = it->second;
        }

        if(!done)
            ++it;
    }

    return result;
}
#endif
