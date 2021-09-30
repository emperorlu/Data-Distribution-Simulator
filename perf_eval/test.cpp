/* 
 * File:   test.cpp
 * Author: fermat
 *
 * Created on 20. Januar 2010, 14:41
 */
//#define no_gcrypt
//#define no_sqlite

#include <stdlib.h>
#include <stdio.h>
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
#include <map>
#include <tr1/unordered_map>
#include <pthread.h>
#include "test.h"
#include "test_private.h"
#include <gmpxx.h>
#ifndef no_gcrypt
#include <gcrypt.h>
#endif

// TODO: Add param to set the used Hash Algorithm and give a list of (some) known Algorithms.

using namespace VDRIVE;
using namespace std;

static VDRIVE::Distributor *distThread = 0;
static std::list<VDRIVE::Disk*> *disksThread = 0;
static std::tr1::unordered_map<int32_t, std::tr1::unordered_map<int64_t, uint64_t>* > *results = 0;
static std::tr1::unordered_map<int32_t, mpz_class> sum_times;
static std::tr1::unordered_map<int32_t, mpz_class> sum_square_times;

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
            "    cppdistributorstester [options]\n" <<
            "This Programm runs some Distribution test and exits\n" <<
            "Options:\n" <<
            "  -dt <distType>   use given Distributor\n" <<
            "  -dif <distFile>  read a saved Distributor from file\n" <<
            "  -dof <distFile>  write Distributor to given File\n" <<
            "  -Dn N            number of Disks, that shall be used\n" <<
            "  -Dhet            make heterogeneous Disks (default are homogeneous)\n" <<
            "  -Dif <diskFile>  read Disks from from File\n" <<
#ifndef no_sqlite
            "  -DiSQLite        disk file to read is an SQLite DB (not XML)\n" <<
#endif
            "  -Dof <diskFile>  write Disks to File (Only configuration)\n" <<
#ifndef no_sqlite
            "  -DoSQLite        disk file to write is an SQLite DB (not XML)\n" <<
#endif
            "  -Dp              print Disks after creation/loading to stdout\n" <<
            "  -kn N            number of copies to be distributed\n" <<
            "  -of <outFile>    write result to given File\n" <<
            "  -en N            distribute given number of extents\n" <<
            "  -fen N           number of first Extent\n"
            "  -Tn N            Number of Threads distributing the Extents\n" <<
            "  -Tof <outFile>   Send disk usage per Thread to given file (debug)\n" <<
            "  -bm baseMessage  Set the baseMessage to be used for Hashing\n" <<
            "  -Mof <outFile>   cat /proc/PID/status to the given file after creating the\n" <<
            "                   distributor and before placing extents.\n" <<
            "  -st              measure time to place single extents.\n" <<
#ifndef no_gcrypt
            "  -icrypt          use the given internal Hash algorithm.\n" <<
            "                   * SHA1: " << VDRIVE::Distributor::HASH_SHA1 << "\n" <<
            "                   * LC: " << VDRIVE::Distributor::HASH_XOR << "\n" <<
            "                   * Rand: " << VDRIVE::Distributor::HASH_RAND << "\n" <<
            "                   * MT: " << VDRIVE::Distributor::HASH_MT << "\n" <<
            "  -gcrypt N        use the given gcrypt hash algorithm instead of build in SHA1\n" <<
            "                   GCrypt knows (at least) the following Hash algorithms:\n" <<
            "                   * SHA1: " << GCRY_MD_SHA1 << "\n" <<
            "                   * SHA224: " << GCRY_MD_SHA224 << "\n" <<
            "                   * SHA256: " << GCRY_MD_SHA256 << "\n" <<
            "                   * SHA284: " << GCRY_MD_SHA384 << "\n" <<
            "                   * SHA512: " << GCRY_MD_SHA512 << "\n" <<
            "                   * MD4: " << GCRY_MD_MD4 << "\n" <<
            "                   * MD5: " << GCRY_MD_MD5 << "\n" <<
            "                   * TIGER: " << GCRY_MD_TIGER << "\n" <<
            "                   * WHIRLPOOL: " << GCRY_MD_WHIRLPOOL << "\n" <<
#endif
            "  -dp              An Parameters following after this param will be given to\n" <<
            "                   the Distributor\n" <<
            "\n" <<
            "I know the following distTypes and special params (after -dp):\n" <<
            "  * FastRedundantShare\n" <<
            "  * RedundantShare\n" <<
            "    -decID         Even sized disks will be sorted by decreasing id (else by increasing)\n" <<
            "  * Share\n" <<
            "    -sf N          Constant for the Stretchfactor used for each Disk\n" <<
            "    -ssf N         Use exactly this Stretchfactor\n" <<
            "    -sc N          Number of copies of each disk to be placed in Share\n" <<
            "    -nf N          delegation to each NearestNeighbour\n" <<
            "    -nc N          delegation to each NearestNeighbour\n" <<
            "    -cnn           copies are created by NearestNeighbour, not by Share\n" <<
            "  * NearestNeighbour\n" <<
            "    -nf N          Number of copies of each disk to be placed\n" <<
            "                   (N*log(#Disks) will be placed)\n" <<
            "    -nc N          Use exactly this number of copies\n" <<
            "    -cut N         Make for each N Blocks of each Disk a virtual Disk\n" <<
            "                   (more copies of the Disk are placed.)\n" <<
            "  * RoundRobin\n" <<
            "    -hash          hash all Positions before placing\n" <<
            "    -im            use an ImprovedMap to place Elements\n" <<
            "  * RUSHp, RUSHr, RUSHt\n" <<
            "    -copies        a number of replicas\n" <<
            "  * CRUSH\n" <<
            "    -copies        a number of replicas\n" <<
            "    -layer         define a crush map. Each layer is 'name buckettype'\n" <<
            "                   (e.g. default map: '-layer domain 1 root 4')" <<
            "                   * UNIFORM : 1\n" <<
            "                   * LIST: 2\n" <<
            "                   * TREE: 3\n" <<
            "                   * STRAW: 4\n" <<
            "    -ndom          a size of failure domains, default value is 10\n" <<
            "  * RandSlice\n";
}

void *runTest(void *arg) {
    struct runTestParms *parms = (struct runTestParms *) arg;
    int64_t lastID = parms->firstID + parms->numExtents;
    std::tr1::unordered_map<int64_t, uint64_t> *map = (*results)[parms->threadID];
    
    for (int64_t i = parms->firstID; i < lastID; i++) {
        std::list<Disk*>* choosenDisks = distThread->placeExtent(0, i);
        for (list<Disk*>::iterator it = (*choosenDisks).begin(); it != (*choosenDisks).end(); ++it) {
            Disk* disk = *it;
            (*map)[disk->getId()]++;
            delete disk;
        }
        delete choosenDisks;
    }
    return arg;
}

void *runTimedTest(void *arg) {
    struct runTestParms *parms = (struct runTestParms *) arg;
    int64_t lastID = parms->firstID + parms->numExtents;
    std::tr1::unordered_map<int64_t, uint64_t> *map = (*results)[parms->threadID];
    struct timeval before, after;
    mpz_class sums = 0, square_sums = 0, step;

    for (int64_t i = parms->firstID; i < lastID; i++) {
        gettimeofday(&before, 0);
        std::list<Disk*>* choosenDisks = distThread->placeExtent(0, i);
        gettimeofday(&after, 0);
        step = timeInNS(before, after);
        sums += step;
        step *= step;
        square_sums += step;
        for (list<Disk*>::iterator it = (*choosenDisks).begin(); it != (*choosenDisks).end(); ++it) {
            Disk* disk = *it;
            (*map)[disk->getId()]++;
            delete disk;
        }
        delete choosenDisks;
    }
    sum_times[parms->threadID] = sums;
    sum_square_times[parms->threadID] = square_sums;
    return arg;
}

static void copyMemdata(char* memFileName) {
    if (memFileName != 0) {
        try {
            char buff[2048];
            pid_t pid = getpid();
            std::stringstream pidFile;
            pidFile << "/proc/" << pid << "/status";
            int readBytes = 1;

            ifstream inFileStream(pidFile.str().c_str(), ios::in | ios::binary);
            if (!inFileStream) {
                throw string("Could not open pidfile " + pidFile.str());
            }

            ifstream tmpStream(memFileName);
            if (tmpStream) {
                inFileStream.close();
                throw string("outFile for memory usage already exists");
            }
            tmpStream.close();

            ofstream outFileStream(memFileName, ios::out | ios::binary);
            if (!outFileStream) {
                inFileStream.close();
                throw string("Could not open outFile for memory usage");
            }

            while (readBytes != 0) {
                inFileStream.read((char*) buff, 2048);
                readBytes = inFileStream.gcount();
                outFileStream.write((char*) buff, readBytes);
            }
            inFileStream.close();
            outFileStream.close();
        } catch (std::string s) {
            cerr << "Error wile copying pidfile:\n    " << s << "\n";
        }
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    int32_t numDisks = 10, numCopies = 3;
    int32_t distType = Distributor::REDUNDANT_SHARE;
    char *memFileName = 0, *distFileName = 0, *distOutFileName = 0, *outFileName = 0, *threadOutFileName = 0, *diskFileName = 0, *diskOutFileName = 0;
    int64_t numExtends = 0, firstExtentNumber = 0;
    mpz_class all_sum_times = 0, all_sum_square_times = 0, stepTime = 0;
    bool printDisks = false, singleTimes = false;
#ifndef no_sqlite
    bool diSQLite = false, doSQLite = false;
#endif

#ifndef no_gcrypt
    int gcryptAlgorithm = 0;
    bool internalHash = true;
    int icryptAlgorithm = VDRIVE::Distributor::HASH_SHA1;
#endif

    struct timeval before, after, stepbefore, stepafter;
    list<Disk*>* disks;
    Distributor* dist;
    Disk* disk;
    ostream *out = 0;
    ofstream *outf = 0;
    int argInd;
    int32_t numThreads = 1;
    std::tr1::unordered_map<int64_t, int64_t> diskSizes, diskUsage;
    uint8_t *baseMessage = 0;

    bool heterogeneous = false;

    cout << "CPPDistTest called with following params: ";
    for (argInd = 1; argInd < argc; argInd++) {
        cout << argv[argInd] << " ";
    }
    cout << "\n";

    for (argInd = 1; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            usage();
            abort();
        } else if (strcmp(argv[argInd], "-dt") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option dt needs a value";
                usage();
                abort();
            }
            if (strcmp(argv[argInd], "RedundantShare") == 0) {
                distType = Distributor::REDUNDANT_SHARE;
            } else if (strcmp(argv[argInd], "NearestNeighbour") == 0) {
                distType = Distributor::NEAREST_NEIGHBOUR;
            } else if (strcmp(argv[argInd], "FastRedundantShare") == 0) {
                distType = Distributor::REDUNDANT_SHARE_K;
            } else if (strcmp(argv[argInd], "Share") == 0) {
                distType = Distributor::SHARE;
            } else if (strcmp(argv[argInd], "RoundRobin") == 0) {
                distType = Distributor::ROUND_ROBIN;
            } else if (strcmp(argv[argInd], "RUSHr") == 0) {
                distType = Distributor::RUSH_R;
            } else if (strcmp(argv[argInd], "RUSHt") == 0) {
                distType = Distributor::RUSH_T;
            } else if (strcmp(argv[argInd], "RUSHp") == 0) {
                distType = Distributor::RUSH_P;
            } else if (strcmp(argv[argInd], "CRUSH") == 0) {
                distType = Distributor::CRUSH;
            } else if (strcmp(argv[argInd], "RandSlice") == 0) {
                distType = Distributor::RAND_SLICE;
            } else {
                //Check for Integer Value
                cerr << "Unknown distributor: " << argv[argInd] << "\n";
                usage();
                abort();
            }
        } else if (strcmp(argv[argInd], "-Mof") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option Mof needs a value";
                usage();
                abort();
            }
            memFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-dif") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option dif needs a value";
                usage();
                abort();
            }
            distFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-bm") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option bm needs a value";
                usage();
                abort();
            }
            baseMessage = new uint8_t[64];
            size_t length = strlen(argv[argInd]);
            for (int i = 0; i < 64; i++) {
                baseMessage[i] = argv[argInd][i % length];
            }
        } else if (strcmp(argv[argInd], "-Dif") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option Dif needs a value";
                usage();
                abort();
            }
            diskFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-Dof") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option Dof needs a value";
                usage();
                abort();
            }
            diskOutFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-dof") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option dof needs a value";
                usage();
                abort();
            }
            distOutFileName = argv[argInd];
#ifndef no_gcrypt
        } else if (strcmp(argv[argInd], "-gcrypt") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option gcrypt needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                cout << "-grypt has to be followed by a number\n";
                usage();
                abort();
            }

            if (val > 2147483647) {
                cout << "Maximum number of Disks: %d\n" << 2147483647;
                usage();
                abort();
            }
            internalHash = false;
            gcryptAlgorithm = (int32_t) val;
        } else if (strcmp(argv[argInd], "-icrypt") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option icrypt needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                cout << "-irypt has to be followed by a number\n";
                usage();
                abort();
            }

            if (val > 2147483647) {
                cout << "Maximum number of Disks: %d\n" << 2147483647;
                usage();
                abort();
            }
            internalHash = false;
            icryptAlgorithm = (int32_t) val;
#endif
        } else if (strcmp(argv[argInd], "-Dn") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option Dn needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                cout << "-Dn has to be followed by a number\n";
                usage();
                abort();
            }
            if (val < 0) {
                cout << "Negative number of Disks?!?\n";
                usage();
                abort();
            }
            if (val > 2147483647) {
                cout << "Maximum number of Disks: %d\n" << 2147483647;
                usage();
                abort();
            }
            numDisks = (int32_t) val;
        } else if (strcmp(argv[argInd], "-Tn") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option Tn needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                cout << "-Tn has to be followed by a number\n";
                usage();
                abort();
            }
            if (val < 0) {
                cout << "Negative number of Threads?!?\n";
                usage();
                abort();
            }
            if (val > 2147483647) {
                cout << "Maximum number of Threads: %d\n" << 2147483647;
                usage();
                abort();
            }
            numThreads = (int32_t) val;
        } else if (strcmp(argv[argInd], "-Dp") == 0) {
            printDisks = true;
        } else if (strcmp(argv[argInd], "-st") == 0) {
            singleTimes = true;
        } else if (strcmp(argv[argInd], "-Dhet") == 0) {
            heterogeneous = true;
#ifndef no_sqlite
        } else if (strcmp(argv[argInd], "-DiSQLite") == 0) {
            diSQLite = true;
        } else if (strcmp(argv[argInd], "-DoSQLite") == 0) {
            doSQLite = true;
#endif
        } else if (strcmp(argv[argInd], "-kn") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option kn needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                cout << "-kn has to be followed by a number\n";
                usage();
                abort();
            }
            if (val < 0) {
                cout << "Negative number of kopies?!?\n";
                usage();
                abort();
            }
            if (val > 2147483647) {
                printf("Maximum number of kopies: %d\n", 2147483647);
                usage();
                abort();
            }
            numCopies = (int32_t) val;
        } else if (strcmp(argv[argInd], "-of") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option of needs a value";
                usage();
                abort();
            }
            outFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-Tof") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option Tof needs a value";
                usage();
                abort();
            }
            threadOutFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-dp") == 0) {
            argInd++;
            break;
        } else if (strcmp(argv[argInd], "-en") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option en needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                printf("-en has to be followed by a number\n");
                usage();
                abort();
            }
            if (val < 0) {
                printf("Negative number of extents?!?\n");
                usage();
                abort();
            }
            numExtends = val;
        } else if (strcmp(argv[argInd], "-fen") == 0) {
            char* end;
            long val;
            argInd++;
            if (argInd == argc) {
                cerr << "Option fen needs a value";
                usage();
                abort();
            }
            val = strtol(argv[argInd], &end, 0);
            if (end == argv[argInd]) {
                printf("-fen has to be followed by a number\n");
                usage();
                abort();
            }
            if (val < 0) {
                printf("Negative first Extent Number?!?\n");
                usage();
                abort();
            }
            firstExtentNumber = val;
        } else {
            cout << "unknown option: " << argv[argInd] << "\n";
            usage();
            exit(-1);
        }
    }




    if (distFileName == 0) {
        if (diskFileName != 0) {
            cout << "Loading Disks from File\n";
#ifndef no_sqlite
            if (diSQLite) {
                disks = Disk::loadDiskListDBFile(string(diskFileName));
            } else {
#endif
                disks = Disk::loadDiskList(string(diskFileName));
#ifndef no_sqlite
            }
#endif

            numDisks = disks->size();
            for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
                diskSizes[(*it)->getId()] = (*it)->getCapacity();
            }
            cout << "Loaded " << numDisks << " Disks from File\n";
        } else {
            cout << "Creating Disks\n";
            disks = new list<Disk*>();
            Disk* aDisk;
            for (int64_t i = 0; i < numDisks; i++) {
                if (heterogeneous) {
                    aDisk = new Disk(i, 10000 + (i * 1000), 0);
                } else {
                    aDisk = new Disk(i, 10000, 0);
                }
                disks->push_back(aDisk);
                if (aDisk == 0) {
                    cout << "disk was null\n";
                }
                diskSizes[i] = aDisk->getCapacity();
            }
            disk = 0;
        }

        if (printDisks) {
            for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
                Disk* d = *it;
                cout << "Disk: " << d->getId() << "\n";
                cout << "  Capacity: " << d->getCapacity() << "\n";
            }
        }

        if (diskOutFileName != 0) {
#ifndef no_sqlite
            if (doSQLite) {
                Disk::storeDiskListDBFile(disks, string(diskOutFileName));
            } else {
#endif
                Disk::storeDiskList(disks, string(diskOutFileName));
#ifndef no_sqlite
            }
#endif
        }

        cout << "Creating new Distributor\n";
        if (argInd == argc) {
            dist = Distributor::createDistributor(distType, 0, 0);
        } else {
            dist = Distributor::createDistributor(distType, argc - argInd, argv + argInd);
        }
        if (baseMessage != 0)
            dist->setBaseMessage(baseMessage);
#ifndef no_gcrypt
        dist->useInternalHashAlgorithm(icryptAlgorithm);
        if (!internalHash)
            dist->useGCryptHashAlgorithm(gcryptAlgorithm);
#endif
        if (numThreads > 1)
            dist->setNumThreads((uint16_t) numThreads);
        cout << "Will set configuration,\n";
        gettimeofday(&before, 0);
        dist->setConfiguration(disks, 1, numCopies);
        gettimeofday(&after, 0);
        copyMemdata(memFileName);
        cout << "Used Time for Initialization:" << timeInMS(before, after) << "\n";
    } else {
        cout << "Loading Distributor from file\n";
        gettimeofday(&before, 0);
        dist = Distributor::loadDistributor(string(distFileName));
        gettimeofday(&after, 0);
        cout << "Used Time for Loading distributor: " << timeInMS(before, after) << "\n";
        copyMemdata(memFileName);
        disks = dist->getDisks();
        numDisks = disks->size();
        numCopies = disks->size();
        for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
            Disk* myDisk = *it;
            diskSizes[myDisk->getId()] = myDisk->getCapacity();
        }
        if (baseMessage != 0)
            dist->setBaseMessage(baseMessage);
    }

    if (baseMessage != 0)
        delete[] baseMessage;

    if (distOutFileName != 0) {
        dist->save(string(distOutFileName));
    }

    for (int32_t i = 0; i < numDisks; i++) {
        diskUsage[i] = 0;
    }

    if (numThreads == 1) {
        cout << "Will Place Extents without pThreads\n";
        gettimeofday(&before, 0);
        if (singleTimes) {
            uint64_t lastExtent = firstExtentNumber + numExtends;
            for (int64_t i = firstExtentNumber; i < lastExtent; i++) {
                gettimeofday(&stepbefore, 0);
                std::list<Disk*>* choosenDisks = dist->placeExtent(0, i);
                gettimeofday(&stepafter, 0);
                stepTime = timeInNS(stepbefore, stepafter);
                all_sum_times += stepTime;
                stepTime *= stepTime;
                all_sum_square_times += stepTime;
                for (list<Disk*>::iterator it = (*choosenDisks).begin(); it != (*choosenDisks).end(); ++it) {
                    Disk* disk = *it;
                    diskUsage[disk->getId()]++;
                    delete disk;
                }
                delete choosenDisks;
            }
        } else {
            uint64_t lastExtent = firstExtentNumber + numExtends;
            for (int64_t i = firstExtentNumber; i < lastExtent; i++) {
                std::list<Disk*>* choosenDisks = dist->placeExtent(0, i);
                for (list<Disk*>::iterator it = (*choosenDisks).begin(); it != (*choosenDisks).end(); ++it) {
                    Disk* disk = *it;
                    diskUsage[disk->getId()]++;
                    delete disk;
                }
                delete choosenDisks;
            }
        }
        gettimeofday(&after, 0);
        cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";
    } else {
        struct runTestParms parms[numThreads];
        int64_t extentsPerThread = numExtends / numThreads;
        int32_t restExtents = numExtends % numThreads;
        int64_t nextID = 0;
        pthread_t threads[numThreads];
        int errorCode = 0;

        results = new std::tr1::unordered_map<int32_t, std::tr1::unordered_map<int64_t, uint64_t>* > ();
        for (int32_t i = 0; i < numThreads; i++) {
            parms[i].firstID = firstExtentNumber + nextID;
            parms[i].threadID = i;
            if (i < restExtents) {
                parms[i].numExtents = extentsPerThread + 1;
            } else {
                parms[i].numExtents = extentsPerThread;
            }
            nextID += parms[i].numExtents;
            std::tr1::unordered_map<int64_t, uint64_t> *resMap = new std::tr1::unordered_map<int64_t, uint64_t > ();
            for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
                (*resMap)[(*it)->getId()] = 0;
            }
            (*results)[i] = resMap;
            sum_times[i] = 0;
            sum_square_times[i] = 0;
        }

        disksThread = disks;
        distThread = dist;

        //Run the Tests
        gettimeofday(&before, 0);
        if (singleTimes) {
            for (int32_t i = 0; i < numThreads; i++) {
                if ((errorCode = pthread_create(&threads[i], 0, runTimedTest, &parms[i]))) {
                    cerr << "Error " << errorCode << " while trying to start thread " << i << "\n";
                    abort();
                }
            }
        } else {
            for (int32_t i = 0; i < numThreads; i++) {
                if ((errorCode = pthread_create(&threads[i], 0, runTest, &parms[i]))) {
                    cerr << "Error " << errorCode << " while trying to start thread " << i << "\n";
                    abort();
                }
            }
        }

        // Wait for all Threads to be done
        for (int32_t i = 0; i < numThreads; i++) {
            void *threadRes = 0;
            if ((errorCode = pthread_join(threads[i], &threadRes))) {
                cerr << "Error " << errorCode << " joining Thread " << i << "\n";
                abort();
            }
        }
        gettimeofday(&after, 0);
        cout << "Used Time for Placing Extents:" << timeInMS(before, after) << "\n";

        if (singleTimes) {
            for (int i = 0; i < numThreads; i++) {
                all_sum_times += sum_times[i];
                all_sum_square_times += sum_square_times[i];
            }
        }

        if (threadOutFileName != 0) {
            outf = new ofstream();
            outf->open(threadOutFileName);
            out = outf;
        }

        for (std::tr1::unordered_map<int32_t, std::tr1::unordered_map<int64_t, uint64_t>* >::iterator it =
                results->begin(); it != results->end(); it++) {
            std::tr1::unordered_map<int64_t, uint64_t> *resMap = it->second;
            for (std::tr1::unordered_map<int64_t, uint64_t>::iterator it2 = resMap->begin();
                    it2 != resMap->end(); it2++) {
                diskUsage[it2->first] += it2->second;
                //cout << it->first << ";" << it2->first << ";" << it2->second << "\n";
                if (threadOutFileName != 0) {
                    (*out) << it->first << ";" << it2->first << ";" << it2->second << "\n";
                }
            }
            delete resMap;
        }
        delete results;

        if (threadOutFileName != 0) {
            outf->close();
            delete outf;
            outf = 0;
        }
        disksThread = 0;
        distThread = 0;
    }

    cout << "Placed Extents:" << numExtends << "\n";
    cout << "Placed copies of each Extent:" << numCopies << "\n";
    if (singleTimes) {
        cout << "Sum time of placing single Extents:" << all_sum_times.get_str(10) << "\n";
        cout << "Sum of squares of placing single Extents:" << all_sum_square_times.get_str(10) << "\n";
        if (numExtends > 0) {
            mp_exp_t ext;
            mpz_class tmp1 = (create_mpz_from_uint64(numExtends) * all_sum_square_times) - (all_sum_times * all_sum_times);
            mpf_class varianz = mpf_class(tmp1) / create_mpz_from_uint64(numExtends * (numExtends - 1));
#ifdef my_gmp_ref
            cout << "Varianz of time placing Extents:" << varianz.get_str(ext, 10, 0) << "\n";
#else
            cout << "Varianz of time placing Extents:" << varianz.get_str(&ext, 10, 0) << "\n";
#endif
            cout << "Varianz of time placing Extents Exponend:" << ext << "\n";
            mpf_class deviation = sqrt(varianz);
#ifdef my_gmp_ref
            cout << "Standard Deviation placing Extents:" << deviation.get_str(ext, 10, 0) << "\n";
#else
            cout << "Standard Deviation placing Extents:" << deviation.get_str(&ext, 10, 0) << "\n";
#endif
            cout << "Standard Deviation placing Extents Exponend:" << ext << "\n";
        }
    }
    cout << "Used Threads to place Extents:" << numThreads << "\n";

    if (outFileName == 0) {
        out = &cout;
        cout << "Disk usage:\n";
    } else {
        outf = new ofstream();
        outf->open(outFileName);
        out = outf;
    }
    for (int32_t i = 0; i < numDisks; i++) {
        (*out) << i << ";" << diskUsage[i] << ";" << diskSizes[i] << "\n";
    }
    
    ofstream file;
    file.open("consistent.csv");
    if (file)
    {
        for (int32_t i = 0; i < numDisks; i++) {
            file << diskUsage[i] << "\n";
        }
    }
    file.close();

    if (outFileName != 0) {
        outf->close();
        delete outf;
    }
    for (list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        delete *it;
    }
    delete disks;
    delete dist;
    return (EXIT_SUCCESS);
}
