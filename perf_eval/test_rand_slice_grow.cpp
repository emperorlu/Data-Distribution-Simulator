/*
 * File:    test_rand_slice_grow.cpp
 * Author:  amiranda
 *
 * Created on 11 Novembre 2010, 14:22
 */

#include <dadisi/Distributor.h>
#include <dadisi/DistRandSlice.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iomanip>
#include <sys/resource.h>
#include <sstream>
#include <gmpxx.h>

#include <tr1/random>

const int64_t gigabyte = ((int64_t)1)<<30;
const int64_t terabyte = ((int64_t)1)<<40;

// FIXME: receive this as program args
unsigned int seed = 0/*42*/;
const int64_t block_size = 512;
const int64_t min_disk_capacity = 100*gigabyte/block_size;
const int64_t max_disk_capacity = 3*terabyte/block_size;
const bool heterogeneous = false;
const uint32_t num_initial_disks = 50;
const uint32_t num_min_additional_disks = 5;
const uint32_t num_capacity_increase = 10; // percentage
const uint32_t num_repetitions = 66;


uint64_t disk_id = 0;

using namespace VDRIVE;
using namespace std;

std::tr1::minstd_rand rnd;

static mpz_class 
create_mpz_from_uint64(uint64_t num) {
    uint64_t tmp = num;
    tmp = tmp >> 32;
    uint32_t tmp32 = (uint32_t) tmp;
    mpz_class z(tmp32);
    z = z << 32;
    tmp = num;
    tmp = tmp << 32;
    tmp = tmp >> 32;
    tmp32 = (uint32_t) tmp;
    z = z + tmp32;
    return z;
}

static mpz_class 
timeInNS(struct timeval before, struct timeval after) {
    mpz_class beforeNS = create_mpz_from_uint64(before.tv_sec);
    beforeNS *= 1000000;
    beforeNS += before.tv_usec;
    mpz_class afterNS = create_mpz_from_uint64(after.tv_sec);
    afterNS *= 1000000;
    afterNS += after.tv_usec;
    return afterNS - beforeNS;
}

static uint64_t 
timeInMS(struct timeval before, struct timeval after) {
    uint64_t beforems = before.tv_sec * 1000 + before.tv_usec / 1000;
    uint64_t afterms = after.tv_sec * 1000 + after.tv_usec / 1000;
    return afterms - beforems;
}

static void 
copyMemdata(uint32_t num_copies) {
    try {
        ifstream inFileStream("/proc/self/status", ios::in | ios::binary);
        if (!inFileStream) {
            throw string("Could not open pidfile");
        }

		string line;
		while(getline(inFileStream, line)){
			cerr << num_copies << "K:" << line << "\n";
		}

        inFileStream.close();
        cerr << "---------------------------------------------------------\n";
    } catch (std::string s) {
        cerr << "Error wile copying pidfile:\n    " << s << "\n";
    }
}







uint64_t 
get_new_disk_id(void){
    return disk_id++;
}

void
reset_disk_id(void){
    disk_id = 0;
}

std::vector<int64_t>
get_capacities(uint32_t num_disks, int64_t total_capacity){

    std::vector<int64_t> result(num_disks);
    uint64_t current_sum = 0;

    if(heterogeneous){
        int64_t low, high, calc;

        if(max_disk_capacity * num_disks < total_capacity)
            throw std::logic_error("capacity requested too large");

        if(min_disk_capacity * num_disks > total_capacity)
            throw std::logic_error("capacity requested too small");

        for(uint32_t i=0; i<num_disks; ++i){
            calc = (total_capacity - current_sum) - (max_disk_capacity * (num_disks - 1 - i));
            low  = calc < min_disk_capacity ? min_disk_capacity : calc;
            calc = (total_capacity - current_sum) - (min_disk_capacity * (num_disks - 1 - i));
            high = calc > max_disk_capacity ? max_disk_capacity : calc;

            std::tr1::uniform_int<int64_t> unif(low, high+1);
            result[i] = unif(rnd);

            current_sum += result[i];
        }

        // the tail numbers will tend to drift higher or lower, so we should shuffle
        std::random_shuffle(result.begin(), result.end());
    }
    else{

        for(uint32_t i=0; i<num_disks; ++i){
            result[i] = max_disk_capacity;
            current_sum += result[i];
        }
    }

//    std::copy(result.begin(), result.end(), std::ostream_iterator<uint64_t>(std::cout, ","));
//    std::cout << "\n";

//    std::cout << "total_capacity: " << total_capacity << " vs. current_sum " << current_sum << "\n";

    return result;
}

list<Disk*>*
create_disks(uint32_t num_disks, int64_t& total_capacity){


    list<Disk*>* disks = new list<Disk*>();

    // compute a reasonable value for the initial capacity
    if(total_capacity == 0){
        total_capacity = num_disks*max_disk_capacity;
    }

    std::cout << "  Created capacity: " << total_capacity << " (~ " << total_capacity*block_size/terabyte << " terabytes)" << "\n\n";

    std::vector<int64_t> c = get_capacities(num_disks, total_capacity);

    for(uint32_t i=0; i<num_disks; ++i){
        Disk* d = new Disk(get_new_disk_id(), c[i], NULL);
        disks->push_back(d);
    }

    return disks;
}

void
print_memory_usage(uint32_t num_copies){

#if 0
	// a crappy way to do it, but ...
	ifstream f("/proc/self/statm");

	if(f.is_open()){
		unsigned size;		// total program size
		unsigned resident;	// resident set size
		unsigned share;		// shared pages
		unsigned text;		// text (code)
		unsigned lib;		// library
		unsigned data;		// data/stack

		f >> size;
		f >> resident;
		f >> share;
		f >> text;
		f >> lib;
		f >> data;

		f.close();

		cout << "\nMEMORY USAGE:\n";

		cout << "  total program size: " << size / 1024.0     << " MB\n";
		cout << "  resident set size: "  << resident / 1024.0 << " MB\n";
		cout << "  shared pages: "       << share / 1024.0    << " MB\n";
		cout << "  text (code): "        << text / 1024.0     << " MB\n";
		cout << "  library: "            << lib / 1024.0      << " MB\n";
		cout << "  data/stack: "         << data / 1024.0     << " MB\n";
	}
	else{
		cout << "unable to get memory usage\n";
	}
#else
    
    pid_t pid = getpid();
    
    stringstream cmd;
    cmd << "ps -p " << pid << " o rss h";

    cout << num_copies << "K:Memory usage: ";
    cout.flush();
    system(cmd.str().c_str());

#endif
}

void
print_capacities(const std::list<Disk*>* disks){

    for(std::list<Disk*>::const_iterator it = disks->begin();
        it != disks->end();
        ++it){

        Disk* d = *it;

        cout << d->getId() << ";" << d->getCapacity() << "\n";
    }
}

void
hom_mem_time_test(int argc, char* argv[]){

    cout << "*** Memory and Time consumption test (homogeneus)\n";

#define cout (std::cout << num_copies << "K:" )

    uint32_t disks[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    uint32_t copies[] = {0, 1, 2, 3}; // XXX we only support 1 copy atm

    for(uint32_t j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){

		reset_disk_id();

        uint32_t num_copies = 1<<copies[j];
		uint32_t disk_size = 500000;
		uint64_t num_extents = 1000000;
		uint32_t old_disks = 0;
		uint64_t first_extent_number = 0;
		tr1::unordered_map<int64_t, int64_t> disk_sizes, disk_usage;

        Distributor* dist = NULL;

        for(uint32_t i=0; i<sizeof(disks)/sizeof(disks[0]); ++i){

            mpz_class all_sum_times, all_sum_square_times, step_time;
            all_sum_times = all_sum_square_times = step_time = 0;
            
            list<Disk*> disk_base;
            uint32_t num_disks = (1<<disks[i]) - old_disks;
            old_disks += num_disks;

            // create disks
            for(uint32_t j=0; j<num_disks; ++j){
                Disk* d = new Disk(get_new_disk_id(), disk_size, NULL);

                disk_sizes[d->getId()] = disk_size;
                disk_usage[d->getId()] = 0;

                disk_base.push_back(d);
            }

            cout << "Created " << num_disks << " Disks\n";
            cout << "Distributor now contains " << old_disks << " Disks\n";

            struct timeval before, after, stepbefore, stepafter;

            cout << "Will set configuration,\n";
            gettimeofday(&before, 0);

            // add disks to distributor
            if(dist == NULL){
                dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
                dist->setConfiguration(&disk_base, 1, num_copies);
            }
            else{
                dist->setDisks(&disk_base);
            }

            gettimeofday(&after, 0);

            copyMemdata(num_copies);

/*cout << "  num_partitions,num_intervals: " 
     << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
     << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";*/

            cout << "Used Time for Initialisation:" << timeInMS(before, after) << "\n";

            // place extents
            cout << "Will Place Extents without pThreads\n";
            gettimeofday(&before, 0);
            
            uint64_t last_extent_number = first_extent_number + num_extents;
            for(uint64_t i=first_extent_number; i<last_extent_number; ++i){

                gettimeofday(&stepbefore, 0);

                std::list<Disk*>* chosen_disks = dist->placeExtent(0, i);

                gettimeofday(&stepafter, 0);

                step_time = timeInNS(stepbefore, stepafter);
                all_sum_times += step_time;
                step_time *= step_time;
                all_sum_square_times += step_time;

                for(list<Disk*>::iterator it = chosen_disks->begin();
                    it != chosen_disks->end();
                    ++it){
                    ++disk_usage[(*it)->getId()];
                    delete *it;
                }

                delete chosen_disks;
            }

            gettimeofday(&after, 0);

            cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";
            cout << "Placed Extents:" << num_extents << "\n";
            cout << "Placed copies of each Extent:" << num_copies << "\n";
            cout << "Sum time of placing single Extents:" << all_sum_times.get_str(10) << "\n";
            cout << "Sum of squares of placing single Extents:" << all_sum_square_times.get_str(10) << "\n";

            mp_exp_t ext;
            mpz_class tmp1 = (create_mpz_from_uint64(num_extents) * all_sum_square_times) - (all_sum_times * all_sum_times);
            mpf_class variance = mpf_class(tmp1) / create_mpz_from_uint64(num_extents * (num_extents-1)); 
            
#ifdef my_gmp_ref            
            cout << "Varianz of time placing Extents:" << variance.get_str(ext, 10, 0) << "\n";
#else
            cout << "Varianz of time placing Extents:" << variance.get_str(&ext, 10, 0) << "\n";
#endif                      
            cout << "Varianz of time placing Extents Exponend:" << ext << "\n";

            mpf_class deviation = sqrt(variance);

#ifdef my_gmp_ref            
            cout << "Standard Deviation placing Extents:" << deviation.get_str(ext, 10, 0) << "\n";
#else
            cout << "Standard Deviation placing Extents:" << deviation.get_str(&ext, 10, 0) << "\n";
#endif
            cout << "Standard Deviation placing Extents Exponend:" << ext << "\n";
            cout << "Used Threads to place Extents:1\n";

            print_memory_usage(num_copies);

            for(uint32_t i=0; i<old_disks; ++i){
                cout << i << ";" << disk_usage[i] << ";" << disk_sizes[i] << "\n";
            }

#undef cout
            cout << "--------------------------------------------------------------------------\n";

            // cleanup
            for(list<Disk*>::iterator it=disk_base.begin();
                it != disk_base.end();
                ++it){
                delete *it;
            }
        }

        delete dist;
    }
}


void
het_mem_time_test(int argc, char* argv[]){

    cout << "*** Memory and Time consumption test (heterogeneous)\n";

#define cout (std::cout << num_copies << "K:" )

    uint32_t num_steps = 10;
    uint32_t disks_per_step = 128;
    uint32_t copies[] = {0, 1, 2, 3}; // XXX we only support 1 copy atm

    for(uint32_t j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){

    	reset_disk_id();
        
		uint32_t num_copies = 1<<copies[j];
		uint32_t disk_size = 500000;
		uint64_t num_extents = 1000000;
		tr1::unordered_map<int64_t, int64_t> disk_sizes, disk_usage;
		uint32_t old_disks = 0;
		uint64_t first_extent_number = 0;

        Distributor* dist = NULL;

        for(uint32_t i=0; i<num_steps; ++i){

            mpz_class all_sum_times, all_sum_square_times, step_time;
            all_sum_times = all_sum_square_times = step_time = 0;
            
            list<Disk*> disk_base;
            uint32_t num_disks = disks_per_step*(i+1) - old_disks;
            old_disks += num_disks;

            // create disks
            for(uint32_t j=0; j<num_disks; ++j){
                Disk* d = new Disk(get_new_disk_id(), disk_size, NULL);

                disk_sizes[d->getId()] = disk_size;
                disk_usage[d->getId()] = 0;

                disk_base.push_back(d);
            }

            cout << "Created " << num_disks << " Disks " << "\n";
            cout << "Distributor now contains " << old_disks << " Disks\n";

            disk_size *= 1.5;

            struct timeval before, after, stepbefore, stepafter;

            cout << "Will set configuration,\n";
            gettimeofday(&before, 0);

            // add disks to distributor
            if(dist == NULL){
                dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
                dist->setConfiguration(&disk_base, 1, num_copies);
            }
            else{
                dist->setDisks(&disk_base);
            }

            gettimeofday(&after, 0);

            copyMemdata(num_copies);

/*cout << "  num_partitions,num_intervals: " 
     << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
     << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";*/

            cout << "Used Time for Initialisation:" << timeInMS(before, after) << "\n";

            // place extents
            cout << "Will Place Extents without pThreads\n";
            gettimeofday(&before, 0);
            
            uint64_t last_extent_number = first_extent_number + num_extents;
            for(uint64_t i=first_extent_number; i< last_extent_number; ++i){

                gettimeofday(&stepbefore, 0);

                std::list<Disk*>* chosen_disks = dist->placeExtent(0, i);

                gettimeofday(&stepafter, 0);

                step_time = timeInNS(stepbefore, stepafter);
                all_sum_times += step_time;
                step_time *= step_time;
                all_sum_square_times += step_time;

                for(list<Disk*>::iterator it = chosen_disks->begin();
                    it != chosen_disks->end();
                    ++it){
                    ++disk_usage[(*it)->getId()];
                    delete *it;
                }

                delete chosen_disks;
            }

            gettimeofday(&after, 0);

            cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";
            cout << "Placed Extents:" << num_extents << "\n";
            cout << "Placed copies of each Extent:" << num_copies << "\n";
            cout << "Sum time of placing single Extents:" << all_sum_times.get_str(10) << "\n";
            cout << "Sum of squares of placing single Extents:" << all_sum_square_times.get_str(10) << "\n";

            mp_exp_t ext;
            mpz_class tmp1 = (create_mpz_from_uint64(num_extents) * all_sum_square_times) - (all_sum_times * all_sum_times);
            mpf_class variance = mpf_class(tmp1) / create_mpz_from_uint64(num_extents * (num_extents-1)); 
            
#ifdef my_gmp_ref
            cout << "Varianz of time placing Extents:" << variance.get_str(ext, 10, 0) << "\n";
#else
            cout << "Varianz of time placing Extents:" << variance.get_str(&ext, 10, 0) << "\n";
#endif
            cout << "Varianz of time placing Extents Exponend:" << ext << "\n";

            mpf_class deviation = sqrt(variance);
#ifdef my_gmp_ref            
            cout << "Standard Deviation placing Extents:" << deviation.get_str(ext, 10, 0) << "\n";
#else
            cout << "Standard Deviation placing Extents:" << deviation.get_str(&ext, 10, 0) << "\n";
#endif
            cout << "Standard Deviation placing Extents Exponend:" << ext << "\n";
            cout << "Used Threads to place Extents:1\n";

            print_memory_usage(num_copies);

            for(uint32_t i=0; i<old_disks; ++i){
                cout << i << ";" << disk_usage[i] << ";" << disk_sizes[i] << "\n";
            }

#undef cout
            cout << "--------------------------------------------------------------------------\n";

            // cleanup
            for(list<Disk*>::iterator it=disk_base.begin();
                it != disk_base.end();
                ++it){
                delete *it;
            }
        }

        delete dist;
    }
}

void
hom_fairness_test(int argc, char* argv[]){

    cout << "*** Fairness test (homogeneous)\n";

#define cout (std::cout << num_copies << "K:" )

    uint32_t disks[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    //uint32_t disks[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    uint32_t copies[] = {0,1,2,3}; // XXX we only support 1 copy atm

    for(uint32_t j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){

		reset_disk_id();

        uint32_t num_copies = 1<<copies[j];
		uint64_t disk_size = 500000;
		uint32_t usage_factor = 2;
		uint32_t old_disks = 0;
		uint64_t first_extent_number = 0;
		tr1::unordered_map<int64_t, int64_t> disk_sizes, disk_usage;

        Distributor* dist = NULL;

        for(uint32_t i=0; i<sizeof(disks)/sizeof(disks[0]); ++i){

            mpz_class all_sum_times, all_sum_square_times, step_time;
            all_sum_times = all_sum_square_times = step_time = 0;
            
            list<Disk*> disk_base;
            uint32_t num_disks = (1<<disks[i]) - old_disks;
            old_disks += num_disks;

            // create disks
            for(uint32_t j=0; j<num_disks; ++j){
                Disk* d = new Disk(get_new_disk_id(), disk_size, NULL);

                disk_sizes[d->getId()] = disk_size;
                disk_usage[d->getId()] = 0;

                disk_base.push_back(d);
            }

            // since we're not migrating, clean usage
            for(tr1::unordered_map<int64_t, int64_t>::iterator it=disk_usage.begin();
                it != disk_usage.end();
                ++it){

                it->second = 0;
            }

            cout << "Created " << num_disks << " Disks\n";
            cout << "Distributor now contains " << old_disks << " Disks\n";

            uint64_t num_extents = disk_size*old_disks/usage_factor;

            struct timeval before, after, stepbefore, stepafter;


            cout << "Will set configuration,\n";
            gettimeofday(&before, 0);

            // add disks to distributor
            if(dist == NULL){
                dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
                dist->setConfiguration(&disk_base, 1, num_copies);
            }
            else{
                dist->setDisks(&disk_base);
            }

            gettimeofday(&after, 0);

            copyMemdata(num_copies);

/*cout << "  num_partitions,num_intervals: " 
     << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
     << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";*/

            cout << "Used Time for Initialisation:" << timeInMS(before, after) << "\n";

            // place extents
            cout << "Will Place Extents without pThreads\n";
            gettimeofday(&before, 0);

            uint64_t last_extent_number = first_extent_number + num_extents;
            for(uint64_t i=first_extent_number; i<last_extent_number; ++i){

                gettimeofday(&stepbefore, 0);

                std::list<Disk*>* chosen_disks = dist->placeExtent(0, i);

                gettimeofday(&stepafter, 0);

                step_time = timeInNS(stepbefore, stepafter);
                all_sum_times += step_time;
                step_time *= step_time;
                all_sum_square_times += step_time;

                for(list<Disk*>::iterator it = chosen_disks->begin();
                    it != chosen_disks->end();
                    ++it){
                    ++disk_usage[(*it)->getId()];
                    delete *it;
                }

                delete chosen_disks;
            }

            gettimeofday(&after, 0);

            cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";
            cout << "Placed Extents:" << num_extents << "\n";
            cout << "Placed copies of each Extent:" << num_copies << "\n";
            cout << "Sum time of placing single Extents:" << all_sum_times.get_str(10) << "\n";
            cout << "Sum of squares of placing single Extents:" << all_sum_square_times.get_str(10) << "\n";

            mp_exp_t ext;
            mpz_class tmp1 = (create_mpz_from_uint64(num_extents) * all_sum_square_times) - (all_sum_times * all_sum_times);
            mpf_class variance = mpf_class(tmp1) / create_mpz_from_uint64(num_extents * (num_extents-1)); 
            
#ifdef my_gmp_ref
            cout << "Varianz of time placing Extents:" << variance.get_str(ext, 10, 0) << "\n";
#else
            cout << "Varianz of time placing Extents:" << variance.get_str(&ext, 10, 0) << "\n";
#endif

            cout << "Varianz of time placing Extents Exponend:" << ext << "\n";

            mpf_class deviation = sqrt(variance);
#ifdef my_gmp_ref
            cout << "Standard Deviation placing Extents:" << deviation.get_str(ext, 10, 0) << "\n";
#else
            cout << "Standard Deviation placing Extents:" << deviation.get_str(&ext, 10, 0) << "\n";
#endif

            cout << "Standard Deviation placing Extents Exponend:" << ext << "\n";
            cout << "Used Threads to place Extents:1\n";

            for(uint32_t i=0; i<old_disks; ++i){
                cout << i << ";" << disk_usage[i] << ";" << disk_sizes[i] << "\n";
            }

#undef cout
            cout << "--------------------------------------------------------------------------\n";
        }
    }

}

void
het_fairness_test(int argc, char* argv[]){

    cout << "*** Fairness test (heterogeneous)\n";

#define cout (std::cout << num_copies << "K:" )

    uint32_t num_steps = 10;
    uint32_t disks_per_step = 128;
    uint32_t copies[] = {0,1,2,3}; // XXX we only support 1 copy atm

    for(uint32_t j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){

        reset_disk_id();

        uint32_t num_copies = 1<<copies[j];
		uint64_t disk_size = 500000;
		uint32_t total_capacity = 0;
		uint32_t usage_factor = 2;
		uint64_t first_extent_number = 0;
		uint32_t old_disks = 0;
		tr1::unordered_map<int64_t, int64_t> disk_sizes, disk_usage;

        Distributor* dist = NULL;

        for(uint32_t i=0; i<num_steps; ++i){

            mpz_class all_sum_times, all_sum_square_times, step_time;
            all_sum_times = all_sum_square_times = step_time = 0;
            
            list<Disk*> disk_base;
            uint32_t num_disks = disks_per_step*(i+1) - old_disks;
            old_disks += num_disks;
            total_capacity += (num_disks*disk_size);

            // create disks
            for(uint32_t j=0; j<num_disks; ++j){
                Disk* d = new Disk(get_new_disk_id(), disk_size, NULL);

                disk_sizes[d->getId()] = disk_size;
                disk_usage[d->getId()] = 0;

                disk_base.push_back(d);
            }

            // since we're not migrating, clean usage
            for(tr1::unordered_map<int64_t, int64_t>::iterator it=disk_usage.begin();
                it != disk_usage.end();
                ++it){

                it->second = 0;
            }

            cout << "Created " << num_disks << " Disks " << "\n";
            cout << "Distributor now contains " << old_disks << " Disks\n";

            uint64_t num_extents = total_capacity/usage_factor;
            disk_size *= 1.5;

            struct timeval before, after, stepbefore, stepafter;


            cout << "Will set configuration,\n";
            gettimeofday(&before, 0);

            // add disks to distributor
            if(dist == NULL){
                dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
                dist->setConfiguration(&disk_base, 1, num_copies);
            }
            else{
                dist->setDisks(&disk_base);
            }

            gettimeofday(&after, 0);

            copyMemdata(num_copies);

/*cout << "  num_partitions,num_intervals: " 
     << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
     << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";*/

            cout << "Used Time for Initialisation:" << timeInMS(before, after) << "\n";

            // place extents
            cout << "Will Place Extents without pThreads\n";
            gettimeofday(&before, 0);
            
            uint64_t last_extent_number = first_extent_number + num_extents;
            for(uint64_t i=first_extent_number; i< last_extent_number; ++i){

                gettimeofday(&stepbefore, 0);

                std::list<Disk*>* chosen_disks = dist->placeExtent(0, i);

                gettimeofday(&stepafter, 0);

                step_time = timeInNS(stepbefore, stepafter);
                all_sum_times += step_time;
                step_time *= step_time;
                all_sum_square_times += step_time;

                for(list<Disk*>::iterator it = chosen_disks->begin();
                    it != chosen_disks->end();
                    ++it){
                    ++disk_usage[(*it)->getId()];
                    delete *it;
                }

                delete chosen_disks;
            }

            gettimeofday(&after, 0);

            cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";
            cout << "Placed Extents:" << num_extents << "\n";
            cout << "Placed copies of each Extent:" << num_copies << "\n";
            cout << "Sum time of placing single Extents:" << all_sum_times.get_str(10) << "\n";
            cout << "Sum of squares of placing single Extents:" << all_sum_square_times.get_str(10) << "\n";

            mp_exp_t ext;
            mpz_class tmp1 = (create_mpz_from_uint64(num_extents) * all_sum_square_times) - (all_sum_times * all_sum_times);
            mpf_class variance = mpf_class(tmp1) / create_mpz_from_uint64(num_extents * (num_extents-1)); 
            
#ifdef my_gmp_ref
            cout << "Varianz of time placing Extents:" << variance.get_str(ext, 10, 0) << "\n";
#else
            cout << "Varianz of time placing Extents:" << variance.get_str(&ext, 10, 0) << "\n";
#endif

            cout << "Varianz of time placing Extents Exponend:" << ext << "\n";

            mpf_class deviation = sqrt(variance);
#ifdef my_gmp_ref            
            cout << "Standard Deviation placing Extents:" << deviation.get_str(ext, 10, 0) << "\n";
#else            
            cout << "Standard Deviation placing Extents:" << deviation.get_str(&ext, 10, 0) << "\n";
#endif            
            cout << "Standard Deviation placing Extents Exponend:" << ext << "\n";
            cout << "Used Threads to place Extents:1\n";

            for(uint32_t i=0; i<old_disks; ++i){
                cout << i << ";" << disk_usage[i] << ";" << disk_sizes[i] << "\n";
            }

#undef cout
            cout << "--------------------------------------------------------------------------\n";
        }
    }
}


Distributor*
build_distributor(int argc, char* argv[], 
                  uint32_t num_initial_disks, uint32_t initial_disk_size,
                  uint32_t num_additional_disks, uint32_t additional_disk_size,
                  uint32_t num_copies, uint32_t num_extents){

    reset_disk_id();

    Distributor* dist = NULL;

    list<Disk*> disk_base;

    // create disks
    for(uint32_t j=0; j<num_initial_disks; ++j){
        Disk* d = new Disk(get_new_disk_id(), initial_disk_size, NULL);
        disk_base.push_back(d);
    }

    // add disks to distributor
    dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
    dist->setConfiguration(&disk_base, 1, num_copies);

    cout << "Created " << num_initial_disks + num_additional_disks << " Disks\n";
    cout << "Distributor now contains " << num_initial_disks + num_additional_disks << " Disks\n";

    if(num_additional_disks != 0){

        list<Disk*> ad;

        for(uint32_t j=0; j<num_additional_disks; ++j){
            Disk* d = new Disk(get_new_disk_id(), additional_disk_size, NULL);
            ad.push_back(d);
        }

        dist->setDisks(&ad);
    }



    return dist;
}

void 
hom_adaptivity_test(int argc, char* argv[]){

    cout << "*** Adaptivity test (homogeneous)\n";
    reset_disk_id();

    uint32_t num_disks = 128;
    uint32_t additional_disks[] = {1,2,3,5,7,11,13};

    uint32_t copies[] = {0,1,2,3}; // XXX we only support 1 copy atm
    uint32_t disk_size = 500000;
    uint32_t num_extents = 250000*num_disks;

    for(uint j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){
        for(uint i=0; i<sizeof(additional_disks)/sizeof(additional_disks[0]); ++i){

            stringstream dist1_id;
            stringstream dist2_id;

            uint32_t num_copies = 1<<copies[j];


            Distributor* dist1 = build_distributor(argc, argv, 
                                                   num_disks, disk_size, 
                                                   0, 0, num_copies, num_extents);

            Distributor* dist2 = build_distributor(argc, argv, 
                                                   num_disks, disk_size,
                                                   additional_disks[i], disk_size,
                                                   num_copies, num_extents);

            dist1_id << "RandSlice_" << num_copies << "K_1T-" << num_disks << "D_hom_0.disks-0.dist";
            dist2_id << "RandSlice_" << num_copies << "K_1T-" << num_disks+additional_disks[i] << "D_hom_0.disks-0.dist";

            int64_t moved_with_order = 0;
            int64_t moved_without_order = 0;
            int64_t moved_from_removed = 0;
            int64_t moved_to_added = 0;

            // place extents
            for(uint64_t k=0; k<num_extents; ++k){

                std::list<Disk*>* chosen_disks1 = dist1->placeExtent(0, k);
                std::list<Disk*>* chosen_disks2 = dist2->placeExtent(0, k);
                list<Disk*>::iterator it1 = chosen_disks1->begin();
                list<Disk*>::iterator it2 = chosen_disks2->begin();

                if(k%500000 == 0){
                    cout << "placed " << k << " of " << num_extents << "\n";
                    cout.flush();
                }

                vector<uint64_t> d1;
                vector<uint64_t> d2;

                while(it1 != chosen_disks1->end() && it2 != chosen_disks2->end()){

                    Disk* old_disk = *it1;
                    Disk* new_disk = *it2;

                    if(new_disk->getId() >= num_disks)
                        ++moved_to_added;

                    if(old_disk->getId() != new_disk->getId())
                        ++moved_with_order;

                    d1.push_back(old_disk->getId());
                    d2.push_back(new_disk->getId());

                    ++it1;
                    ++it2;

                    delete old_disk;
                    delete new_disk;
                }

/*
cout << "D"<< num_disks << ":d1: ";
copy(d1.begin(), d1.end(), ostream_iterator<uint64_t>(cout, ","));
cout << "\n";

cout << "D"<< num_disks << ":d2: ";
copy(d2.begin(), d2.end(), ostream_iterator<uint64_t>(cout, ","));
cout << "\n";
*/

                sort(d1.begin(), d1.end());
                sort(d2.begin(), d2.end());
                vector<uint64_t> diff(d1.size()+d2.size());
                vector<uint64_t>::iterator it = set_difference(d1.begin(), d1.end(), d2.begin(), d2.end(), diff.begin());

                moved_without_order += int64_t(it - diff.begin());

//cout << "dif " << int(it - diff.begin()) << "\n";

                delete chosen_disks1;
                delete chosen_disks2;
            }

            cout << dist1_id.str() << ";" << dist2_id.str() << ";" 
                 << num_disks*disk_size << ";" << (num_disks+additional_disks[i])*disk_size << ";"
                 << additional_disks[i]*disk_size << ";" << 0 << ";" 
                 << 0 << ";" << moved_with_order << ";"
                 << moved_without_order << ";" << moved_from_removed << ";"
                 << moved_to_added << "\n";
            cout.flush();

            delete dist1;
            delete dist2;
        }
    }
}

void 
het_adaptivity_test(int argc, char* argv[]){

    cout << "*** Adaptivity test (heterogeneous)\n";
    reset_disk_id();

    uint32_t num_disks = 128;
    uint32_t additional_disks[] = {1,2,3,5,7,11,13};

    uint32_t copies[] = {/*0,1,2,*/3}; // XXX we only support 1 copy atm
    uint32_t disk_size = 500000;
    uint32_t num_extents = 250000*num_disks;

    for(uint j=0; j<sizeof(copies)/sizeof(copies[0]); ++j){
        for(uint i=0; i<sizeof(additional_disks)/sizeof(additional_disks[0]); ++i){

            stringstream dist1_id;
            stringstream dist2_id;

            uint32_t num_copies = 1<<copies[j];

            Distributor* dist1 = build_distributor(argc, argv, 
                                                   num_disks, disk_size, 
                                                   0,0,
                                                   num_copies, num_extents);

            Distributor* dist2 = build_distributor(argc, argv, 
                                                   num_disks, disk_size,
                                                   additional_disks[i], disk_size*1.5, 
                                                   num_copies, num_extents); 

            dist1_id << "RandSlice_" << num_copies << "K_1T-" << num_disks << "D_het_0.disks-0.dist";
            dist2_id << "RandSlice_" << num_copies << "K_1T-" << num_disks+additional_disks[i] << "D_het_0.disks-0.dist";

            int64_t moved_with_order = 0;
            int64_t moved_without_order = 0;
            int64_t moved_from_removed = 0;
            int64_t moved_to_added = 0;

            // place extents
            for(uint64_t k=0; k<num_extents; ++k){

                std::list<Disk*>* chosen_disks1 = dist1->placeExtent(0, k);
                std::list<Disk*>* chosen_disks2 = dist2->placeExtent(0, k);
                list<Disk*>::iterator it1 = chosen_disks1->begin();
                list<Disk*>::iterator it2 = chosen_disks2->begin();
                
                if(k%500000 == 0){
                    cout << "placed " << k << " of " << num_extents << "\n";
                    cout.flush();
                }

                vector<uint64_t> d1;
                vector<uint64_t> d2;

                while(it1 != chosen_disks1->end() && it2 != chosen_disks2->end()){

                    Disk* old_disk = *it1;
                    Disk* new_disk = *it2;

                    if(new_disk->getId() >= num_disks)
                        ++moved_to_added;

                    if(old_disk->getId() != new_disk->getId())
                        ++moved_with_order;

                    d1.push_back(old_disk->getId());
                    d2.push_back(new_disk->getId());

                    ++it1;
                    ++it2;

                    delete old_disk;
                    delete new_disk;
                }

/*cout << "d1: ";
copy(d1.begin(), d1.end(), ostream_iterator<uint64_t>(cout, ","));
cout << "\n";

cout << "d2: ";
copy(d2.begin(), d2.end(), ostream_iterator<uint64_t>(cout, ","));
cout << "\n";*/


                sort(d1.begin(), d1.end());
                sort(d2.begin(), d2.end());
                vector<uint64_t> diff(d1.size()+d2.size());
                vector<uint64_t>::iterator it = set_difference(d1.begin(), d1.end(), d2.begin(), d2.end(), diff.begin());

                moved_without_order += int64_t(it - diff.begin());

//cout << "difference " << int(it - diff.begin()) << "\n";

                delete chosen_disks1;
                delete chosen_disks2;
            }

            cout << dist1_id.str() << ";" << dist2_id.str() << ";" 
                 << num_disks*disk_size << ";" << (num_disks+additional_disks[i])*disk_size << ";"
                 << additional_disks[i]*disk_size << ";" << 0 << ";" 
                 << 0 << ";" << moved_with_order << ";"
                 << moved_without_order << ";" << moved_from_removed << ";"
                 << moved_to_added << "\n";

            cout.flush();

            delete dist1;
            delete dist2;
        }
    }

}

void
het_fixed_growth_intervals(int argc, char* argv[]){

    cout << "*** Interval growth (fixed heterogeneous increment)\n";
    reset_disk_id();

    uint32_t num_disks = 128;
    uint32_t additional_disks = 16;
    uint32_t nd = num_disks;
    uint32_t num_steps = 128;
    uint32_t disk_size = 500000;
    double size_increment = 1.5;

    Distributor* dist = NULL;
    list<Disk*>* disks = new list<Disk*>();

    for(uint i=0; i<num_steps; ++i){

        // create disks
        for(uint32_t j=0; j<nd; ++j){
            Disk* d = new Disk(get_new_disk_id(), disk_size, NULL);
            disks->push_back(d);
        }

        cout << "Created " << num_disks << " Disks\n";
        cout << "Distributor now contains " << num_disks << " Disks\n";

        if(dist == NULL){
            cout << "Will set configuration,\n";
            dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc, argv);
            dist->setConfiguration(disks, 1, 1);
        }
        else{
            cout << "Will add disks\n";
            dist->setDisks(disks);
        }

        cout << "  num_disks,num_partitions,num_intervals: " 
             << num_disks << "," 
             << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
             << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";

        disks->clear();

        nd = additional_disks;
        num_disks += additional_disks;
        disk_size *= size_increment;
    }
}

int
main(int argc, char** argv){

    if(argc != 2)
        goto help;

 
    if(!strcmp(argv[1], "--hom-mem-time")){
        hom_mem_time_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--het-mem-time")){
        het_mem_time_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--hom-fairness")){
        hom_fairness_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--het-fairness")){
        het_fairness_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--hom-adaptivity")){
        hom_adaptivity_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--het-adaptivity")){
        het_adaptivity_test(argc-2, &argv[2]);
    }
    else if(!strcmp(argv[1], "--het-interval-growth")){
        het_fixed_growth_intervals(argc-2, &argv[2]);
    }
    else{
help:        
        cerr << "Usage: " << argv[0] << " --hom-mem-time | --het-mem-time | --hom-fairness | --het-fairness | --hom-adaptivity | --het-adaptivity\n";
        exit(1);
    }

    exit(0);

#if 0
    Distributor* dist;
    list<Disk*>* initial_disks;
    list<Disk*>* additional_disks;
    int64_t      current_capacity = 0;

    if(seed == 0)
        seed = time(NULL);

    cout.setf(ios::unitbuf);

    cout << "Setting seed for tests\n";
    cout << "   seed = " << seed << "\n";

    rnd.seed(seed);

    cout << "\nCreating " << num_initial_disks 
         << (heterogeneous ? " heterogeneous" : " homogeneus") << " disks\n";
    
    initial_disks = create_disks(num_initial_disks, current_capacity);


    cout << "\nCreating a new RandSlice Distributor\n";
    
    dist = Distributor::createDistributor(Distributor::RAND_SLICE, argc-1, &argv[1]);



    cout << "\nSetting configuration and adding disks to distributor\n";

    dist->setConfiguration(initial_disks, 1, 1);

	cout << "  num_disks,num_partitions,num_intervals: " 
		 << initial_disks->size() << "," 
		 << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
		 << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";


    for(uint32_t i = 0; i < num_repetitions; ++i){

        cout << "\nRound " << i+1 << " of " << num_repetitions << "\n";

        int64_t additional_capacity = current_capacity*num_capacity_increase/100;
        uint32_t num_additional_disks = num_min_additional_disks;

        while(num_additional_disks*max_disk_capacity < additional_capacity)
            num_additional_disks++;

        cout << "\n  Creating " << num_additional_disks 
             << " additional" << (heterogeneous ? " heterogeneous" : " homogeneus") 
             << " disks with an overall " << num_capacity_increase << "\% of current capacity\n";

        additional_disks = create_disks(num_additional_disks, additional_capacity);
        current_capacity += additional_capacity;

        cout << "  Adding additional disks to distributor\n";

        dist->setDisks(additional_disks);

        cout << "    disks added\n";

		initial_disks->splice(initial_disks->end(), *additional_disks);
		delete additional_disks;

		cout << "    num_disks,num_partitions,num_intervals: " 
			 << initial_disks->size() << "," 
			 << static_cast<DistRandSlice*>(dist)->getNumPartitions() << ","
			 << static_cast<DistRandSlice*>(dist)->getNumIntervals() << "\n";
    }

	//print_memory_usage();
    //print_capacities(initial_disks);

	cout << "\n\ntest finished\n";

	// cleanup
    for(std::list<Disk*>::iterator it = initial_disks->begin();
		it != initial_disks->end();
		++it){

		delete *it;
	}
	
	delete initial_disks;
	delete dist;
#endif
}
