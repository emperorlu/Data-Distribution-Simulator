/*
 * File:   fileDistTester.cpp
 * Author: Hubert Doemer
 *
 * Created on 04.10.2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <cerrno>
#include <dadisi/Distributor.h>
#include <sys/time.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <sstream>
#include <vector>
#include <map>
#include <tr1/unordered_map>
#include <pthread.h>
#include <gmpxx.h>


using namespace VDRIVE;
using namespace std;

struct threadParam {
    int32_t fileNum;
    int32_t threadId;
    Distributor* fileDist;
    int32_t blockDistType;
    int32_t numBlocks;
    int32_t numCopies;
    int32_t numBlockCopies;
    std::tr1::unordered_map<int64_t, int64_t>* results;
    char **argv;
    size_t argc;
};

static std::tr1::unordered_map<int64_t, mpz_class> sum_creationTimes, sum_placeTimes;
static std::tr1::unordered_map<int64_t, mpz_class> sum_square_creationTimes, sum_square_placeTimes;

static mpz_class create_mpz_from_uint64(uint64_t num) {
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

static mpz_class timeInNS(struct timeval before, struct timeval after) {
    mpz_class beforeNS = create_mpz_from_uint64(before.tv_sec);
    beforeNS *= 1000000;
    beforeNS += before.tv_usec;
    mpz_class afterNS = create_mpz_from_uint64(after.tv_sec);
    afterNS *= 1000000;
    afterNS += after.tv_usec;
    return afterNS - beforeNS;
}

static uint64_t timeInMS(struct timeval before, struct timeval after) {
    uint64_t beforems = before.tv_sec * 1000 + before.tv_usec / 1000;
    uint64_t afterms = after.tv_sec * 1000 + after.tv_usec / 1000;
    return afterms - beforems;
}

static void usage() {
    cout << "\nUsage:\n" <<
            "fileDistTester [options]\n" <<
            "This Programm runs some Distribution test and exits\n" <<
            "Options:\n" <<
            "   -fdt <file distributor type>  use given distributor for file distribution\n" <<
            "   -bdt <block distributor type> use given distributor for block distribution\n" <<
            "   -Dn N                         number of Disks, that shall be used\n" <<
            "   -Dif <diskFile>               read Disks from XML File\n" <<
            "   -Fn N                         number of Files, that shall be used\n" <<
            "   -Cn N                         number of File Copies, that shall be used\n" <<
            "   -Bn N                         number of Blocks, that shall be used\n" <<
            "   -BCn N                        number of Block Copies, that shall be used\n" <<
            "   -Tn N                         number of Threads\n" <<
            "   -of <outFile>                 write result to given File\n" <<
            "   -fdcp \"parameters\"          parameters for file distributor\n" <<
            "   -bdcp \"parameters\"          parameters for block distributor\n" <<
            "   -tt                           run timed tests\n";
}

static void deleteList(list<Disk*> *delList) {
    for (std::list<Disk*>::iterator it = delList->begin(); it != delList->end(); it++) {
        delete *it;
    }
}

void *runTimedTest(void *arg) {
    Distributor * blockDist;
    struct threadParam *tp = (struct threadParam*) arg;
    struct timeval before, after;
    mpz_class ct_step = 0, ct_sums = 0, ct_square_sums = 0;
    mpz_class pt_step = 0, pt_sums = 0, pt_square_sums = 0;

    /* File distribution */
    for (int64_t i = 0; i < tp->fileNum; i++) {

        std::list<Disk*>* chosenDisks = tp->fileDist->placeExtent(0, i);

        gettimeofday(&before, 0);
        if(tp->argc > 0){
            blockDist = Distributor::createDistributor(tp->blockDistType, tp->argc, tp->argv);
        }
        else {
            blockDist = Distributor::createDistributor(tp->blockDistType, 0, 0);
        }
        blockDist->setConfiguration(chosenDisks, 1, tp->numBlockCopies);
        gettimeofday(&after, 0);

        ct_step = timeInNS(before, after);
        //cout << tp->threadId << ": ct " << ct_step.get_str(10) << "\n";
        ct_sums += ct_step;
        ct_step *= ct_step;
        ct_square_sums += ct_step;

        for (list<Disk*>::iterator diskIterator = (*chosenDisks).begin(); diskIterator != (*chosenDisks).end(); ++diskIterator) {
            delete *diskIterator;
        }

        /* Block distribution */
        for (int64_t j = 0; j < tp->numBlocks; j++) {
            gettimeofday(&before, 0);
            std::list<Disk*>* choosenDisksForBlocks = blockDist->placeExtent(0, j);
            gettimeofday(&after, 0);
            pt_step = timeInNS(before, after);
            pt_sums += pt_step;
            pt_step *= pt_step;
            pt_square_sums += pt_step;

            for (list<Disk*>::iterator it = (*choosenDisksForBlocks).begin(); it != (*choosenDisksForBlocks).end(); ++it) {
                (*tp->results)[(*it)->getId()]++;
                delete *it;
            }
            delete choosenDisksForBlocks;
        }
        delete blockDist;
        delete chosenDisks;
    }
    sum_creationTimes[tp->threadId] = ct_sums;
    sum_square_creationTimes[tp->threadId] = ct_square_sums;
    sum_placeTimes[tp->threadId] = pt_sums;
    sum_square_placeTimes[tp->threadId] = pt_square_sums;
    return NULL;
}

void *runTest(void *arg) {
    Distributor * blockDist;
    struct threadParam *tp = (struct threadParam*) arg;

    /* File distribution */
    for (int64_t i = 0; i < tp->fileNum; i++) {

        std::list<Disk*>* chosenDisks = tp->fileDist->placeExtent(0, i);
        if(tp->argc > 0){
            blockDist = Distributor::createDistributor(tp->blockDistType, tp->argc, tp->argv);
        }
        else {
            blockDist = Distributor::createDistributor(tp->blockDistType, 0, 0);
        }
        blockDist->setConfiguration(chosenDisks, 1, tp->numBlockCopies);

        for (list<Disk*>::iterator diskIterator = (*chosenDisks).begin(); diskIterator != (*chosenDisks).end(); ++diskIterator) {
            //cout << "Chosen disks: " << (*diskIterator)->getId() <<"\n";
            delete *diskIterator;
        }

        /* Block distribution */
        for (int64_t j = 0; j < tp->numBlocks; j++) {
            std::list<Disk*>* choosenDisksForBlocks = blockDist->placeExtent(0, j);

            for (list<Disk*>::iterator it = (*choosenDisksForBlocks).begin(); it != (*choosenDisksForBlocks).end(); ++it) {
                (*tp->results)[(*it)->getId()]++;
                delete *it;
            }
            delete choosenDisksForBlocks;
        }
        delete blockDist;
        delete chosenDisks;
    }
    return NULL;
}

int main(int argc, char** argv) {
    uint64_t numDisks = 100, numCopies = 3, numFiles = 10, numBlocks = 6, numThreads = 1, numBlockCopies = 1;
    int32_t fileDistType = Distributor::REDUNDANT_SHARE;
    int32_t blockDistType = Distributor::ROUND_ROBIN;
    list<Disk*>* disks;
    Distributor* blockDist;
    Distributor* fileDist;
    std::tr1::unordered_map<int64_t, int64_t> diskSizes, diskUsage;
    std::tr1::unordered_map<int32_t, int32_t> blkDistCreationTimes, blkDistPlaceExtTimes;
    struct timeval before, after, stepbefore, stepafter;
    char *outFileName = 0, *diskFileName = 0;
    int argInd;
    ostream *out = 0;
    ofstream *outf = 0;
    bool timedTests = false;
    mpz_class fd_dis;
    mpz_class bd_dis_create;
    mpz_class bd_dist_block;
    mpz_class bd_dist_blocks;
    mpz_class file_dist;
    mpz_class all_ct_sums = 0, all_ct_square_sums = 0, all_pt_sums = 0, all_pt_square_sums = 0;
    mpz_class fd_dis_sum = 0, fd_dis_square_sum = 0, bd_dis_create_sum = 0, bd_dis_create_square_sum = 0;
    mpz_class bd_dist_block_sum = 0, bd_dist_block_square_sum = 0, bd_dist_blocks_sum = 0, bd_dist_blocks_square_sum = 0;
    mpz_class file_dist_sum = 0, file_dist_square_sum = 0;
    mpz_class fd_time = 0;
    std::string fdcp = "", bdcp = "";
    vector<string> fdcp_v, bdcp_v;
    char **fdcp_argv, **bdcp_argv;
    bool fdcp_flag = false, bdcp_flag = false;


    /* parsing arguments */
    for (argInd = 1; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            usage();
            abort();
        }/* file distributor type */
        else if (strcmp(argv[argInd], "-fdt") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option fdt needs a value\n";
                usage();
                abort();
            }
            if (strcmp(argv[argInd], "RedundantShare") == 0) {
                fileDistType = Distributor::REDUNDANT_SHARE;
            } else if (strcmp(argv[argInd], "NearestNeighbour") == 0) {
                fileDistType = Distributor::NEAREST_NEIGHBOUR;
            } else if (strcmp(argv[argInd], "FastRedundantShare") == 0) {
                fileDistType = Distributor::REDUNDANT_SHARE_K;
            } else if (strcmp(argv[argInd], "Share") == 0) {
                fileDistType = Distributor::SHARE;
            } else if (strcmp(argv[argInd], "RoundRobin") == 0) {
                fileDistType = Distributor::ROUND_ROBIN;
            } else if (strcmp(argv[argInd], "RUSHr") == 0) {
                fileDistType = Distributor::RUSH_R;
            } else if (strcmp(argv[argInd], "RUSHt") == 0) {
                fileDistType = Distributor::RUSH_T;
            } else if (strcmp(argv[argInd], "RUSHp") == 0) {
                fileDistType = Distributor::RUSH_P;
            } else if (strcmp(argv[argInd], "CRUSH") == 0) {
                fileDistType = Distributor::CRUSH;
            } else if (strcmp(argv[argInd], "RandSlice") == 0) {
                fileDistType = Distributor::RAND_SLICE;
            } else {
                //Check for Integer Value
                cerr << "Unknown file distributor: " << argv[argInd] << "\n";
                usage();
                abort();
            }
        }/* block distributor type */
        else if (strcmp(argv[argInd], "-bdt") == 0) {
            argInd++;
            std::tr1::unordered_map<int64_t, int64_t> results;

            if (argInd == argc) {
                cerr << "Option bdt needs a value\n";
                usage();
                abort();
            }
            if (strcmp(argv[argInd], "RedundantShare") == 0) {
                blockDistType = Distributor::REDUNDANT_SHARE;
            } else if (strcmp(argv[argInd], "NearestNeighbour") == 0) {
                blockDistType = Distributor::NEAREST_NEIGHBOUR;
            } else if (strcmp(argv[argInd], "FastRedundantShare") == 0) {
                blockDistType = Distributor::REDUNDANT_SHARE_K;
            } else if (strcmp(argv[argInd], "Share") == 0) {
                blockDistType = Distributor::SHARE;
            } else if (strcmp(argv[argInd], "RoundRobin") == 0) {
                blockDistType = Distributor::ROUND_ROBIN;
            } else if (strcmp(argv[argInd], "RUSHr") == 0) {
                blockDistType = Distributor::RUSH_R;
            } else if (strcmp(argv[argInd], "RUSHt") == 0) {
                blockDistType = Distributor::RUSH_T;
            } else if (strcmp(argv[argInd], "RUSHp") == 0) {
                blockDistType = Distributor::RUSH_P;
            } else if (strcmp(argv[argInd], "CRUSH") == 0) {
                blockDistType = Distributor::CRUSH;
            } else if (strcmp(argv[argInd], "RandSlice") == 0) {
                blockDistType = Distributor::RAND_SLICE;
            } else {
                //Check for Integer Value
                cerr << "Unknown block distributor: " << argv[argInd] << "\n";
                usage();
                abort();
            }
        }/* number of threads */
        else if (strcmp(argv[argInd], "-Tn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option Tn needs a value\n";
                usage();
                abort();
            }
            numThreads = atoi(argv[argInd]);
            if (numThreads == 0) {
                cerr << "Value for Tn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for Tn is out of range\n";
                abort();
            }
        }/* number of files */
        else if (strcmp(argv[argInd], "-Fn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option Fn needs a value\n";
                usage();
                abort();
            }
            numFiles = atoi(argv[argInd]);
            if (numFiles == 0) {
                cerr << "Value for Fn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for Fn is out of range\n";
                abort();
            }
        }/* number of disks */
        else if (strcmp(argv[argInd], "-Dn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option Dn needs a value\n";
                usage();
                abort();
            }
            numDisks = atoi(argv[argInd]);
            if (numDisks == 0) {
                cerr << "Value for Dn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for Dn is out of range\n";
                abort();
            }
        } else if (strcmp(argv[argInd], "-Dif") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option Dif needs a value";
                usage();
                abort();
            }
            diskFileName = argv[argInd];
        }/* number of blocks */
        else if (strcmp(argv[argInd], "-Bn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option Bn needs a value\n";
                usage();
                abort();
            }
            numBlocks = atoi(argv[argInd]);
            if (numBlocks == 0) {
                cerr << "Value for Bn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for Bn is out of range\n";
                abort();
            }
        }/* number of block copies */
        else if (strcmp(argv[argInd], "-BCn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option BCn needs a value\n";
                usage();
                abort();
            }
            numBlockCopies = atoi(argv[argInd]);
            if (numBlockCopies == 0) {
                cerr << "Value for BCn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for BCn is out of range\n";
                abort();
            }
        }/* number of copies */
        else if (strcmp(argv[argInd], "-Cn") == 0) {
            argInd++;
            errno = 0;

            if (argInd == argc) {
                cerr << "Option Cn needs a value\n";
                usage();
                abort();
            }
            numCopies = atoi(argv[argInd]);
            if (numCopies == 0) {
                cerr << "Value for Cn is not a number\n";
                abort();
            } else if (errno == ERANGE) {
                cerr << "Value for Cn is out of range\n";
                abort();
            }
        }/* file name for output file */
        else if (strcmp(argv[argInd], "-of") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option of needs a value\n";
                usage();
                abort();
            }
            outFileName = argv[argInd];
        }/* Timed Tests */
        else if (strcmp(argv[argInd], "-tt") == 0) {
            timedTests = true;
        }/* parameters for file distributor */
        else if (strcmp(argv[argInd], "-fdcp") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option fdcp needs a value\n";
                usage();
                abort();
            }
            fdcp = argv[argInd];
            fdcp_flag = true;
        }/* parameters for block distributor */
        else if (strcmp(argv[argInd], "-bdcp") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option bdcp needs a value\n";
                usage();
                abort();
            }
            bdcp_flag = true;
            bdcp = argv[argInd];
        } else if (strcmp(argv[argInd], "-h") == 0) {
            usage();
            exit(0);
        }
    }/* parsing arguments */

    /* Arguments for file distributor */
    if (fdcp != "") {
        string tmp = "";
        for (std::string::iterator it(fdcp.begin()); it != fdcp.end(); ++it) {
            if (*it == ' ') {
                fdcp_v.push_back(tmp);
                tmp = "";
            } else {
                tmp += *it;
            }
        }
        fdcp_v.push_back(tmp);
    }

    /* Arguments for block distributor */
    if (bdcp != "") {
        string tmp = "";
        for (std::string::iterator it(bdcp.begin()); it != bdcp.end(); ++it) {
            if (*it == ' ') {
                bdcp_v.push_back(tmp);
                tmp = "";
            } else {
                tmp += *it;
            }
        }
        bdcp_v.push_back(tmp);
    }

    fdcp_argv = (char**) new void*[fdcp_v.size()];
    uint j = 0;
    cout << "Parameters for file distributor\n";
    for (std::vector<string>::iterator i = fdcp_v.begin(); i != fdcp_v.end(); ++i) {
        cout << *i << " ";
        fdcp_argv[j] = new char[(*i).length() + 1];
        strcpy(fdcp_argv[j], i->c_str());
        j++;
    }
    cout << "\n";

    bdcp_argv = (char**) new void*[bdcp_v.size()];
    j = 0;
    cout << "Parameters for block distributor\n";
    for (std::vector<string>::iterator i = bdcp_v.begin(); i != bdcp_v.end(); ++i) {
        cout << *i << " ";
        bdcp_argv[j] = new char[(*i).length() + 1];
        strcpy(bdcp_argv[j], i->c_str());
        j++;
    }
    cout << "\n";

    if (diskFileName == 0) {
        cout << "Creating Disks\n";
        disks = new list<Disk*>();
        Disk* aDisk;
        for (uint64_t i = 0; i < numDisks; i++) {
            aDisk = new Disk(i, 10000, 0);
            disks->push_back(aDisk);
            if (aDisk == 0) {
                cout << "disk was null\n";
            }
            diskSizes[i] = aDisk->getCapacity();
        }
    } else {
        disks = Disk::loadDiskList(string(diskFileName));
        numDisks = disks->size();
        for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
            diskSizes[(*it)->getId()] = (*it)->getCapacity();
        }
        cout << "Loaded " << numDisks << " Disks from File\n";
    }


    /*for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
            Disk* d = *it;
            cout << "Disk: " << d->getId() << "\n";
            cout << "  Capacity: " << d->getCapacity() << "\n";
    }*/

    gettimeofday(&before, 0);
    if (!fdcp_flag) {
        fileDist = Distributor::createDistributor(fileDistType, 0, 0);
    } else {
        fileDist = Distributor::createDistributor(fileDistType, fdcp_v.size(), fdcp_argv);
    }

    fileDist->setConfiguration(disks, 1, numCopies);
    gettimeofday(&after, 0);

    fd_time = timeInNS(before, after);

    if (numThreads == 1) {


        gettimeofday(&before, 0);

        /* File distribution */
        for (uint64_t i = 0; i < numFiles; i++) {

            gettimeofday(&stepbefore, 0);
            std::list<Disk*>* chosenDisks = fileDist->placeExtent(0, i);
            // WARNING: I make homogeneous disk, so the filedistributor has 
            // to care about the heterogeneous disks, while the block
            // distributor only has to handle homogeneous ones.
            for (list<Disk*>::iterator diskIterator = (*chosenDisks).begin(); diskIterator != (*chosenDisks).end(); ++diskIterator) {
                (*diskIterator)->setCapacity(1);
            }
            gettimeofday(&stepafter, 0);
            fd_dis = timeInNS(stepbefore, stepafter);
            fd_dis_sum += fd_dis;
            fd_dis_square_sum += fd_dis * fd_dis;

            if (!bdcp_flag) {
                gettimeofday(&stepbefore, 0);
                blockDist = Distributor::createDistributor(blockDistType, bdcp_v.size(), bdcp_argv);
                gettimeofday(&stepafter, 0);
            } else {
                gettimeofday(&stepbefore, 0);
                blockDist = Distributor::createDistributor(blockDistType, 0, 0);
                gettimeofday(&stepafter, 0);
            }
            blockDist->setConfiguration(chosenDisks, 1, numBlockCopies);
            //cout << "File: " << i << "\n";
            for (list<Disk*>::iterator diskIterator = (*chosenDisks).begin(); diskIterator != (*chosenDisks).end(); ++diskIterator) {
                //cout << "Chosen disks: " << (*diskIterator)->getId() << "\n";
                delete *diskIterator;
            }
            bd_dis_create = timeInNS(stepbefore, stepafter);
            bd_dis_create_sum += bd_dis_create;
            bd_dis_create_square_sum += bd_dis_create * bd_dis_create;

            bd_dist_blocks = 0;
            for (uint64_t j = 0; j < numBlocks; j++) {
                gettimeofday(&stepbefore, 0);
                std::list<Disk*>* choosenDisksForBlocks = blockDist->placeExtent(0, j);
                gettimeofday(&stepafter, 0);
                bd_dist_block = timeInNS(stepbefore, stepafter);
                bd_dist_blocks += bd_dist_block;
                bd_dist_block_sum += bd_dist_block;
                bd_dist_block_square_sum += bd_dist_block * bd_dist_block;

                //cout << "	Block: " << j << "\n";
                for (list<Disk*>::iterator it = (*choosenDisksForBlocks).begin(); it != (*choosenDisksForBlocks).end(); ++it) {
                    diskUsage[(*it)->getId()]++;
                    //cout << "	Chosen disk: " << (*it)->getId() << "\n";
                    delete *it;
                }
                delete choosenDisksForBlocks;
            }
            bd_dist_blocks_sum += bd_dist_blocks;
            bd_dist_blocks_square_sum += bd_dist_blocks * bd_dist_blocks;
            file_dist = fd_dis + bd_dis_create + bd_dist_blocks;
            file_dist_sum += file_dist;
            file_dist_square_sum += file_dist * file_dist;
            delete blockDist;
            delete chosenDisks;
        }
        gettimeofday(&after, 0);
    }

    if (numThreads > 1) {
        uint64_t filesPerThread = numFiles / numThreads;
        uint64_t resFiles = numFiles % numThreads;
        pthread_t threads[numThreads];
        struct threadParam tp[numThreads];

        std::tr1::unordered_map<int64_t, int64_t> results[numThreads];

        fileDist->setNumThreads((uint16_t) numThreads);

        gettimeofday(&before, 0);

        for (uint64_t t = 0; t < numThreads; t++) {

            tp[t].fileNum = filesPerThread;
            tp[t].threadId = t;
            tp[t].fileDist = fileDist;
            tp[t].blockDistType = blockDistType;
            tp[t].numBlocks = numBlocks;
            tp[t].results = &results[t];
            tp[t].numCopies = numCopies;
            tp[t].numBlockCopies = numBlockCopies;
            if (bdcp_flag) {
                tp[t].argv = bdcp_argv;
                tp[t].argc = bdcp_v.size();
            } else {
                tp[t].argc = 0;
            }

            if (t < resFiles) {
                tp[t].fileNum++;
            }
            if (timedTests) {
                if (pthread_create(&threads[t], 0, &runTimedTest, &tp[t])) {
                    cout << "failed to create thread\n";
                }
            } else {
                if (pthread_create(&threads[t], 0, &runTest, &tp[t])) {
                    cout << "failed to create thread\n";
                }
            }
        }

        for (uint64_t t = 0; t < numThreads; t++) {
            pthread_join(threads[t], NULL);
        }
        gettimeofday(&after, 0);

        /* Summarize results */
        for (uint64_t rm = 0; rm < numThreads; rm++) {
            //cout << "thread: " << rm << "\n";
            std::tr1::unordered_map<int64_t, int64_t>::iterator it;
            for (it = results[rm].begin(); it != results[rm].end(); ++it) {
                diskUsage[it->first] += it->second;
                //cout << it->first << ";" << it->second << "\n";
            }

            all_ct_sums += sum_creationTimes[rm];
            all_ct_square_sums += sum_square_creationTimes[rm];
            all_pt_sums += sum_placeTimes[rm];
            all_pt_square_sums += sum_square_placeTimes[rm];

        }

    }

    cout << "Timing Informations:\n";
    cout << "Get Disk from File Distributor time sum:" << fd_dis_sum.get_str(10) << "\n";
    cout << "Get Disk from File Distributor square time sum:" << fd_dis_square_sum.get_str(10) << "\n";
    cout << "Create Block Distributor time sum:" << bd_dis_create_sum.get_str(10) << "\n";
    cout << "Create Block Distributor square time sum:" << bd_dis_create_square_sum.get_str(10) << "\n";
    cout << "Place single Block time sum:" << bd_dist_block_sum.get_str(10) << "\n";
    cout << "Place single Block square time sum:" << bd_dist_block_square_sum.get_str(10) << "\n";
    cout << "Place file blocks time sum:" << bd_dist_blocks_sum.get_str(10) << "\n";
    cout << "Place file blocks square time sum:" << bd_dist_blocks_square_sum.get_str(10) << "\n";
    cout << "Complete File time sum:" << file_dist_sum.get_str(10) << "\n";
    cout << "Complete File square time sum:" << file_dist_square_sum.get_str(10) << "\n";
    /*cout << "File distributor creation time sum:" << fd_time.get_str(10) << "\n";
    cout << "Block distributor creation times sum:" << all_ct_sums.get_str(10) << "\n";
    cout << "Block distributor creation times square sum:" << all_ct_square_sums.get_str(10) << "\n";
    cout << "Block distributor place times sum:" << all_pt_sums.get_str(10) << "\n";
    cout << "Block distributor creation times square sum:" << all_pt_square_sums.get_str(10) << "\n";*/
    cout << "Used Time [ms]:" << timeInMS(before, after) << "\n";


    if (outFileName == 0) {
        out = &cout;
        cout << "Disk usage:\n";
    } else {
        outf = new ofstream();
        outf->open(outFileName);
        out = outf;
    }

    for (uint64_t i = 0; i < numDisks; i++) {
        (*out) << i << ";" << diskUsage[i] << ";" << diskSizes[i] << "\n";
    }

    if (outFileName != 0) {
        outf->close();
        delete outf;
    }

    deleteList(disks);
    delete disks;
    delete fileDist;
    for (j = 0; j < fdcp_v.size(); j++) {
        delete []fdcp_argv[j];
    }
    delete []fdcp_argv;
    for (j = 0; j < bdcp_v.size(); j++) {
        delete []bdcp_argv[j];
    }
    delete []bdcp_argv;
    return (EXIT_SUCCESS);
}
