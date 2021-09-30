#! /usr/bin/python

from xml.dom.minidom import getDOMImplementation
import os
import stat
import subprocess
#import sqlite3

__author__ = "fermat"
__date__ = "$09.04.2010 13:52:04$"

RedundantShare = "RedundantShare"
FastRedundantShare = "FastRedundantShare"
Share = "Share"
NearestNeighbour = "NearestNeighbour"
CRUSH = "CRUSH"
RUSHp = "RUSHp"
RUSHt = "RUSHt"
RUSHr = "RUSHr"


create_prefix = "Create_"
distribute_prefix = "Run_"
direct_run_prefix = "CreateAndRun_"
compare_prefix = "Compare_"

skript_postfix = ".run.sh"
stderr_postfix = ".run.out.sterr"
stdout_postfix = ".run.out.std"
mem_postfix = ".run.out.mem"
memdata_postfix =".run.out.memdata"
dist_postfix = ".dist"
dist_result_postfix = ".csv"
dist_file_result_postfix = ".file.csv"
disk_postfix = ".disks"

adap_add_disks = (1, 2, 3, 5, 7, 11, 13)

stdtasks = os.environ.get('TASKS')
if stdtasks is None:
    stdtasks = 8
else:
    stdtasks = int(stdtasks)

class Disk:
    def __init__(self, id, capacity):
        self.id = id
        self.capacity = capacity
        
    def get_element_name(self):
        return "Disk"
    
    def to_XML(self, domDoc):
        """
        Convert the Disk to an XML-Element
        """
        element = domDoc.createElement(self.get_element_name())
        element.setAttribute("capacity", str(self.capacity))
        element.setAttribute("id", str(self.id))
        return element

def create_distributor_skript(distributor, diskFile, copies = 1, threads = stdtasks, baseMessage = None, extraOps=None, extraDistOps=None, extraName=None):
    """
    Create Skript to create a new Distributor. The function returns the basename of the skript and the files, that will be created by the skript.
    """
    description = distributor + "_" + str(copies) + "K_" + str(threads) + "T-" + diskFile + "-"
    if (baseMessage is None) or (baseMessage == ""):
        description = description + "0";
    else:
        description = description + baseMessage;
    if extraName is not None:
        description = description + extraName;
    filebase = create_prefix + description;
    file = open(filebase + skript_postfix, "w");
    file.write("#!/bin/bash\n")
    file.write("echo starting " + filebase + "\n")
    file.write("touch RUNNING-CREATE-" + filebase + "\n")
    file.write("CPPDistTest -dt " + distributor + " -Tn " + str(threads) + " -Dif " + diskFile + " -kn " + str(copies) + " -dof " + description + dist_postfix + " -of /dev/null")
    if (baseMessage is not None) and (baseMessage != ""):
        file.write(" -bm " + baseMessage)
    if (extraOps is not None) and (extraOps != ""):
        file.write(" " + extraOps)
    if (extraDistOps is not None) and (extraDistOps != ""):
        file.write(" -dp " + extraDistOps)
    file.write(" > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    file.write("pid=$!\n")
    file.write("state=0\n")
    file.write("rm " + filebase + mem_postfix + "\n")
    file.write("touch " + filebase + mem_postfix + "\n")
    file.write('while [ ${state} == "0" ] ; do\n')
    file.write("        ps -p ${pid} o rss h >> " + filebase + mem_postfix + "\n")
    file.write("        state=$?\n")
    file.write("        sleep 1\n")
    file.write("done\n")
    file.write("rm RUNNING-CREATE-" + filebase + "\n")
    file.write("echo succeeded " + filebase + "\n")
    file.close()
    os.chmod(filebase + skript_postfix, stat.S_IRWXU);
    return description

def create_run_skript(distribution, extentsNum, threads = stdtasks, baseMessage = None):
    """
    create Skript to run a Distributor
    """
    if (baseMessage is None) or (baseMessage == ""):
        filebase = distribute_prefix + distribution + "-"
    else:
        filebase = distribute_prefix + distribution + "-" + baseMessage

    file = open(filebase + skript_postfix, "w")
    file = open(filebase + skript_postfix, "w");
    file.write("#!/bin/bash\n")
    file.write("echo starting " + filebase + "\n")
    file.write("touch RUNNING-RUN-" + filebase + "\n")
    if (baseMessage is None) or (baseMessage == ""):
        file.write("CPPDistTest -dif " + distribution + dist_postfix + " -Tn " + str(threads) + " -of " + distribution + dist_result_postfix + " -en " + str(extentsNum) + " > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    else:
        file.write("CPPDistTest -dif " + distribution + dist_postfix + " -Tn " + str(threads) + " -of " + distribution + dist_result_postfix + " -en " + str(extentsNum) + " -bm " + baseMessage + " > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    file.write("pid=$!\n")
    file.write("state=0\n")
    file.write("rm " + filebase + mem_postfix + "\n")
    file.write("touch " + filebase + mem_postfix + "\n")
    file.write('while [ ${state} == "0" ] ; do\n')
    file.write("        ps -p ${pid} o rss h >> " + filebase + mem_postfix + "\n")
    file.write("        state=$?\n")
    file.write("        sleep 1\n")
    file.write("done\n")
    file.write("rm RUNNING-RUN-" + filebase + "\n")
    file.write("echo succeeded " + filebase + "\n")
    file.close()
    os.chmod(filebase + skript_postfix, stat.S_IRWXU);
    return filebase

def create_compare_skript(distributionFirst, distributionSecond, extentsNum, threads = stdtasks):
    filebase = compare_prefix + distributionSecond
    file = open(distributionSecond + "_distlist", "w")
    file.write(distributionFirst + dist_postfix + "\n" + distributionSecond + dist_postfix + "\n")
    file.close()
    file = open(filebase + skript_postfix, "w");
    file.write("#!/bin/bash\n")
    file.write("echo starting " + filebase + "\n")
    file.write("touch RUNNING-COMP-" + filebase + "\n")
    file.write("CPPDistCompare -dlf " + distributionSecond + "_distlist" + " -Tn " + str(threads) + " -odf " + distributionSecond + dist_result_postfix +" -ocf " + distributionSecond + "_movements.csv" +" -en " + str(extentsNum) + " > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    file.write("pid=$!\n")
    file.write("state=0\n")
    file.write("rm " + filebase + mem_postfix + "\n")
    file.write("touch " + filebase + mem_postfix + "\n")
    file.write('while [ ${state} == "0" ] ; do\n')
    file.write("        ps -p ${pid} o rss h >> " + filebase + mem_postfix + "\n")
    file.write("        state=$?\n")
    file.write("        sleep 1\n")
    file.write("done\n")
    file.write("rm RUNNING-COMP-" + filebase + "\n")
    file.write("echo succeeded " + filebase + "\n")
    file.close()
    os.chmod(filebase + skript_postfix, stat.S_IRWXU);
    return filebase
    pass

def create_direct_run_skript(distributor, diskFile, extentsNum, copies = 1, threads = stdtasks, baseMessage = None, extraOps=None, extraDistOps=None, extraName=None):
    if (baseMessage is None) or (baseMessage == ""):
        description = distributor + "_" + str(copies) + "K_" + str(threads) + "T-" + diskFile + "-0";
    else:
        description = distributor + "_" + str(copies) + "K_" + str(threads) + "T-" + diskFile + "-" + baseMessage;
    if extraName is not None:
        description = description + "-" + extraName
    filebase = direct_run_prefix + description;
    file = open(filebase + skript_postfix, "w");
    file.write("#!/bin/bash\n")
    file.write("echo starting " + filebase + "\n")
    file.write("touch RUNNING-DR-" + filebase + "\n")
    file.write("CPPDistTest -dt " + distributor + " -Tn " + str(threads) + " -Dif " + diskFile)
    file.write(" -kn " + str(copies) + " -of " + filebase + dist_result_postfix + " -en " + str(extentsNum))
    file.write(" -Mof " + filebase + memdata_postfix)
    if (baseMessage is not None) and (baseMessage != ""):
        file.write(" -bm " + baseMessage)
    if (extraOps is not None) and (extraOps != ""):
        file.write(" " + extraOps)
    if (extraDistOps is not None) and (extraDistOps != ""):
        file.write(" -dp " + extraDistOps)
    file.write(" > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    file.write("pid=$!\n")
    file.write("state=0\n")
    file.write("rm " + filebase + mem_postfix + "\n")
    file.write("touch " + filebase + mem_postfix + "\n")
    file.write('while [ ${state} == "0" ] ; do\n')
    file.write("        ps -p ${pid} o rss h >> " + filebase + mem_postfix + "\n")
    file.write("        state=$?\n")
    file.write("        sleep 1\n")
    file.write("done\n")
    file.write("rm RUNNING-DR-" + filebase + "\n")
    file.write("echo succeeded " + filebase + "\n")
    file.close()
    os.chmod(filebase + skript_postfix, stat.S_IRWXU)
    return description

def create_direct_file_run_skript(mainDistributor, fileDistributor, diskFile, filesNum, blocksPerFile, disksPerFile, copies, threads=stdtasks, baseMessage=None, extraOps=None, extraDistOps=None, extraName=None):
    description = mainDistributor + "_" + str(disksPerFile) + "K-" + fileDistributor + "_" + str(copies) + "K-" + diskFile
    if extraName is not None:
        description = description + extraName
    filebase = direct_run_prefix + description
    file = open(filebase + skript_postfix, "w")
    file.write("#!/bin/bash\n")
    file.write("echo starting " + filebase + "\n")
    file.write("touch RUNNING-DR-" + filebase + "\n")
    file.write("fileDistTester -fdt " + mainDistributor + " -bdt " + fileDistributor + " -Dif " + diskFile)
    file.write(" -Cn " + str(disksPerFile) + " -BCn " + str(copies))
    file.write(" -Fn " + str(filesNum) + " -Bn " + str(blocksPerFile))
    file.write(" -Tn " + str(threads))
    file.write(" -of " + filebase + dist_result_postfix)
    file.write(" -fof " + filebase + dist_file_refult_postfix)
    if extraOps is not None:
        file.write(" " + extraOps)
    file.write(" > " + filebase + stdout_postfix + " 2> " + filebase + stderr_postfix + " &\n")
    file.write("pid=$!\n")
    file.write("state=0\n")
    file.write("rm " + filebase + mem_postfix + "\n")
    file.write("touch " + filebase + mem_postfix + "\n")
    file.write('while [ ${state} == "0" ] ; do\n')
    file.write("        ps -p ${pid} o rss h >> " + filebase + mem_postfix + "\n")
    file.write("        state=$?\n")
    file.write("        sleep 1\n")
    file.write("done\n")
    file.write("rm RUNNING-DR-" + filebase + "\n")
    file.write("echo succeeded " + filebase + "\n")
    file.close()
    os.chmod(filebase + skript_postfix, stat.S_IRWXU)
    return description


def run_skript(skript):
    skriptStart = ["./" + skript]
    print("Will start " + skriptStart[0])
    subprocess.call(skriptStart, shell=True)

def create_disk_file(disks, filebase):
    impl = getDOMImplementation()
    doc = impl.createDocument(None, "Disks", None);
    top_element = doc.documentElement

    for disk in disks:
        top_element.appendChild(disk.to_XML(doc))

    file = open(filebase + disk_postfix, "w");
    doc.writexml(file, "", "  ", "\n")
    file.close()
    return filebase

def create_homogeneous_disk_file(diskNum, diskCapacity = 500000, firstID = 0):
    """
    creates a File with some homogeneous disks of the given Capacity. Returns the prefix of the created Diskfile.
    """
    disks = []

    for i in xrange(firstID, diskNum):
        disks.append(Disk(i, diskCapacity))
        
    filebase = str(diskNum) + "D_hom"
    create_disk_file(disks, filebase)
    return filebase

def create_heterogeneous_disk_file(diskNum, firstDiskCapacity = 500000, firstID = 0, firstDisksizedNum = 128, evenDiskSizedNum = 128, disksGrowth = 100000):
    """
    creates heterogeneous disks following the sheme described in my Mail
    """
    restDiskNum = diskNum
    placeNum = firstDisksizedNum
    placeSize = firstDiskCapacity
    diskdata = []
    if restDiskNum < placeNum:
        placeNum = restDiskNum
    diskdata.append((placeNum, placeSize))
    restDiskNum -= placeNum

    while (restDiskNum > 0):
        if restDiskNum < evenDiskSizedNum:
            placeNum = restDiskNum
        else:
            placeNum = evenDiskSizedNum
        placeSize += disksGrowth
        diskdata.append((placeNum, placeSize))
        restDiskNum -= placeNum

    return create_heterogeneous_disk_file_given_values(diskdata, firstID)

def create_heterogeneous_disk_file_testcase(diskNum, firstDiskCapacity = 500000, firstID = 0, firstDisksizedNum = 128, evenDiskSizedNum = 128, disksGrowth = 1.5):
    """
    creates heterogeneous disks following the sheme described in my Mail
    """
    restDiskNum = diskNum
    placeNum = firstDisksizedNum
    placeSize = firstDiskCapacity
    diskdata = []
    if restDiskNum < placeNum:
        placeNum = restDiskNum
    diskdata.append((placeNum, placeSize))
    restDiskNum -= placeNum

    while (restDiskNum > 0):
        if restDiskNum < evenDiskSizedNum:
            placeNum = restDiskNum
        else:
            placeNum = evenDiskSizedNum
        placeSize *= disksGrowth
        diskdata.append((placeNum, placeSize))
        restDiskNum -= placeNum

    return create_heterogeneous_disk_file_given_values(diskdata, firstID)

def create_heterogeneous_disk_file_given_values(diskCapacities, firstID = 0):
    """
    creates heterogeneous disks following the sheme described in my Mail
    """
    id = firstID
    capacity = 0
    disks=[]

    for i in xrange(len(diskCapacities)):
        numDisks = diskCapacities[i][0]
        diskSize = diskCapacities[i][1]
        capacity += numDisks * diskSize
        for j in xrange(numDisks):
            disks.append(Disk(id, diskSize))
            id += 1

    filebase = str(id - firstID) + "D_het_" + str(firstID)
    create_disk_file(disks, filebase)
    return filebase, capacity

def run_file_hom(mainDistributor, fileDistributor, blocksPerFile=4, disksPerFileCopy=2, disks=(3,14), copies=(0,4), usageFactor=20, threads=stdtasks, baseMessage=None, extraOps=None, extraDistOps=None, extraName=None):
    diskSize = 500000
    for i in xrange(disks[0], disks[1]):
        disksNum = 2**i
        disk_base = create_homogeneous_disk_file(disksNum, diskSize)
        extents = disksNum * diskSize / usageFactor
        filesNum = extents / blocksPerFile
        for j in xrange(copies[0], copies[1]):
            copiesNum = 2 ** j
            disksPerFile = disksPerFileCopy * copiesNum
            if disksPerFile > disksNum:
                disksPerFile = disksNum
            run_base = create_direct_file_run_skript(mainDistributor, fileDistributor, disk_base + disk_postfix, filesNum, blocksPerFile, disksPerFile, copiesNum, threads, baseMessage, extraOps, extraDistOps, extraName)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_file_mem_time_hom(mainDistributor, fileDistributor, blocksPerFile=4, disksPerFileCopy=2, disks=(3,14), copies=(0,4), threads=1, baseMessage=None, extraOps=None, extraDistOps=None, extraName=None):
    diskSize = 500000
    for i in xrange(disks[0], disks[1]):
        disksNum = 2**i
        disk_base = create_homogeneous_disk_file(disksNum, diskSize)
        extents = 1000000
        filesNum = extents / blocksPerFile
        for j in xrange(copies[0], copies[1]):
            copiesNum = 2 ** j
            disksPerFile = disksPerFileCopy * copiesNum
            if disksPerFile > disksNum:
                disksPerFile = disksNum
            run_base = create_direct_file_run_skript(mainDistributor, fileDistributor, disk_base + disk_postfix, filesNum, blocksPerFile, disksPerFile, copiesNum, threads, baseMessage, extraOps, extraDistOps, extraName)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_hom(distri, disks=(3,14), copies=(0,4), direct=True, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2, extentsNum=None):
    diskSize = 500000
    for i in xrange (disks[0],disks[1]):
        disks = 2**i
        if extentsNum is None:
            extentsNum = 2**(i) * diskSize / usageFactor
        disk_base = create_homogeneous_disk_file(disks, diskSize)
        for j in xrange(copies[0],copies[1]):
            copy = 2**j
            if direct:
                run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
                run_skript(direct_run_prefix + run_base + skript_postfix)
            else:
                create_base = create_distributor_skript(distri, disk_base + disk_postfix, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
                run_skript(create_prefix + create_base + skript_postfix)
                run_base = create_run_skript(create_base, extentsNum, 8)
                run_skript(run_base + skript_postfix)

def run_het(distri, steps=(0,10), copies=(0,4), direct=True, disksPerStep=128, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2):
    for i in xrange(steps[0], steps[1]):
        disks = disksPerStep * (i + 1)
        disk_base, capacity = create_heterogeneous_disk_file_testcase(disks, firstDisksizedNum = disksPerStep, evenDiskSizedNum = disksPerStep)
        for j in xrange(copies[0], copies[1]):
            copy = 2**j
            extentsNum = capacity / usageFactor
            if direct:
                run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
                run_skript(direct_run_prefix + run_base + skript_postfix)
            else:
                create_base = create_distributor_skript(distri, disk_base + disk_postfix, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
                run_skript(create_prefix + create_base + skript_postfix)
                run_base = create_run_skript(create_base, extentsNum, 8)
                run_skript(run_base + skript_postfix)

def run_maxhet(distri, steps=(0,10), copies=(0,4), extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2):
    diskSizes = []
    diskSizes.append((2,600000))
    diskSizes.append((12,100000))
    disk_base, capacity = create_heterogeneous_disk_file_given_values(diskSizes)
    extentsNum = capacity / 4
    copy = 4
    run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)

def run_compare(distri, steps=(0,7), copies=(0,4), homogen=True, diskNum=128, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2):
    diskSize = 500000
    diskcapacities = []
    diskcapacities.append((diskNum, diskSize))
    disk_base, capacity = create_heterogeneous_disk_file_given_values(diskcapacities)
    extents = capacity / usageFactor
    for j in xrange(copies[0], copies[1]):
        my_copies = 2**j;
        create_base =create_distributor_skript(distri, disk_base + disk_postfix, my_copies, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
        run_skript(create_prefix + create_base + skript_postfix)
        for i in xrange(steps[0], steps[1]):
            addDisks = adap_add_disks[i]
            tDisccapacities = []
            tDisccapacities.append(diskcapacities[0])
            if homogen:
                tDisccapacities.append((addDisks, diskSize))
            else:
                tDisccapacities.append((addDisks, diskSize * 3 / 2))
            tDisk_base, tCapacity = create_heterogeneous_disk_file_given_values(tDisccapacities)
            tCreate_base =create_distributor_skript(distri, tDisk_base + disk_postfix, my_copies, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=extraName)
            run_skript(create_prefix + tCreate_base + skript_postfix)
            comp_base = create_compare_skript(create_base, tCreate_base, extents, tasks)
            run_skript(comp_base + skript_postfix)

def run_hash(distri, hashs=(None, 4002, 4003, 4004, 2, 11, 8, 9, 10, 301, 1, 6, 305), copies=(0,4), homogen=True, disks=64, extraOps=None, extraDistOps=None, extraName=None, tasks=1, usageFactor=2):
    diskSize = 500000
    mtExtraOps = "-st"
    if extraOps is not None:
        mtExtraOps = mtExtraOps + " " + extraOps
    extentsNum = diskSize * disks / usageFactor;
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
    else:
        diskdata=[]
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, diskSize * 3 / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
        extentsNum = capacity / usageFactor
    for i in xrange(copies[0], copies[1]):
        my_copies = 2**i
        for hash in hashs:
            if (hash is None) or (hash == 0):
                myExtraOps=mtExtraOps
                if extraName is None:
                    myExtraName="own"
                else:
                    myExtraName=extraName + "own"
            elif hash > 4000:
                useHash = hash - 4000
                myExtraOps=mtExtraOps + " -icrypt " + str(useHash)
                if extraName is None:
                    myExtraName="own" + str(useHash)
                else:
                    myExtraName=extraName + "own" + str(useHash)
            else:
                myExtraOps=mtExtraOps + " -gcrypt " + str(hash)
                if extraName is None:
                    myExtraName="gcrypt" + str(hash)
                else:
                    myExtraName=extraName + "gcrypt" + str(hash)
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, my_copies, tasks, extraOps=myExtraOps, extraName=myExtraName, extraDistOps=extraDistOps)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_hom_mem_time_Test(distri, disks=(3,14), copies=(0,4), extraOps=None, extraDistOps=None, extraName=None, tasks=1):
    diskSize = 500000
    mtExtraOps = "-st"
    if extraOps is not None:
        mtExtraOps = mtExtraOps + " " + extraOps
    for i in xrange (disks[0],disks[1]):
        disks = 2**i
        extentsNum = 1000000
        disk_base = create_homogeneous_disk_file(disks, diskSize)
        for j in xrange(copies[0],copies[1]):
            copy = 2**j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraOps=mtExtraOps, extraDistOps=extraDistOps, extraName=extraName)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_het_mem_time_Test(distri, steps=(0,10), copies=(0,4), disksPerStep=128, extraOps=None, extraDistOps=None, extraName=None, tasks=1):
    extentsNum = 1000000
    mtExtraOps = "-st"
    if extraOps is not None:
        mtExtraOps = mtExtraOps + " " + extraOps
    for i in xrange(steps[0], steps[1]):
        disks = disksPerStep * (i + 1)
        disk_base, capacity = create_heterogeneous_disk_file_testcase(disks, firstDisksizedNum = disksPerStep, evenDiskSizedNum = disksPerStep)
        for j in xrange(copies[0], copies[1]):
            copy = 2**j
            #extentsNum = capacity / 2
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraOps=mtExtraOps, extraDistOps=extraDistOps, extraName=extraName)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_extent_test(distri, homogen=True, extents=(0,5), copies=(0,4), disks=64, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks):
    diskSize=500000
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
    else:
        diskdata=[]
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, diskSize * 3 / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
    for i in xrange(copies[0], copies[1]):
        copy = 2**i
        for j in xrange(extents[0], extents[1]):
            extentsNum = 25 * (10 ** j)
            eExtraName=str(extentsNum)+"Extents"
            if extraName is not None:
                eExtraName = eExtraName + "-" + extraName
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum * disks, copy, tasks, extraOps=extraOps, extraDistOps=extraDistOps, extraName=eExtraName)
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_nn_copy_test(range=(0,20)):
    distri = NearestNeighbour
    tasks = 1
    disks = 64
    diskSize = 500000
    extentsNum = disks * diskSize / 2
    disk_base = create_homogeneous_disk_file(disks, diskSize)
    for i in xrange(range[0],range[1]):
        diskCopy = 100 * (i + 1)
        for j in xrange(0,4):
            copy = 2**j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-nc " + str(diskCopy), extraName="-" + str(diskCopy) + "DiskCopy")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_share_sf_test():
    distri = Share
    tasks = 1
    disks = 64
    diskSize = 500000
    extentsNum = disks * diskSize / 2
    disk_base = create_homogeneous_disk_file(disks, diskSize)
    for i in xrange(0,7):
        sf = 2 ** i
        for j in xrange(0,4):
            copy = 2**j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-ssf " + str(sf), extraName="-" + str(sf) + "sf")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_share_sf_test_het():
    distri = Share
    tasks = 1
    diskdata = []
    diskdata.append((32, 500000))
    diskdata.append((32, 750000))
    disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
    #disks = 64
    #diskSize = 500000
    extentsNum = capacity / 2
    #disk_base = create_homogeneous_disk_file(disks, diskSize)
    for i in xrange(0,7):
        sf = 2 ** i
        for j in xrange(0,4):
            copy = 2**j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-sf " + str(sf), extraName="-" + str(sf) + "sf")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_share_sf_memtime(homogen=True, disks=256, steps=(0,7), copies=(0,4)):
    distri = Share
    tasks = 1
    diskSize = 500000
    extentsNum = 1000000
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
    else:
        diskdata = []
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, (diskSize * 3) / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
    for i in xrange(steps[0], steps[1]):
        sf = 5 * (i + 1)
        for j in xrange(copies[0], copies[1]):
            copy = 2 ** j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-sf " + str(sf), extraName="-" + str(sf) + "sf", extraOps="-st")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_share_sf_fair(homogen=True, disks=256, steps=(0,7), copies=(0,4), tasks=stdtasks):
    distri = Share
    diskSize = 500000
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
        extentsNum = disks * diskSize / 20
    else:
        diskdata = []
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, (diskSize * 3) / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
        extentsNum = capacity / 20
    for i in xrange(steps[0], steps[1]):
        sf = 5 * (i + 1)
        for j in xrange(copies[0], copies[1]):
            copy = 2 ** j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-sf " + str(sf), extraName="-" + str(sf) + "sf", extraOps="-st")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_nn_copy_memtime(homogen=True, disks=256, steps=(0,10), copies=(0,4)):
    distri = NearestNeighbour
    tasks = 1
    diskSize = 500000
    extentsNum = 1000000
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
    else:
        diskdata = []
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, (diskSize * 3) / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
    for i in xrange(steps[0], steps[1]):
        dcopies = 100 * 3 * (i + 1)
        for j in xrange(copies[0], copies[1]):
            copy = 2 ** j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-nc " + str(dcopies), extraName="-" + str(dcopies) + "DiskCopy", extraOps="-st")
            run_skript(direct_run_prefix + run_base + skript_postfix)

def run_nn_copy_fair(homogen=True, disks=256, steps=(0,10), copies=(0,4), tasks=stdtasks):
    distri = NearestNeighbour
    diskSize = 500000
    if homogen:
        disk_base = create_homogeneous_disk_file(disks, diskSize)
        extentsNum = disks * diskSize / 20
    else:
        diskdata = []
        diskdata.append((disks / 2, diskSize))
        diskdata.append((disks / 2, (diskSize * 3) / 2))
        disk_base, capacity = create_heterogeneous_disk_file_given_values(diskdata)
        extentsNum = capacity / 20
    for i in xrange(steps[0], steps[1]):
        dcopies = 100 * 3 * (i + 1)
        for j in xrange(copies[0], copies[1]):
            copy = 2 ** j
            run_base = create_direct_run_skript(distri, disk_base + disk_postfix, extentsNum, copy, tasks, extraDistOps="-nc " + str(dcopies), extraName="-" + str(dcopies) + "DiskCopy", extraOps="-st")
            run_skript(direct_run_prefix + run_base + skript_postfix)

if __name__ == "__main__":
    #def run_hom(distri, disks=(3,14), copies=(0,4), direct=True, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2, extentsNum=None):
    #def run_het(distri, steps=(0,10), copies=(0,4), direct=True, disksPerStep=128, extraOps=None, extraDistOps=None, extraName=None, tasks=stdtasks, usageFactor=2):

    #run_hom(NearestNeighbour, usageFactor=2, disks=(13,14), copies=(0,1)) # 2011-03-21 13-48-20

    #run_hom(Share, usageFactor=2, disks=(3,5), copies=(0,1))
    #run_het(NearestNeighbour, usageFactor=2, steps=(8,10), copies=(0,1))
    #run_het(RedundantShare, usageFactor=2, steps=(7,8), copies=(2,3))
    #run_het(RedundantShare, usageFactor=2, steps=(7,8), copies=(3,4))
    #run_het(RedundantShare, usageFactor=2, steps=(8,9), copies=(1,2))
    #run_het(RedundantShare, usageFactor=2, steps=(8,9), copies=(2,3))
    #run_het(RedundantShare, usageFactor=2, steps=(8,9), copies=(3,4))
    #run_het(RedundantShare, usageFactor=2, steps=(9,10), copies=(3,4))

    #pass
    #Running in megabrain: (Extent_Het_RedundantShare_s9_c8)
    # done run_het(RedundantShare, usageFactor=20, steps=(9,10), copies=(3,4))
    # done run_het(Share, extraName="-5sf", extraDistOps="-sf 5", usageFactor=20, steps=(0,8))
    # done run_het(Share, usageFactor=20, extraDistOps="-nc 2800 -ssf 24", extraName="StaticStretchfactor", steps=(0,10))
    # done run_het(RedundantShare, usageFactor=20, steps=(9,10), copies=(2,3))
    # done run_het(RedundantShare, usageFactor=20, steps=(9,10), copies=(1,2))
    # done run_het(RedundantShare, usageFactor=20, steps=(8,9), copies=(2,3))
    # done run_het(RedundantShare, usageFactor=20, steps=(7,8), copies=(3,4))
    # done run_het(RedundantShare, usageFactor=20, steps=(7,8), copies=(0,1))
    #allExtentsNum = 8192 * 25000
    #stepExtentsNum = allExtentsNum / 64
    #run_hom(RedundantShare, disks=(13,14), copies=(3,4), extraOps="-fen " + str(0 * stepExtentsNum), extraName="-1", extentsNum=str(stepExtentsNum * 16))
    # done run_hom(RedundantShare, usageFactor=20, disks=(13,14), copies=(3,4))
    #run_compare(RedundantShare, extraDistOps="-decID", steps=(0,1), copies=(0,7), usageFactor=20)

    #TO RUN:
    #run_hom(RedundantShare, disks=(13,14), copies=(3,4), extraOps="-fen " + str(16 * stepExtentsNum), extraName="-1", extentsNum=str(stepExtentsNum * 16))
    #run_hom(RedundantShare, disks=(13,14), copies=(3,4), extraOps="-fen " + str(32 * stepExtentsNum), extraName="-1", extentsNum=str(stepExtentsNum * 16))
    #run_hom(RedundantShare, disks=(13,14), copies=(3,4), extraOps="-fen " + str(48 * stepExtentsNum), extraName="-1", extentsNum=str(stepExtentsNum * 16))

    

    #Running on bisgrid:
    # run_file_mem_time_hom(RedundantShare, RedundantShare, blocksPerFile=4, disksPerFileCopy=4) # 2011-01-11 15-20-29
    # run_file_mem_time_hom(RedundantShare, RedundantShare, blocksPerFile=5000, disksPerFileCopy=16) # 2011-01-11 15-20-53
    # run_file_hom(RedundantShare, RedundantShare, blocksPerFile=4, disksPerFileCopy=4) # 2011-01-11 15-21-18
    # run_file_hom(RedundantShare, RedundantShare, blocksPerFile=5000, disksPerFileCopy=16) # 2011-01-21 11-42-20

    #To Place:

    #for i in xrange(0,64):
    #    run_hom(RedundantShare, disks=(13,14), copies=(3,4), extraOps="-fen " + str(i * stepExtentsNum), extraName="-1", extentsNum=str(stepExtentsNum))

    # This servers are now under control of tonicwater testAdmin.py
    # Fairness for RS and NN
    #Running on sprite:
    #Running on drpepper:
    #Running on redbull:
    #Running on bitterlemon:
    #Running on bionade:
    
    #gingerale lend to meatz

    #run_hom_mem_time_Test(RUSHr)
    #run_het_mem_time_Test(RUSHr)

    # run_hom_mem_time_Test(RUSHt)
    # run_het_mem_time_Test(RUSHt)

    # run_hom(RUSHt, usageFactor=200)
    # run_hom(RUSHr, usageFactor=200)
    # run_het(RUSHt, usageFactor=200)
    # run_het(RUSHr, usageFactor=200)
    # run_compare(RUSHr, homogen=True)
    # run_compare(RUSHt, homogen=True)
    # run_compare(RUSHt, homogen=False)
    # run_compare(RUSHr, homogen=False)
    pass



    # One complete Run of memtime + hash for RUSHp + CRUSH on bisgrid:
    #run_hom_mem_time_Test(RUSHp)
    #run_het_mem_time_Test(RUSHp)
    #run_hom_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 1", extraName="-l1r1")
    #run_het_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 1", extraName="-l1r1")
    #run_hom_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 2", extraName="-l1r2")
    #run_het_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 2", extraName="-l1r2")
    #run_hom_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 3", extraName="-l1r3")
    #run_het_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 3", extraName="-l1r3")
    #run_hom_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 4", extraName="-l1r4")
    #run_het_mem_time_Test(CRUSH, extraDistOps="-layer domain 1 root 4", extraName="-l1r4")
    #run_hash(RUSHp, homogen=True, usageFactor=200)
    #run_hash(RUSHp, homogen=False, usageFactor=200)
    #run_hash(CRUSH, homogen=True, extraDistOps="-layer domain 1 root 1", extraName="-l1r1", usageFactor=200)
    #run_hash(CRUSH, homogen=False, extraDistOps="-layer domain 1 root 1", extraName="-l1r1", usageFactor=200)
    #run_hash(CRUSH, homogen=True, extraDistOps="-layer domain 1 root 2", extraName="-l1r2", usageFactor=200)
    #run_hash(CRUSH, homogen=False, extraDistOps="-layer domain 1 root 2", extraName="-l1r2", usageFactor=200)
    #run_hash(CRUSH, homogen=True, extraDistOps="-layer domain 1 root 3", extraName="-l1r3", usageFactor=200)
    #run_hash(CRUSH, homogen=False, extraDistOps="-layer domain 1 root 3", extraName="-l1r3", usageFactor=200)
    #run_hash(CRUSH, homogen=True, extraDistOps="-layer domain 1 root 4", extraName="-l1r4", usageFactor=200)
    #run_hash(CRUSH, homogen=False, extraDistOps="-layer domain 1 root 4", extraName="-l1r4", usageFactor=200)
    

    # fairness Tests for RUSHp + CRUSH (d = done, c = computing, 0 = to be placed)
    # d run_hom(RUSHp, usageFactor=200)
    # d run_het(RUSHp, usageFactor=200)
    # d run_hom(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 1", extraName="-l1r1")
    # d run_het(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 1", extraName="-l1r1")
    # d run_hom(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 2", extraName="-l1r2")
    # d run_het(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 2", extraName="-l1r2")
    # d run_hom(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 3", extraName="-l1r3")
    # d run_het(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 3", extraName="-l1r3")
    # d run_hom(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 4", extraName="-l1r4")
    # d run_het(CRUSH, usageFactor=200, extraDistOps="-ndom 8 -layer domain 1 root 4", extraName="-l1r4")

    # adaptivity Tests for RUSHp + CRUSH (d = done, c = computing, 0 = to be placed)
    # d run_compare(RUSHp, homogen=True)
    # d run_compare(RUSHp, homogen=False)
    # d run_compare(CRUSH, homogen=True, extraDistOps="-ndom 8 -layer domain 1 root 1", extraName="-l1r1")
    # d run_compare(CRUSH, homogen=False, extraDistOps="-ndom 8 -layer domain 1 root 1", extraName="-l1r1")
    # d run_compare(CRUSH, homogen=True, extraDistOps="-ndom 8 -layer domain 1 root 2", extraName="-l1r2")
    # d run_compare(CRUSH, homogen=False, extraDistOps="-ndom 8 -layer domain 1 root 2", extraName="-l1r2")
    # d run_compare(CRUSH, homogen=True, extraDistOps="-ndom 8 -layer domain 1 root 3", extraName="-l1r3")
    # d run_compare(CRUSH, homogen=False, extraDistOps="-ndom 8 -layer domain 1 root 3", extraName="-l1r3")
    # d run_compare(CRUSH, homogen=True, extraDistOps="-ndom 8 -layer domain 1 root 4", extraName="-l1r4")
    # d run_compare(CRUSH, homogen=False, extraDistOps="-ndom 8 -layer domain 1 root 4", extraName="-l1r4")

    # Results, that Santa Cruz should check:
    # * run_hom(CRUSH, usageFactor=200, extraDistOps="-layer domain 1 root 1", extraName="-l1r1")
    # * run_hom(CRUSH, usageFactor=200, extraDistOps="-layer domain 1 root 2", extraName="-l1r2")
    # * run_compare(RUSHp, homogen=True)
    # * run_compare(RUSHp, homogen=False)
    # * run_compare(CRUSH, homogen=True, extraDistOps="-layer domain 1 root 1", extraName="-l1r1")


    #Missing:
    #    Tests for different number copies of disks in Nearest Neighbour (how good can we get using 4 GB RAM?)
    #    Tests for different Stretchfactors in Share (how good can we get using 4 GB RAM?)
    #    run_het(RedundantShare, steps=(8,9), copies = (1,2))
    #    run_het(RedundantShare, steps=(9,10), copies = (3,4))

