#!/usr/bin/python

__author__="fermat"
__date__ ="$14.07.2010 10:36:10$"

import os.path
import os
from disAnalysis.imageFactory import draw_image_bar_line_with_points, generate_german_dis_yticklabels, DisColorChooser
from disAnalysis.analysis import calc_memtime_data, calc_file_memtime_data
from createDistributor import NearestNeighbour, Share, RedundantShare, FastRedundantShare, RUSHp, CRUSH, RUSHt, RUSHr
import matplotlib

matplotlib.rc('text', usetex=True)

imagePath = "../data/images/"
disImagePath = "../data/disImages/"

def draw_img_memtime(distri, homogen, extraName="", extraDirName="", tasks=1, hetDiskBase=128):
    path = "../data/mem_time/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "MemTime_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        memfiles = dict()
        speedFiles = dict()
        for i in range(0,4):
            copy = 2**i
            copyMems = dict()
            copySpeeds = dict()
            if homogen:
                for j in range(3,14):
                    disks = 2**j
                    copyMems[disks] = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-" + str(disks) + "D_hom.disks-0" + extraName + ".run.out.memdata"
                    copySpeeds[disks] = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-" + str(disks) + "D_hom.disks-0" + extraName + ".run.out.std"
            else:
                for j in range(1,11):
                    disks = hetDiskBase * j
                    copyMems[disks] = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-" + str(disks) + "D_het_0.disks-0" + extraName + ".run.out.memdata"
                    copySpeeds[disks] = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-" + str(disks) + "D_het_0.disks-0" + extraName + ".run.out.std"
            memfiles[copy] = copyMems
            speedFiles[copy] = copySpeeds
        xticks=[]
        if homogen:
            for i in range(3,14):
                #xticks.append("$2^{" + str(i) + "}$")
                xticks.append("$"+str(2**i)+"$")
        else:
            for i in range(1,11):
                #xticks.append("$" + str(hetDiskBase) + "\cdot " + str(i) +"$")
                xticks.append("$" + str(hetDiskBase * i) +"$")
        result = calc_memtime_data(memfiles, speedFiles, imagefile)
        imgRanges = [0.17, 0.3, 0.67, 0.6]
        draw_image_bar_line_with_points(result.allMems, imagefile, result.xticks, None, "Storage Systems", "memory (MB)", "time ($\mu s$)", imgRanges=imgRanges)
        if homogen:
            rotation=45
        else:
            rotation=45
        draw_image_bar_line_with_points(result.allMems, disImageFile, xticks, None, "Festplatten", "RAM (MByte)", "Zeit ($\mu s$)", imgRanges=imgRanges, colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        print("Changed:  " + imagefile + ".png")

def draw_img_memtime_share_sf(homogen, extraDirName="", tasks=1):
    distri = Share
    path = "../data/mem_time/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "MemTime_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        memfiles = dict()
        speedFiles = dict()
        for i in range(0,4):
            copy = 2**i
            copyMems = dict()
            copySpeeds = dict()
            for j in range(0,7):
                diskCopies = 5 * (j+1)
                cmf = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-256D_"
                if homogen:
                    cmf += "hom"
                else:
                    cmf += "het_0"
                cmf += ".disks-0--" + str(diskCopies) + "sf"
                csf = cmf + ".run.out.std"
                cmf += ".run.out.memdata"
                copyMems[diskCopies] = cmf
                copySpeeds[diskCopies] = csf
            memfiles[copy] = copyMems
            speedFiles[copy] = copySpeeds
        xticks=[]
        for i in range(0,7):
            sf = 5 * (i+1) * 8
            xticks.append("$" + str(sf) + "$")
        result = calc_memtime_data(memfiles, speedFiles, imagefile)
        imgRanges = [0.17, 0.3, 0.67, 0.6]
        draw_image_bar_line_with_points(result.allMems, imagefile, result.xticks, None, "Stretchfactor", "memory (MB)", "time ($\mu s$)", imgRanges=imgRanges)
        if homogen:
            rotation=0
        else:
            rotation=45
        draw_image_bar_line_with_points(result.allMems, disImageFile, xticks, None, "Dehnungsfaktor", "RAM (MByte)", "Zeit ($\mu s$)", imgRanges=imgRanges, colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        print("Changed:  " + imagefile + ".png")

def draw_img_memtime_nn_nc(homogen, extraDirName="", tasks=1):
    distri = NearestNeighbour
    path = "../data/mem_time/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "MemTime_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        memfiles = dict()
        speedFiles = dict()
        for i in range(0,4):
            copy = 2**i
            copyMems = dict()
            copySpeeds = dict()
            for j in range(0,10):
                diskCopies = 300 * (j+1)
                cmf = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-256D_"
                if homogen:
                    cmf += "hom"
                else:
                    cmf += "het_0"
                cmf += ".disks-0--" + str(diskCopies) + "DiskCopy"
                csf = cmf + ".run.out.std"
                cmf += ".run.out.memdata"
                copyMems[diskCopies] = cmf
                copySpeeds[diskCopies] = csf
            memfiles[copy] = copyMems
            speedFiles[copy] = copySpeeds
        xticks=[]
        for i in range(0,10):
            sf = 300 * (i+1) * 8
            xticks.append("$" + str(sf) + "$")
        result = calc_memtime_data(memfiles, speedFiles, imagefile)
        imgRanges = [0.17, 0.3, 0.67, 0.6]
        draw_image_bar_line_with_points(result.allMems, imagefile, result.xticks, None, "Placed Copies of each Disk", "memory (MB)", "time ($\mu s$)", imgRanges=imgRanges)
        if homogen:
            rotation=0
        else:
            rotation=45
        draw_image_bar_line_with_points(result.allMems, disImageFile, xticks, None, "Platzierte Kopien jeder Festplatte", "RAM (MByte)", "Zeit ($\mu s$)", imgRanges=imgRanges, colors=DisColorChooser(), ytickfct=generate_german_dis_yticklabels)
        print("Changed:  " + imagefile + ".png")

def draw_img_memtime_file(filedistri, blockdistri, homogen, big, tasks=1):
    path = "../data/mem_time/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += "File_"
    if big:
        path += "Big_"
    else:
        path += "Small_"
    path += filedistri + "_" + blockdistri +"/ausgaben/"

    f = "MemTime_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += "File_"
    if big:
        f += "Big_"
    else:
        f += "Small_"
    f += filedistri + "_" + blockdistri

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    disksPerFileCopy = 4
    if big:
        disksPerFileCopy = 16

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        speedFiles = dict()
        for i in range(0,4):
            copy = 2**i
            copySpeeds = dict()
            if homogen:
                for j in range(3,14):
                    disks = 2**j
                    disks_per_file = disksPerFileCopy * copy
                    if disks_per_file > disks:
                        disks_per_file = disks
                    copySpeeds[disks] = path + "CreateAndRun_" + filedistri + "_" + str(disks_per_file) + "K-" + blockdistri + "_" + str(copy) + "K-" + str(disks) + "D_hom.disks.run.out.std"
            else:
                # TODO: heterogeneous
                pass
            speedFiles[copy] = copySpeeds
        xticks=[]
        if homogen:
            for i in range(3,14):
                #xticks.append("$2^{" + str(i) + "}$")
                xticks.append("$"+str(2**i)+"$")
        else:
            for i in range(1,11):
                #xticks.append("$" + str(hetDiskBase) + "\cdot " + str(i) +"$")
                xticks.append("$" + str(hetDiskBase * i) +"$")
        result = calc_file_memtime_data(speedFiles, big, imagefile)
        #imgRanges = [0.17, 0.3, 0.67, 0.6]

        # TODO: Draw Image
        
        print("Changed:  " + imagefile + ".png")

if __name__ == "__main__":
    draw_img_memtime(NearestNeighbour, True)
    draw_img_memtime(Share, True)
    draw_img_memtime(Share, True, extraDirName="SF5", extraName="--5sf")
    draw_img_memtime(RedundantShare, True)
    draw_img_memtime(RedundantShare, True, extraDirName="Improved")
    draw_img_memtime(RUSHp, True)
    draw_img_memtime(RUSHt, True)
    draw_img_memtime(RUSHr, True)
    draw_img_memtime(CRUSH, True, extraDirName="l1r1", extraName="--l1r1")
    draw_img_memtime(CRUSH, True, extraDirName="l1r2", extraName="--l1r2")
    #draw_img(CRUSH, True, extraDirName="l1r3", extraName="--l1r3")
    draw_img_memtime(CRUSH, True, extraDirName="l1r4", extraName="--l1r4")
    draw_img_memtime(FastRedundantShare, True)

    draw_img_memtime(NearestNeighbour, False)
    draw_img_memtime(NearestNeighbour, False, extraDirName="HetCopies", extraName="--HetCopies")
    draw_img_memtime(Share, False)
    draw_img_memtime(Share, False, extraDirName="SF5", extraName="--5sf")
    draw_img_memtime(RedundantShare, False)
    draw_img_memtime(RedundantShare, False, extraDirName="Improved")
    draw_img_memtime(RUSHp, False)
    draw_img_memtime(RUSHt, False)
    draw_img_memtime(RUSHr, False)
    #draw_img(CRUSH, False, extraDirName="l1r1", extraName="--l1r1")
    draw_img_memtime(CRUSH, False, extraDirName="l1r2", extraName="--l1r2")
    #draw_img(CRUSH, False, extraDirName="l1r3", extraName="--l1r3")
    draw_img_memtime(CRUSH, False, extraDirName="l1r4", extraName="--l1r4")
    draw_img_memtime(FastRedundantShare, False, hetDiskBase=16)

    #draw_img_memtime_nn_nc(True, "DiskCopies")
    #draw_img_memtime_nn_nc(False, "DiskCopies")
    
    #draw_img_memtime_share_sf(True, "StretchFactor")
    #draw_img_memtime_share_sf(False, "StretchFactor")

    #draw_img_memtime_file(RedundantShare, RedundantShare, True, True)
    #draw_img_memtime_file(RedundantShare, RedundantShare, True, False)
    
