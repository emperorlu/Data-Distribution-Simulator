#! /usr/bin/python

import os.path
import numpy as np
import os
import matplotlib
import matplotlib.pyplot as plt
import DistributorData
import csv
import createImages
import createHashImages

__author__="fermat"
__date__ ="$07.08.2010 12:25:37$"

imagePath = "/Users/fermat/Uni/Dissertation/Messergebnisse/images/"

def draw_img_memtime_diskcopy(memfiles, speedfiles, label, filename=None, xbar="Festplatten", ybar="memory (MB)", ylines="time", disks=None, copies=None, draw_legend=False, imgRanges=None):
    if copies == None:
        copies = list(memfiles.keys())
        copies.sort()
    if len(copies) < 1:
        print "draw_img_fairness: Got empty datafiles"
        return None
    if disks == None:
        disks = list(memfiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print "draw_img_fairness: Got datafiles without diskentries"
        return None
    allMems = []
    allSpeeds = []
    legend = []
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
        fields=("copies", "disks", "mem", "mem Peak", "time ms", "time ms per Extent", "time devergation", "File Mem", "File Speed")
        csvFile.writerow(fields)
    for copy in copies:
        copyMemfiles = memfiles[copy]
        copySpeedfiles = speedfiles[copy]
        means = []
        mins = []
        maxs = []
        speeds = []
        speedDevergations = []
        for disk in disks:
            memfile = copyMemfiles[disk]
            speedfile = copySpeedfiles[disk]
            fields = [0] * 9
            fields[0] = copy
            fields[1] = disk
            fields[7] = memfile
            fields[8] = speedfile
            if (speedfile is not None) and (not os.path.isfile(speedfile)):
                print "File " + speedfile + " does not exist"
                speedfile = None
            if (memfile is not None) and (not os.path.isfile(memfile)):
                print "File " + memfile + " does not exist"
                memfile = None
            if speedfile is None:
                speeds.append(0)
                speedDevergations.append(0)
                fields[4] = ""
                fields[5] = ""
                fields[6] = ""
            else:
                speedData = DistributorData.RuntimeData(speedfile)
                speed = speedData.get_sum_placement_times()
                #speeds.append(float(speed) / float(1000))
                fields[4] = str(speed).replace(".", ",")
                extents = speedData.get_extents()
                speed = float(speed) / float(extents)
                fields[5] = str(speed).replace(".", ",")
                speeds.append(speed)
                speedDevergation = speedData.get_std_deviation_calc()
                speedDevergations.append(speedDevergation)
                fields[6] = str(speedDevergation).replace(".", ",")
            if memfile is None:
                means.append(float(0))
                mins.append(float(0))
                maxs.append(float(0))
                fields[2] = ""
                fields[3] = ""
                pass
            else:
                memdata = DistributorData.read_memdata(memfile)
                means.append(0)
                mins.append(float(memdata[0]) / float(1024))
                maxs.append(float(memdata[1] - memdata[0]) / float(1024))
                fields[2] = str(memdata[0]).replace(".", ",")
                fields[3] = str(memdata[1]).replace(".", ",")
            if csvFile is not None:
                csvFile.writerow(fields)
        allMems.append((mins, maxs, means, speeds, speedDevergations))
        #allConf[0].append([mins, maxs])
        if copy == 1:
            legend.append("1 copy")
        else:
            legend.append(str(copy) + " copies mem")
    xticks = []
    for disk in disks:
        xticks.append(str(disk))
    #errorLegend = None
    if draw_legend:
        createHashImages.draw_image_bar_line(allMems, filename, xticks, None, "Copies of each Disk", ybar, "time ($\mu$s)", label, legend, "VMPeak", None, "used time", imgRanges=imgRanges)
    else:
        if imgRanges is None:
            imgRanges = [0.17, 0.27, 0.67, 0.6]
        createHashImages.draw_image_bar_line(allMems, filename, xticks, None, "Copies of each Disk", ybar, "time ($\mu$s)", label, None, None, None, None, imgRanges=imgRanges)


def draw_img_diskcopy(datafiles, label, filename, xbar="Festplatten", ybar="Auslatung in %", disks=None, copies=None, draw_legend=False):
    if copies == None:
        copies = list(datafiles.keys())
        copies.sort()
    if len(copies) < 1:
        print "draw_img_fairness: Got empty datafiles"
        return None
    if disks == None:
        disks = list(datafiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print "draw_img_fairness: Got datafiles without diskentries"
        return None
    allMeans = []
    allConf = [[],]
    legend = []
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
        fields=("copies", "disks", "mean usage", "min usage", "max usage", "conf Interval", "File Dataplacement")
        csvFile.writerow(fields)
    for copy in copies:
        copyDatafiles = datafiles[copy]
        means = []
        mins = []
        maxs = []
        confs = []
        for disk in disks:
            datafile = copyDatafiles[disk]
            fields = [0] * 7
            fields[0] = copy
            fields[1] = disk
            fields[6] = datafile
            if (datafile != None) and (not os.path.isfile(datafile)):
                print "File " + datafile + " does not exist"
                datafile = None
            if datafile == None:
                means.append(float(0))
                mins.append(float(0))
                maxs.append(float(0))
                confs.append(float(0))
                fields[2] = ""
                fields[3] = ""
                fields[4] = ""
                fields[5] = ""
            else:
                filedata = DistributorData.read_csv_file(datafile, rotated=True)
                dataObj = DistributorData.FairnessData(filedata[0], filedata[1], filedata[2])
                means.append(1)
                mins.append(dataObj.minRatio / dataObj.meanRatio)
                maxs.append((dataObj.maxRatio - dataObj.minRatio) / dataObj.meanRatio)
                confs.append(dataObj.meanConfRatio / dataObj.meanRatio)
                fields[2] = str(dataObj.meanRatio).replace(".", ",")
                fields[3] = str(dataObj.minRatio).replace(".", ",")
                fields[4] = str(dataObj.maxRatio).replace(".", ",")
                fields[5] = str(dataObj.meanConfRatio).replace(".", ",")
            if csvFile is not None:
                csvFile.writerow(fields)
        allMeans.append((mins, maxs, means))
        #allConf[0].append([mins, maxs])
        allConf[0].append(confs)
        if copy == 1:
            legend.append("1 copy")
            #legend.append("1 copy used")
        else:
            legend.append(str(copy) + " copies")
            #legend.append(str(copy) + " copies used")
    xticks = []
    for disk in disks:
        xticks.append(str(disk))
    #errorLegend = None
    if draw_legend:
        createImages.draw_image_bar(allMeans, filename, xticks, allConf, "Copies of each Disk", "usage", label, legend, "min/max usage", None)
    else:
        createImages.draw_image_bar(allMeans, filename, xticks, allConf, "Copies of each Disk", "usage", label, None, None, None)

def draw_img_fairness_nearestneighbour_diskcopys_hom():
    path = "/Users/fermat/Uni/Dissertation/Messergebnisse/NNCopies/homogen/64Disk/ausgaben/"
    files = dict()
    for j in xrange(0,4):
        copies = 2**j
        thisfile = dict()
        for i in xrange (0,20):
            diskCopy = 100 * (i + 1)
            thisfile[diskCopy] = path + "CreateAndRun_NearestNeighbour_" + str(copies) + "K_1T-64D_hom.disks-0--" + str(diskCopy) + "DiskCopy.csv"
        files[copies] = thisfile
    draw_img_diskcopy(files, None, imagePath + "Fair_Hom_NearestNeighbourDiskCopies.png")

def draw_img_mem_time_hom_nearestneighbour_diskcopy():
    # Example for one figure with different axes:
    path = "/Users/fermat/Uni/Dissertation/Messergebnisse/NNCopies/homogen/64Disk/ausgaben/"

    memfiles = dict()
    speedFiles = dict()
    for i in xrange(0,4):
        copy = 2**i
        copyMems = dict()
        copySpeeds = dict()
        for j in xrange(0,20):
            diskCopy = 100 * (j + 1)
            copyMems[diskCopy] = path + "CreateAndRun_NearestNeighbour_" + str(copy) + "K_1T-64D_hom.disks-0--" + str(diskCopy) + "DiskCopy.run.out.memdata"
            copySpeeds[diskCopy] = path + "CreateAndRun_NearestNeighbour_" + str(copy) + "K_1T-64D_hom.disks-0--" + str(diskCopy) + "DiskCopy.run.out.std"
        memfiles[copy] = copyMems
        speedFiles[copy] = copySpeeds
    draw_img_memtime_diskcopy(memfiles, speedFiles, None, imagePath + "MemTime_Hom_NearestNeighbourDiskCopies")

if __name__ == "__main__":
    print "Hello World";
    draw_img_fairness_nearestneighbour_diskcopys_hom()
    draw_img_mem_time_hom_nearestneighbour_diskcopy()
