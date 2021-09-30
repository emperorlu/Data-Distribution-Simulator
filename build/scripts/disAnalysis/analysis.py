import os.path
import os
import DistributorData
import csv

__author__="fermat"
__date__ ="$17.09.2010 12:03:53$"

hashes = (0, 10, 1, 6, 305, 4000, 4003, 4004)

hashNames = dict()
hashNames[4000] = "LC"
hashNames[4003] = "Rand"
hashNames[4004] = "MT"
hashNames[0] = "SHA1"
hashNames[2] = "SHA1"
hashNames[11] = "SHA224"
hashNames[8] = "SHA256"
hashNames[9] = "SHA284"
hashNames[10] = "SHA512"
hashNames[301] = "MD4"
hashNames[1] = "MD5"
hashNames[6] = "TIGER"
hashNames[305] = "WHIRLPOOL"

class FairnessData:
    def __init__(self):
        pass

class HashData:
    def __init__(self):
        pass

class AdaptivityData:
    def __init__(self):
        pass

class MemtimeData:
    def __init__(self):
        pass

def calc_fairness_data(datafiles, filename, disks=None, copies=None):
    if copies == None:
        copies = list(datafiles.keys())
        copies.sort()
    if len(copies) < 1:
        print("draw_img_fairness: Got empty datafiles")
        return None
    if disks == None:
        disks = list(datafiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print("draw_img_fairness: Got datafiles without diskentries")
        return None
    allMeans = []
    allConf = [[],]
    legend = []
    areaLegend = []
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
        fields=("copies", "disks", "mean usage", "min usage", "max usage", "devergation", "min Factor", "max Factor", "devergation Factor", "File Dataplacement")
        csvFile.writerow(fields)
    for copy in copies:
        copyDatafiles = datafiles[copy]
        means = []
        mins = []
        maxs = []
        confs = []
        for disk in disks:
            datafile = copyDatafiles[disk]
            fields = [0] * 10
            fields[0] = copy
            fields[1] = disk
            fields[9] = datafile
            if (datafile != None) and (not os.path.isfile(datafile)):
                print("File " + datafile + " does not exist")
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
                fields[6] = ""
                fields[7] = ""
                fields[8] = ""
            else:
                filedata = DistributorData.read_csv_file(datafile, rotated=True)
                dataObj = DistributorData.FairnessData(filedata[0], filedata[1], filedata[2])
                means.append(1)
                mins.append(dataObj.minRatio / dataObj.meanRatio)
                maxs.append((dataObj.maxRatio - dataObj.minRatio) / dataObj.meanRatio)
                confs.append(dataObj.devergation / dataObj.meanRatio)
                fields[2] = str(dataObj.meanRatio).replace(".", ",")
                fields[3] = str(dataObj.minRatio).replace(".", ",")
                fields[4] = str(dataObj.maxRatio).replace(".", ",")
                fields[5] = str(dataObj.devergation).replace(".", ",")
                fields[6] = str(dataObj.minRatio / dataObj.meanRatio).replace(".", ",")
                fields[7] = str(dataObj.maxRatio / dataObj.meanRatio).replace(".", ",")
                fields[8] = str(dataObj.devergation / dataObj.meanRatio).replace(".", ",")
            if csvFile is not None:
                csvFile.writerow(fields)
        allMeans.append((mins, maxs, means))
        allConf[0].append(confs)
        if copy == 1:
            legend.append("1 copy")
            areaLegend.append("1 copy min/max")
        else:
            legend.append(str(copy) + " copies")
            areaLegend.append(str(copy) + " copies min/max")
    xticks = []
    for disk in disks:
        xticks.append(str(disk))

    result = FairnessData()
    result.allMeans = allMeans
    result.allConf = allConf
    result.legend = legend
    result.areaLegend = areaLegend
    result.xticks = xticks
    result.intervallLegend = "conf. Int. 95%"

    return result

def calc_hash_data(datafiles, dataspeeds, filename=None, usedHashes=None, copies=None):
    if usedHashes is None:
#        usedHashes = hashes
        usedHashes = hashes
        #usedHashes.sort()
    if len(usedHashes) < 1:
        print("No HashData")
        return None
    if copies is None:
        copies = datafiles[usedHashes[0]].keys()
        copies.sort()
    if len(copies) < 1:
        print("No Copies")
        return None
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
        fields=("copies", "hash algorithm", "mean usage", "min usage", "max usage", "conf Interval", "time ms", "time ms per Extent", "time devergation", "File Dataplacement", "File Speed")
        csvFile.writerow(fields)
    allMeans = []
    allConf = [[],]
    legend = []
    areaLegend = []
    speedLegend = []
    for hash in usedHashes:
        copyDatafiles = datafiles[hash]
        speedDatafiles = dataspeeds[hash]
        means = []
        mins = []
        maxs = []
        confs = []
        speeds = []
        speedDevergations = []
        for copy in copies:
            datafile = copyDatafiles[copy]
            dataspeed = speedDatafiles[copy]
            fields = [0] * 11
            fields[0] = copy
            fields[1] = hashNames[hash]
            fields[9] = datafile
            fields[10] = dataspeed
            if (dataspeed is not None) and (not os.path.isfile(dataspeed)):
                print("File " + dataspeed + " does not exist")
                dataspeed = None
            if dataspeed is None:
                speeds.append(float(0))
                speedDevergations.append(0)
                fields[6] = ""
                fields[7] = ""
                fields[8] = ""
            else:
                speedData = DistributorData.RuntimeData(dataspeed)
                speed = speedData.get_sum_placement_times()
                fields[6] = str(speed).replace(".", ",")
                speed = float(speed) / float(speedData.get_extents())
                speeds.append(float(speed))
                fields[7] = str(speed).replace(".", ",")
                speedDevergation = speedData.get_std_deviation_calc()
                speedDevergations.append(speedDevergation)
                fields[8] = str(speedDevergation).replace(".", ",")
            if (datafile is not None) and (not os.path.isfile(datafile)):
                print("File " + datafile + " does not exist")
                datafile = None
            if datafile is None:
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
        allMeans.append((mins, maxs, means, speeds, speedDevergations))
        allConf[0].append(confs)
        legend.append(hashNames[hash])
        areaLegend.append(hashNames[hash] + " min/max")
        speedLegend.append(hashNames[hash] + " time")
    xticks = []
    for copy in copies:
        xticks.append(str(copy))

    result = HashData()
    result = FairnessData()
    result.allMeans = allMeans
    result.allConf = allConf
    result.legend = legend
    result.areaLegend = areaLegend
    result.xticks = xticks
    result.intervallLegend = "conf. Int. 95%"
    result.speedLegend = speedLegend
    return result

def calc_adaptivity_data(movementfiles, filename, disks=None, copies=None):
    if copies == None:
        copies = list(movementfiles.keys())
        copies.sort()
    if len(copies) < 1:
        print("draw_ifmg_fairness: Got empty datafiles")
        return Noneds
    if disks == None:
        disks = list(movementfiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print ("draw_img_fairness: Got datafiles without diskentries")
        return None
    allData = []
    legend = []
    area1Legend = []
    area2Legend = []
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
    first = True
    preFirstLines=[]
    fieldLength = 0
    for copy in copies:
        copyMovementfiles = movementfiles[copy]
        copyOnNew = []
        copyWithoutRange = []
        copyWithRange = []
        for disk in disks:
            movementfile = copyMovementfiles[disk]
            fields = [0] * 7
            fields[0] = disk
            fields[1] = copy
            movementdata = None
            if (movementfile != None) and (not os.path.isfile(movementfile)):
                print("File " + movementfile + " does not exist")
                movementfile = None
            if movementfile == None:
                copyOnNew.append(0)
                copyWithRange.append(0)
                copyWithoutRange.append(0)
                fields[2] = ""
                fields[3] = ""
                fields[4] = ""
                fields[5] = ""
                fields[6] = ""
            else:
                movementdata = DistributorData.read_movement_data(movementfile)
                if first:
                   titels = ["Added Disks", "Copy" , "usedCapacity", "toMoveCapacity", "FacOnNew", "FacWithoutOrder", "FacWithOrder"] + movementdata[0]
                   csvFile.writerow(titels)
                   fieldLength = len(movementdata[0])
                   for line in preFirstLines:
                       writeline = line + [0] * fieldLength
                       csvFile.writerow(writeline)
                   preFirstLines = None
                   first = False

                oldCapacity = int(movementdata[1][2])
                newCapacity = int(movementdata[1][3])
                addedCapacity = int(movementdata[1][4])
                movedWithOrder = int(movementdata[1][7])
                movedWithoutOrder = int(movementdata[1][8])
                onNew = int(movementdata[1][10])

                usedCapacity = int(oldCapacity) * copy / 2
                toMoveCapacity = float(usedCapacity) * float(addedCapacity) / float(newCapacity);
                copyOnNew.append(float(onNew) / toMoveCapacity)
                copyWithoutRange.append((movedWithoutOrder - onNew)/toMoveCapacity)
                copyWithRange.append((movedWithOrder - movedWithoutOrder)/toMoveCapacity)
                fields[2] = usedCapacity
                fields[3] = str(toMoveCapacity).replace(".", ",")
                fields[4] = str(float(onNew) / toMoveCapacity).replace(".", ",")
                fields[5] = str(movedWithoutOrder/toMoveCapacity).replace(".", ",")
                fields[6] = str(movedWithOrder/toMoveCapacity).replace(".", ",")
            if csvFile is not None:
                if first:
                    preFirstLines.append(fields)
                else:
                    if movementdata is None:
                        writeline = fields + [0] * fieldLength
                        csvFile.writerow(writeline)
                    else:
                        writeline = fields + movementdata[1]
                        csvFile.writerow(writeline)
        allData.append((copyOnNew, copyWithoutRange, copyWithRange))
        if copy == 1:
            legend.append("1 copy")
            area1Legend.append("1 copy dynamic order")
            area2Legend.append("1 copy static order")
        else:
            legend.append(str(copy) + " copies")
            area1Legend.append(str(copy) + " copies dynamic order")
            area2Legend.append(str(copy) + " copies static order")
    xticks = []
    for disk in disks:
        xticks.append(str(disk))

    result = AdaptivityData()
    result.allData = allData
    result.xticks = xticks
    result.legend = legend
    result.area1Legend = area1Legend
    result.area2Legend = area2Legend
    return result

def calc_file_memtime_data(speedfiles, big, filename=None, disks=None, copies=None):
    if copies == None:
        copies = list(speedfiles.keys())
        copies.sort()
    if len(copies) < 1:
        print ("calc_file_memtime_data got len(copies) = 0")
        return None
    if disks == None:
        disks = list(speedfiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print ("calc_file_memtime_data got len(disks) = 0")
        return None
    #speedLegend = []
    #allData = []
    csvFile = None
    if filename is not None:
        csvFile = csv.writer(open(filename + ".csv", "w"), delimiter=";")
        fields = ("copies", "disks", "all time File Dist", "time File Dist", "dev File Dist", "all time Block Create", "time Block Create", "dev Block Create", "all time File Place", "time File Place", "dev File Place", "all time Block Place", "time Block Place", "dev Block Place", "all time Whole File", "time Whole File", "dev Whole File")
        csvFile.writerow(fields)
    for copy in copies:
        copySpeedFiles = speedfiles[copy]
        for disk in disks:
            speedfile = copySpeedFiles[disk]
            fields = [0] * 17
            fields[0] = copy
            fields[1] = disk
            if (speedfile is not None) and (not os.path.isfile(speedfile)):
                print("File " + speedfile + " does not exist")
                speedfile = None
            if speedfile is None:
                for i in xrange(2,17):
                    fields[i] = ""
            else:
                filenum = 1000000 / 4
                if big:
                    filenum = 1000000 / 5000
                speedData = DistributorData.FileRuntimeData(speedfile, filenum)

                i = 2

                val = speedData.get_sum_file_dist_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_average_file_dist_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_std_dev_file_dist_average_time()
                fields[i] = str(val).replace(".", ",")
                i += 1

                val = speedData.get_sum_block_create_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_average_block_create_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_std_dev_block_create_average_time()
                fields[i] = str(val).replace(".", ",")
                i += 1

                val = speedData.get_sum_place_blocks_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_average_place_blocks_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_std_dev_place_blocks_average_time()
                fields[i] = str(val).replace(".", ",")
                i += 1

                val = speedData.get_sum_place_block_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_average_place_block_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_std_dev_place_block_average_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                
                val = speedData.get_sum_place_file_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_average_place_file_time()
                fields[i] = str(val).replace(".", ",")
                i += 1
                val = speedData.get_std_dev_place_file_average_time()
                fields[i] = str(val).replace(".", ",")
                i += 1


            if csvFile is not None:
                csvFile.writerow(fields)
                

def calc_memtime_data(memfiles, speedfiles, filename=None, disks=None, copies=None, vmpeak=True):
    if copies == None:
        copies = list(memfiles.keys())
        copies.sort()
    if len(copies) < 1:
        print("draw_img_fairness: Got empty datafiles")
        return None
    if disks == None:
        disks = list(memfiles[copies[0]].keys())
        disks.sort()
    if len(disks) < 1:
        print("draw_img_fairness: Got datafiles without diskentries")
        return None
    allMems = []
    allSpeeds = []
    legend = []
    speedLegend = []
    peakLegend = []
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
                print("File " + speedfile + " does not exist")
                speedfile = None
            if (memfile is not None) and (not os.path.isfile(memfile)):
                print("File " + memfile + " does not exist")
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
                if (vmpeak):
                    maxs.append(float(memdata[1] - memdata[0]) / float(1024))
                else:
                    maxs.append(float(0))
                fields[2] = str(memdata[0]).replace(".", ",")
                fields[3] = str(memdata[1]).replace(".", ",")
            if csvFile is not None:
                csvFile.writerow(fields)
        allMems.append((mins, maxs, means, speeds, speedDevergations))
        if copy == 1:
            legend.append("1 copy")
            speedLegend.append("1 copy placement time")
            peakLegend.append("1 copy peak")
        else:
            legend.append(str(copy) + " copies mem")
            speedLegend.append(str(copy) + " copies placement time")
            peakLegend.append(str(copy) + " copies peak")
    xticks = []
    for disk in disks:
        xticks.append(str(disk))

    result = MemtimeData()
    result.allMems = allMems
    result.xticks = xticks
    result.legend = legend
    result.speedLegend = speedLegend
    result.peakLegend = peakLegend
    return result
