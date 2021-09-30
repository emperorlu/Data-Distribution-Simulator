# To change this template, choose Tools | Templates
# and open the template in the editor.
#import os.path
import csv
import numpy as np
import scipy.stats
#import matplotlib
#matplotlib.use("PDF")
#import glob
#import os
#import matplotlib.pyplot as plt

__author__="fermat"
__date__ ="$13.07.2010 09:47:20$"

def read_movement_data(filename):
    file = open(filename)
    description = file.readline()
    description = description.strip()
    data = file.readline()
    data = data.strip()
    return (description.split(";"), data.split(";"))
    
def read_memdata(filename):
    file = open(filename)
    for line in file:
        if line.startswith("VmRSS:"):
            entries = line.split()
            mem = int(entries[len(entries) - 2])
        if line.startswith("VmPeak:"):
            entries = line.split()
            maxmem = int(entries[len(entries) - 2])
    return (mem, maxmem)

def read_speed_initialization(filename):
    file = open(filename)
    for line in file:
        if line.startswith("Used Time for Initialisation:"):
            entries = line.split(":")
            speed = int(entries[1])
    file.close()
    return speed

def read_speed_distribution(filename):
    file = open(filename)
    speed=None
    for line in file:
        if line.startswith("Used Time for Placing Extents:"):
            entries = line.split(":")
            speed = int(entries[1])
    file.close()
    return speed
    
def read_csv_file(filename, rotated=False, cellDelimiter=';'):
    f = open(filename)
    r = csv.reader(f, delimiter=cellDelimiter)
    data = [[c for c in row] for row in r]
    f.close()
    if rotated:
        rdata = []
        for i in xrange(len(data[0])):
            rdata.append([])
        for i in xrange(len(data)):
            for j in xrange(len(data[i])):
                rdata[j].append(data[i][j])
        data = rdata
    return data

class FileRuntimeData:
    def __init__(self, filename, files):
        self.placedExtents = 1000000
        self.placedFiles = files
        #print files
        #print filename
        file = open(filename)
        for line in file:    
            if line.startswith("Get Disk from File Distributor time sum:"):
                entries = line.split(":")
                self.fileDist = int(entries[1])
            if line.startswith("Get Disk from File Distributor square time sum:"):
                entries = line.split(":")
                self.fileDistSquare = int(entries[1])
            if line.startswith("Create Block Distributor time sum:"):
                entries = line.split(":")
                self.blockCreate = int(entries[1])
            if line.startswith("Create Block Distributor square time sum:"):
                entries = line.split(":")
                self.blockCreateSquare = int(entries[1])
            if line.startswith("Place single Block time sum:"):
                entries = line.split(":")
                self.placeBlock = int(entries[1])
            if line.startswith("Place single Block square time sum:"):
                entries = line.split(":")
                self.placeBlockSquare = int(entries[1])
            if line.startswith("Place file blocks time sum:"):
                entries = line.split(":")
                self.placeBlocks = int(entries[1])
            if line.startswith("Place file blocks square time sum:"):
                entries = line.split(":")
                self.placeBlocksSquare = int(entries[1])
            if line.startswith("Complete File time sum:"):
                entries = line.split(":")
                self.placeFile = int(entries[1])
            if line.startswith("Complete File square time sum:"):
                entries = line.split(":")
                self.placeFileSquare = int(entries[1])

        #print self.placeBlocks

    def get_extents(self):
        return self.placedExtents

    def get_files(self):
        return self.placedFiles

    def get_sum_file_dist_time(self):
        return self.fileDist

    def get_square_sum_file_dist_time(self):
        return self.fileDistSquare

    def get_average_file_dist_time(self):
        return float(self.fileDist) / float(self.placedFiles)

    def get_std_dev_file_dist_average_time(self):
        up = (self.placedFiles * self.fileDistSquare) - (self.fileDist * self.fileDist)
        down = self.placedFiles * (self.placedFiles - 1)
        return float(up) / float(down)

    def get_sum_block_create_time(self):
        return self.blockCreate

    def get_square_sum_block_create_time(self):
        return self.blockCreateSquare

    def get_average_block_create_time(self):
        return float(self.blockCreate) / float(self.placedFiles)

    def get_std_dev_block_create_average_time(self):
        up = (self.placedFiles * self.blockCreateSquare) - (self.blockCreate * self.blockCreate)
        down = self.placedFiles * (self.placedFiles - 1)
        return float(up) / float(down)

    def get_sum_place_blocks_time(self):
        return self.placeBlocks

    def get_square_sum_place_blocks_time(self):
        return self.placeBlocksSquare

    def get_average_place_blocks_time(self):
        return float(self.placeBlocks) / float(self.placedFiles)

    def get_std_dev_place_blocks_average_time(self):
        up = (self.placedFiles * self.placeBlocksSquare) - (self.placeBlocks * self.placeBlocks)
        down = self.placedFiles * (self.placedFiles - 1)
        return float(up) / float(down)

    def get_sum_place_block_time(self):
        return self.placeBlock

    def get_square_sum_place_block_time(self):
        return self.placeBlockSquare

    def get_average_place_block_time(self):
        return float(self.placeBlock) / float(self.placedExtents)

    def get_std_dev_place_block_average_time(self):
        up = (self.placedExtents * self.placeBlockSquare) - (self.placeBlock * self.placeBlock)
        down = self.placedExtents * (self.placedExtents - 1)
        return float(up) / float(down)

    def get_sum_place_file_time(self):
        return self.placeFile

    def get_square_sum__place_file_time(self):
        return self.placeFileSquare

    def get_average_place_file_time(self):
        return float(self.placeFile) / float(self.placedFiles)

    def get_std_dev_place_file_average_time(self):
        up = (self.placedFiles * self.placeFileSquare) - (self.placeFile * self.placeFile)
        down = self.placedFiles * (self.placedFiles - 1)
        return float(up) / float(down)

class RuntimeData:
    def __init__(self, filename):
        file = open(filename)
        for line in file:
            if line.startswith("Used Time for Initialisation:"):
                entries = line.split(":")
                self.initTime = int(entries[1])
            elif line.startswith("Used Time for Placing Extents:"):
                entries = line.split(":")
                self.placementTime = int(entries[1])
            elif line.startswith("Placed Extents:"):
                entries = line.split(":")
                self.placedExtents = int(entries[1])
            elif line.startswith("Placed copies of each Extent:"):
                entries = line.split(":")
                self.placedCopies = int(entries[1])
            elif line.startswith("Sum time of placing single Extents:"):
                entries = line.split(":")
                self.sumPlacementTimes = int(entries[1])
            elif line.startswith("Sum of squares of placing single Extents:"):
                entries = line.split(":")
                self.sumSquarePlacementTimes = int(entries[1])
            elif line.startswith("Varianz of time placing Extents:"):
                entries = line.split(":")
                self.placementTimesVarianz = int(entries[1])
                self.placementTimesVarianzLength = len(entries[1])
            elif line.startswith("Varianz of time placing Extents Exponend:"):
                entries = line.split(":")
                self.placementTimesVarianzExponent = int(entries[1])
            elif line.startswith("Standard Deviation placing Extents:"):
                entries = line.split(":")
                self.placementTimesDeviation = int(entries[1])
                self.placementTimesDeviationLength = len(entries[1])
            elif line.startswith("Standard Deviation placing Extents Exponend:"):
                entries = line.split(":")
                self.placementTimesDeviationExponent = int(entries[1])
            elif line.startswith("Used Threads to place Extents:"):
                entries = line.split(":")
                self.threads = int(entries[1])

    def get_threads(self):
        return self.threads
    
    def get_used_init_time(self):
        return self.initTime

    def get_allover_placement_time(self):
        return self.placementTime

    def get_extents(self):
        return self.placedExtents

    def get_copies(self):
        return self.placedCopies

    def get_sum_placement_times(self):
        return self.sumPlacementTimes

    def get_sum_square_placent_times(self):
        return self.sumSquarePlacementTimes

    def get_varianz_calc(self):
        up = (self.placedExtents * self.sumSquarePlacementTimes) - (self.sumPlacementTimes * self.sumPlacementTimes)
        down = self.placedExtents * (self.placedExtents - 1)
        return float(up) / float(down)

    def get_varianz_read_q(self):
        devisior = 10 ** (self.placementTimesVarianzLength - self.placementTimesVarianzExponent)
        return self.placementTimesVarianz, devisior

    def get_varianz_read_f(self):
        devisior = 10 ** (self.placementTimesVarianzLength - self.placementTimesVarianzExponent)
        return float(self.placementTimesVarianz) / float(devisior)

    def get_std_deviation_calc(self):
        return self.get_varianz_calc() ** 0.5

    def get_std_deviation_read_q(self):
        devisior = 10 ** (self.placementTimesDeviationLength - self.placementTimesDeviationExponent)
        return self.placementTimesDeviation, devisior

    def get_std_deviation_read_f(self):
        devisior = 10 ** (self.placementTimesDeviationLength - self.placementTimesDeviationExponent)
        return float(self.placementTimesDeviation) / float(devisior)

    def get_average_placement_time(self):
        return float(self.sumPlacementTimes) / float(self.placedExtents)

    def get_average_placement_time_per_copy(self):
        return self.get_average_placement_time() / float(self.placedCopies)

class FairnessData:
    def __init__(self, ids, used, capacities):
        self.ids = np.array(ids, dtype=np.int)
        self.used = np.array(used)
        self.capacities = np.array(capacities)
        self.ratios = np.zeros(len(used), dtype=np.float)
        self.allUsed = 0
        self.allCapacity = 0
        for i in xrange(len(used)):
            self.ratios[i] = 1.0 * float(self.used[i])
            self.ratios[i] = self.ratios[i] / float(self.capacities[i])
            self.allCapacity += int(capacities[i])
            self.allUsed += int(used[i])
            #print "Disk " + str(ids[i]) + ": " + str(used[i]) + " / " + str(capacities[i])
            if i == 0:
                self.minRatio = self.ratios[0]
                self.maxRatio = self.ratios[0]
            else:
                if self.minRatio > self.ratios[i]:
                    self.minRatio = self.ratios[i]
                if self.maxRatio < self.ratios[i]:
                    self.maxRatio = self.ratios[i]
            #if (self.ratios[i] < 0.4) or (self.ratios[i]>0.6):
            #    print "At position " + str(i) + " (id: " + str(self.ids[i]) + ", used: " + str(self.used[i]) + ", capacity: " + str(self.capacities[i]) + ") found ratio " + str(self.ratios[i]) + "\n";
        #self.meanRatio = np.mean(self.ratios)
        #print "All Used: " + str(allUsed)
        self.meanRatio = float(self.allUsed) / float(self.allCapacity)
        tmp = 0
        for i in xrange(len(self.ratios)):
            tmp += (self.meanRatio - self.ratios[i]) ** 2
        self.varianz = tmp / len(self.ratios)
        self.eVarianz = tmp / (len(self.ratios) - 1)
        self.devergation = self.varianz ** 0.5
        #t = scipy.stats.t._ppf((1+0.95)/2., len(self.ratios)-1)
        #self.myMeanConfRatio = t * (self.eVarianz ** 0.5) / (len(self.ratios) ** 0.5) delivers exactly the same as mean_confidence_spread
        self.meanConfRatio = mean_confidence_spread(self.ratios)

def mean(data):
    """
    Calculate the Mean value of the given array.
    """
    a = 1.0 * np.array(data)
    return np.mean(a)

def mean_confidence_spread(data, confidence=0.95):
    """
    calculate the confidence intervall for the given array
    """
    a = 1.0*np.array(data)
    n = len(a)
    se = scipy.stats.sem(a)

    # calls the inverse CDF of the Student's t distribution
    h = se * scipy.stats.t._ppf((1+confidence)/2., n-1)
    return h

def mean_confidence_interval(data, confidence=0.95):
    """
    calculate the confidence intervall for the given array
    """
    a = 1.0*np.array(data)
    m = np.mean(a)
    # calls the inverse CDF of the Student's t distribution
    h = mean_confidence_spread(data, confidence)
    return m-h, m+h
