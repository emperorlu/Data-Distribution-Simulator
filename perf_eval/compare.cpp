/* 
 * File:   compare.cpp
 * Author: fermat
 *
 * Created on 31. März 2010, 15:17
 */

#include <stdlib.h>
#include <stdio.h>
#include <dadisi/Distributor.h>
#include <sys/time.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <limits.h>
#include <stdint.h>
#include <sstream>
#include <map>
#include <tr1/unordered_map>
#include <pthread.h>
#include <set>

#include "compare.h"

using namespace std;
using namespace VDRIVE;

typedef uint8_t idType;
typedef int64_t diskIdType;
typedef int32_t copyType;
typedef uint32_t threadType;
typedef std::list<idType> IDListType;
typedef std::set<idType> IDSetType;
typedef std::set<diskIdType> diskIDSetType;
typedef std::list<diskIdType> diskIDListType;
typedef std::tr1::unordered_map<idType, Distributor*> distributorMapType;
typedef std::tr1::unordered_map<idType, std::list<VDRIVE::Disk*>* > diskMapType;
typedef std::tr1::unordered_map<diskIdType, uint64_t> resultMapType;
typedef std::tr1::unordered_map<idType, resultMapType* > resultsMapType;
typedef std::tr1::unordered_map<threadType, resultsMapType* > resultsThreadMapType;
typedef std::tr1::unordered_map<idType, int64_t> movedMapType;
typedef std::tr1::unordered_map<threadType, movedMapType* > movedThreadMapType;
typedef std::list<Disk*> diskListType;
typedef std::tr1::unordered_map<uint8_t, std::string*> distNamesType;

static IDListType *distributorIDs = 0;
static distributorMapType *distributorsThread;
static diskMapType *disksThread = 0;
static resultsThreadMapType *resultsThread;
static movedThreadMapType *movedWithOrderThread;
static movedThreadMapType *movedWithoutOrderThread;

static void usage() {
    cout << "\nUsage:\n" <<
            "    CPPDistComp [options]\n" <<
            "This Programm takes a file containing a list of initialized distributors. " <<
            "Then the skript places the given number of of extents " <<
            "using the first distributor " <<
            "and calculates how much Elements have to be replaced to get tor the next " <<
            "Distributor. Then it calculates how much has to be replaced get form second " <<
            "Distributro to third and so on...\n" <<
            "\n" <<
            "Options:\n" <<
            "  -dlf <distListFile>  A File containing the list of Distributors\n" <<
            "                       (necessary)\n" <<
            "  -odf <outFile>       write result (Elements placed per Disk) to given File\n" <<
            "                       (default: stdout)\n" <<
            "  -ocf <outFile>       write result (Movements) to given File\n" <<
            "                       (default: stdout)\n" <<
            "  -en N                distribute given number of extents\n" <<
            "                       (necessary)\n" <<
            "  -Tn N                Number of Threads distributing the Extents\n";
}

/*
static uint64_t timeInMS(struct timeval before, struct timeval after) {
    uint64_t beforems = before.tv_sec * 1000 + before.tv_usec / 1000;
    uint64_t afterms = after.tv_sec * 1000 + after.tv_usec / 1000;
    return afterms - beforems;
}
 */

void *runCompareTest(void *arg) {
    typedef std::tr1::unordered_map<diskIdType, copyType> choosenDiskMapType;

    struct runCompareTestParms *parms = (struct runCompareTestParms *) arg;
    int64_t lastID = parms->firstID + parms->numExtents;

    resultsMapType* results = (*resultsThread)[parms->threadID];
    movedMapType* movedWithOrder = (*movedWithOrderThread)[parms->threadID];
    movedMapType* movedWithoutOrder = (*movedWithoutOrderThread)[parms->threadID];

    IDListType::iterator idIterator;
    diskListType* choosenDisks;
    choosenDiskMapType *choosenDisksBefore, *choosenDisksAfter;
    idType id;
    copyType copy;
    resultMapType *resultDist = 0;
    Distributor *current;

    if (distributorIDs->empty())
        return 0;

    for (diskIdType i = parms->firstID; i < lastID; i++) {
        idIterator = distributorIDs->begin();
        id = *idIterator;
        resultDist = (*results)[id];
        current = (*distributorsThread)[id];
        choosenDisks = current->placeExtent(0, i);
        choosenDisksAfter = new choosenDiskMapType();
        copy = 0;
        for (diskListType::iterator it = choosenDisks->begin(); it != choosenDisks->end(); ++it) {
            Disk* disk = *it;
            (*resultDist)[disk->getId()]++;
            (*choosenDisksAfter)[disk->getId()] = copy;
            delete disk;
            copy++;
        }
        delete choosenDisks;
        idIterator++;
        while (idIterator != distributorIDs->end()) {
            id = *idIterator;
            resultDist = (*results)[id];
            current = (*distributorsThread)[id];
            choosenDisksBefore = choosenDisksAfter;
            choosenDisks = current->placeExtent(0, i);
            choosenDisksAfter = new choosenDiskMapType();
            copy = 0;
            for (list<Disk*>::iterator it = choosenDisks->begin(); it != choosenDisks->end(); ++it) {
                Disk* disk = *it;
                (*resultDist)[disk->getId()]++;
                (*choosenDisksAfter)[disk->getId()] = copy;
                if (choosenDisksBefore->count(disk->getId()) > 0) {
                    if ((*choosenDisksBefore)[disk->getId()] != copy) {
                        (*movedWithOrder)[id]++;
                    }
                } else {
                    (*movedWithOrder)[id]++;
                    (*movedWithoutOrder)[id]++;
                }
                copy++;
                delete disk;
            }
            delete choosenDisks;
            idIterator++;
            delete choosenDisksBefore;
        }
        delete choosenDisksAfter;
    }

    return arg;
}

static void readParams(int argc, char** argv, char** distListFileName, char** outDataFileName, char** outCompareFileName, uint64_t* numExtents, threadType* numThreads) {
    int argInd = 0;
    for (argInd = 1; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            usage();
            abort();
        } else if (strcmp(argv[argInd], "-dlf") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option dlf needs a value";
                usage();
                abort();
            }
            *distListFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-odf") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option odf needs a value";
                usage();
                abort();
            }
            *outDataFileName = argv[argInd];
        } else if (strcmp(argv[argInd], "-ocf") == 0) {
            argInd++;
            if (argInd == argc) {
                cerr << "Option ocf needs a value";
                usage();
                abort();
            }
            *outCompareFileName = argv[argInd];
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
                cerr << "-en has to be followed by a number\n";
                usage();
                abort();
            }
            if (val < 0) {
                cerr << "Negative number of extents?!?\n";
                usage();
                abort();
            }
            *numExtents = val;
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
            *numThreads = (int32_t) val;
        } else {
            cout << "unknown option: " << argv[argInd] << "\n";
            usage();
            abort();
        }
    }
}

static void trim(std::string &line) {
    std::stringstream trimmer;
    trimmer << line;
    line.clear();
    trimmer >> line;
}

static void loadDistributors(char* distListFileName, distNamesType *distFileNames) {
    ifstream inFile;
    string line;
    idType id;
    distributorIDs = new IDListType();
    distributorsThread = new distributorMapType();
    disksThread = new diskMapType();
    inFile.open(distListFileName);
    std::getline(inFile, line);
    id = 0;

    while (!inFile.eof()) {
        if (line.compare("") != 0) {
            cout << "will load " << line << "\n";
            trim(line);
            Distributor *dis = Distributor::loadDistributor(line);
            distributorIDs->push_back(id);
            (*distributorsThread)[id] = dis;
            (*distFileNames)[id] = new std::string(line);
            std::list<Disk*> *disks = dis->getDisks();
            (*disksThread)[id] = disks;
            id++;
        }
        std::getline(inFile, line);
    }
    inFile.close();
}

static void initThreads(uint32_t numThreads) {
    resultsThread = new resultsThreadMapType();
    movedWithOrderThread = new movedThreadMapType();
    movedWithoutOrderThread = new movedThreadMapType();

    for (uint32_t i = 0; i < numThreads; i++) {
        resultsMapType *resultsStep = new resultsMapType();
        movedMapType *movedWithOrderStep = new movedMapType();
        movedMapType *movedWithoutOrderStep = new movedMapType();
        (*resultsThread)[i] = resultsStep;
        (*movedWithOrderThread)[i] = movedWithOrderStep;
        (*movedWithoutOrderThread)[i] = movedWithoutOrderStep;
        for (IDListType::iterator it = distributorIDs->begin(); it != distributorIDs->end(); it++) {
            idType id = *it;
            diskListType *disks = (*disksThread)[id];
            resultMapType *resultsStepStep = new resultMapType();
            (*resultsStep)[id] = resultsStepStep;
            (*movedWithOrderStep)[id] = 0;
            (*movedWithoutOrderStep)[id] = 0;
            for (diskListType::iterator it2 = disks->begin(); it2 != disks->end(); it2++) {
                Disk *d = *it2;
                (*resultsStepStep)[d->getId()] = 0;
            }
        }
    }
}

static void runThreads(threadType numThreads, uint64_t numExtents) {
    uint64_t extentsPerThread = numExtents / numThreads;
    uint64_t restExtents = numExtents % numThreads;
    uint64_t nextID = 0;
    struct runCompareTestParms parms[numThreads];
    int errorCode = 0;
    pthread_t threads[numThreads];

    for (threadType i = 0; i < numThreads; i++) {
        parms[i].firstID = nextID;
        parms[i].threadID = i;
        if (i < restExtents) {
            parms[i].numExtents = extentsPerThread + 1;
        } else {
            parms[i].numExtents = extentsPerThread;
        }
        nextID += parms[i].numExtents;
    }
    if (numThreads > 1) {
        cout << "Multithreaded!\n";
        for (threadType i = 0; i < numThreads; i++) {
            if ((errorCode = pthread_create(&threads[i], 0, runCompareTest, &parms[i]))) {
                cerr << "Error " << errorCode << " while trying to start thread " << i << "\n";
                abort();
            }
        }
        for (threadType i = 0; i < numThreads; i++) {
            void *threadRes = 0;
            if ((errorCode = pthread_join(threads[i], &threadRes))) {
                cerr << "Error " << errorCode << " joining Thread " << i << "\n";
                abort();
            }
        }
    } else {
        cout << "No Threads!\n";
        runCompareTest(&parms[0]);
    }
    cout << "Terminating runThreads\n";
}

static void crunchData(movedMapType *movedWithOrder, movedMapType *movedWithoutOrder, resultsMapType *results, resultsMapType *capacities, diskIDSetType *allDiskIDs) {
    for (IDListType::iterator it = distributorIDs->begin(); it != distributorIDs->end(); it++) {
        idType id = *it;
        resultMapType *resultsStep = new resultMapType();
        resultMapType *capacitiesStep = new resultMapType();
        diskListType *disks = (*disksThread)[id];
        (*movedWithOrder)[id] = 0;
        (*movedWithoutOrder)[id] = 0;
        (*results)[id] = resultsStep;
        (*capacities)[id] = capacitiesStep;
        for (diskListType::iterator it2 = disks->begin(); it2 != disks->end(); it2++) {
            Disk *d = *it2;
            (*resultsStep)[d->getId()] = 0;
            (*capacitiesStep)[d->getId()] = d->getCapacity();
        }
    }

    for (diskMapType::iterator it = disksThread->begin(); it != disksThread->end(); it++) {
        diskListType *disks = it->second;
        for (diskListType::iterator it2 = disks->begin(); it2 != disks->end(); it2++) {
            allDiskIDs->insert((*it2)->getId());
            delete *it2;
        }
        delete disks;
    }
    delete disksThread;

    for (movedThreadMapType::iterator it = movedWithOrderThread->begin(); it != movedWithOrderThread->end(); it++) {
        movedMapType *step = it->second;
        for (movedMapType::iterator it2 = step->begin(); it2 != step->end(); it2++) {
            (*movedWithOrder)[it2->first] += it2->second;
        }
        delete step;
    }
    delete movedWithOrderThread;

    for (movedThreadMapType::iterator it = movedWithoutOrderThread->begin(); it != movedWithoutOrderThread->end(); it++) {
        movedMapType *step = it->second;
        for (movedMapType::iterator it2 = step->begin(); it2 != step->end(); it2++) {
            (*movedWithoutOrder)[it2->first] += it2->second;
        }
        delete step;
    }
    delete movedWithoutOrderThread;

    for (resultsThreadMapType::iterator it = resultsThread->begin(); it != resultsThread->end(); it++) {
        resultsMapType *resultStep = it->second;
        for (resultsMapType::iterator it2 = resultStep->begin(); it2 != resultStep->end(); it2++) {
            resultMapType *resultStepStep = it2->second;
            uint8_t id = it2->first;
            resultMapType *realResultsStep = (*results)[id];
            for (resultMapType::iterator it3 = resultStepStep->begin(); it3 != resultStepStep->end(); it3++) {
                (*realResultsStep)[it3->first] += it3->second;
            }
            delete resultStepStep;
        }
        delete resultStep;
    }
    delete resultsThread;
}

static void writeData(char* outDataFileName, diskIDSetType *allDiskIDs, distNamesType *distFileNames, resultsMapType *results, resultsMapType *capacities) {
    ostream *out = 0;
    ofstream *outf = 0;
    if (outDataFileName == 0) {
        out = &cout;
    } else {
        outf = new ofstream();
        outf->open(outDataFileName);
        out = outf;
    }

    (*out) << "Distributor;diskdata;";
    for (diskIDSetType::iterator it = allDiskIDs->begin(); it != allDiskIDs->end(); it++) {
        (*out) << *it << ";";
    }
    (*out) << "\n";
    
    ofstream file("consistent.csv");
    for (IDListType::iterator idIt = distributorIDs->begin(); idIt != distributorIDs->end(); idIt++) {
        resultMapType *resultsStep = (*results)[*idIt];
        resultMapType *capacitiesStep = (*capacities)[*idIt];
        (*out) << (*(*distFileNames)[*idIt]) << ";used;";
        for (diskIDSetType::iterator it = allDiskIDs->begin(); it != allDiskIDs->end();) {
            if (resultsStep->count(*it) != 0) {
                (*out) << (*resultsStep)[*it];
                file << (*resultsStep)[*it] << "\n";
                //file << (*resultsStep)[*it] << "\n";
            }
            it++;
            if (it != allDiskIDs->end()) {
                (*out) << ";";
            }
        }
        (*out) << "\n";
        (*out) << (*(*distFileNames)[*idIt]) << ";capacity;";
        for (diskIDSetType::iterator it = allDiskIDs->begin(); it != allDiskIDs->end(); ) {
            if (capacitiesStep->count(*it) != 0) {
                (*out) << (*capacitiesStep)[*it];
            }
            it++;
            if (it != allDiskIDs->end()) {
                (*out) << ";";
            }
        }
        (*out) << "\n";
    }
    file.close();

    if (outf != 0) {
        outf->close();
        delete outf;
        outf = 0;
    }
}

static void writeCompare(char* outCompareFileName, distNamesType *distFileNames, resultsMapType *results, resultsMapType *capacities, movedMapType *movedWithOrder, movedMapType *movedWithoutOrder) {
    ostream *out = 0;
    ofstream *outf = 0;
    idType id = 0, beforeId = 0;
    diskIDSetType *disksBeforeID = 0, *disksAfterID = 0;
    uint64_t sysCapacityBefore = 0, sysCapacityAfter = 0;

    if (outCompareFileName == 0) {
        out = &cout;
    } else {
        outf = new ofstream();
        outf->open(outCompareFileName);
        out = outf;
    }

    (*out) << "DistributorBefore;DistributorAfter;SysCapacityBefore;SysCapacityAfter;AddedCapacity;RemovedCapacity;ChangedCapacity;MovedWithOrder;MovedWithoutOrder;MovedFromRemoved;\nMovedToAdded\n";

    for (IDListType::iterator idIt = distributorIDs->begin(); idIt != distributorIDs->end(); idIt++) {
        beforeId = id;
        id = *idIt;
        resultMapType *capacitiesStepAfter = (*capacities)[id];

        disksBeforeID = disksAfterID;
        sysCapacityBefore = sysCapacityAfter;
        disksAfterID = new diskIDSetType();
        sysCapacityAfter = 0;

        for (resultMapType::iterator it = capacitiesStepAfter->begin(); it != capacitiesStepAfter->end(); it++) {
            disksAfterID->insert(it->first);
            sysCapacityAfter += it->second;
        }

        if (disksBeforeID != 0) {
            resultMapType *capacitiesStepBefore = (*capacities)[beforeId];
            resultMapType *resultsStepBefore = (*results)[beforeId];
            resultMapType *resultsStepAfter = (*results)[id];
            uint64_t addedCapacity = 0, removedCapacity = 0, changedCapacity = 0, movedFromRemoved = 0, movedToAdded = 0;
            diskIDListType *addedDisks, *changedDisks, *removedDisks;
            addedDisks = new diskIDListType();
            changedDisks = new diskIDListType();
            removedDisks = new diskIDListType();

            for (resultMapType::iterator it = capacitiesStepBefore->begin(); it != capacitiesStepBefore->end(); it++) {
                if (capacitiesStepAfter->count(it->first) == 0) {
                    removedDisks->push_back(it->first);
                    removedCapacity += it->second;
                    movedFromRemoved += (*resultsStepBefore)[it->first];
                } else {
                    if (it->second != (*capacitiesStepAfter)[it->first]) {
                        changedDisks->push_back(it->first);
                        if (it->second > (*capacitiesStepAfter)[it->first]) {
                            changedCapacity += it->second - (*capacitiesStepAfter)[it->first];
                        } else {
                            changedCapacity += (*capacitiesStepAfter)[it->first] - it->second;
                        }
                    }
                }
            }

            for (resultMapType::iterator it = capacitiesStepAfter->begin(); it != capacitiesStepAfter->end(); it++) {
                if (capacitiesStepBefore->count(it->first) == 0) {
                    addedDisks->push_back(it->first);
                    addedCapacity += it->second;
                    movedToAdded += (*resultsStepAfter)[it->first];
                }
            }

            (*out) << (*(*distFileNames)[beforeId]) << ";" << (*(*distFileNames)[id]) << ";" << sysCapacityBefore << ";" << sysCapacityAfter << ";";
            (*out) << addedCapacity << ";" << removedCapacity << ";" << changedCapacity << ";";
            (*out) << (*movedWithOrder)[id] << ";" << (*movedWithoutOrder)[id] << ";" << movedFromRemoved << ";\n" << movedToAdded;
            (*out) << "\n";

            delete addedDisks;
            delete removedDisks;
            delete changedDisks;
            delete disksBeforeID;
        }
    }
    delete disksAfterID;


    if (outf != 0) {
        outf->close();
        delete outf;
        outf = 0;
    }
}

int main(int argc, char** argv) {
    char *distListFileName = 0, *outDataFileName = 0, *outCompareFileName = 0;
    uint64_t numExtends = 0;
    threadType numThreads = 1;
    resultsMapType *results, *capacities;
    movedMapType *movedWithOrder, *movedWithoutOrder;
    diskIDSetType *allDiskIDs;
    distNamesType *distFileNames;

    readParams(argc, argv, &distListFileName, &outDataFileName, &outCompareFileName, &numExtends, &numThreads);

    if ((distListFileName == 0) || (numExtends == 0)) {
        cerr << "You need to set -dbf, -daf and -en\n";
        usage();
        abort();
    }

    // Load Distributors
    distFileNames = new distNamesType();
    loadDistributors(distListFileName, distFileNames);

    // Initialize Data Structures for Threads.
    initThreads(numThreads);

    // run the tests
    runThreads(numThreads, numExtends);

    // First we delete the distributors, so we get lots of free mem...
    for (distributorMapType::iterator it = distributorsThread->begin(); it != distributorsThread->end(); it++) {
        delete it->second;
    }
    delete distributorsThread;

    // bring the data together
    movedWithOrder = new movedMapType();
    movedWithoutOrder = new movedMapType();
    results = new resultsMapType();
    capacities = new resultsMapType();
    allDiskIDs = new diskIDSetType();
    crunchData(movedWithOrder, movedWithoutOrder, results, capacities, allDiskIDs);

    // Output of data
    writeData(outDataFileName, allDiskIDs, distFileNames, results, capacities);

    // Output movements
    writeCompare(outCompareFileName, distFileNames, results, capacities, movedWithOrder, movedWithoutOrder);

    for (distNamesType::iterator it = distFileNames->begin(); it != distFileNames->end(); it++) {
        delete it->second;
    }
    delete distFileNames;
    delete allDiskIDs;
    delete distributorIDs;
    for (resultsMapType::iterator it = results->begin(); it != results->end(); it++) {
        delete it->second;
    }
    delete results;
    for (resultsMapType::iterator it = capacities->begin(); it != capacities->end(); it++) {
        delete it->second;
    }
    delete capacities;
    delete movedWithOrder;
    delete movedWithoutOrder;
    return (EXIT_SUCCESS);
}
