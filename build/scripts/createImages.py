#!/usr/bin/python

from disAnalysis.imageFactory import draw_image_bar, generate_german_dis_yticklabels, DisColorChooser
from disAnalysis.analysis import calc_fairness_data
from createDistributor import NearestNeighbour, Share, RedundantShare, FastRedundantShare, RUSHp, CRUSH, RUSHt, RUSHr
import os
import os.path

__author__="fermat"
__date__ ="$09.04.2010 17:01:23$"

imagePath = "../data/images/"
disImagePath = "../data/disImages/"

def draw_img_fair(distri, homogen, extraName="", extraDirName="", tasks=8, extraDirPreName="", hetDiskNum=128):
    path = "../data/fairness/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += extraDirPreName + distri + extraDirName +"/ausgaben/"

    f = ""
    if len(extraDirPreName) > 0:
        f += extraDirPreName + "_"
    f += "Fair_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        files = dict()
        for j in range(0,4):
            copies = 2**j
            thisfile = dict()
            if homogen:
                for i in range (3,14):
                    disks = 2**i
                    f = path + "CreateAndRun_" + distri + "_" + str(copies) + "K_" + str(tasks) + "T-" + str(disks) + "D_hom.disks-0" + extraName + ".csv"
                    thisfile[disks] = f
            else:
                for i in range(0,10):
                    disks = hetDiskNum * (i + 1)
                    f = path + "CreateAndRun_" + distri + "_" + str(copies) + "K_" + str(tasks) + "T-" + str(disks) + "D_het_0.disks-0" + extraName + ".csv"
                    thisfile[disks] = f
            files[copies] = thisfile
        xticks = []
        if homogen:
            for i in range(3,14):
                xticks.append("$" + str(2**i) + "$")
        else:
            for i in range(0,10):
                xticks.append("$" + str(hetDiskNum * (i + 1)) +"$")
        data = calc_fairness_data(files, imagefile)
        draw_image_bar(data.allMeans, imagefile, data.xticks, data.allConf, "Storage Systems", "usage")
        if homogen:
            rotation=45
        else:
            rotation=45
        draw_image_bar(data.allMeans, disImageFile, xticks, data.allConf, "Festplatten", "Auslastung", colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        #draw_img_fairness(files, None, imagefile)
        print("Changed:  " + imagefile + ".png")
    #else:
    #    print("Uptodate: " + imagefile + ".png")

def draw_img_fair_file(distri1, distri2, homogen=True, big=True):
    path = "../data/fairness/"

    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += "File_"
    if big:
        path += "Big_"
    else:
        path += "Small_"
    path += distri1 + "_" + distri2 + "/ausgaben/"

    f = ""
    f += "Fair_File_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    if big:
        f += "Big_"
    else:
        f += "Small_"
    f += distri1 + "_" + distri2

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    disksPerFileCopy = 4
    if big:
        disksPerFileCopy = 16





    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        files = dict()
        for j in range(0,4):
            copy = 2**j
            thisfile = dict()
            if homogen:
                for i in range (3,14):
                    disks = 2**i
                    disks_per_file = disksPerFileCopy * copy
                    if disks_per_file > disks:
                        disks_per_file = disks
                    f = path + "CreateAndRun_" + distri1 + "_" + str(disks_per_file) + "K-" + distri2 + "_" + str(copy) + "K-" + str(disks) + "D_hom.disks.csv"
                    thisfile[disks] = f
            else:
                # TODO: heterogeneous
                pass
            files[copy] = thisfile
        xticks = []
        if homogen:
            for i in range(3,14):
                xticks.append("$" + str(2**i) + "$")
        else:
            for i in range(0,10):
                xticks.append("$" + str(hetDiskNum * (i + 1)) +"$")
        data = calc_fairness_data(files, imagefile)
        draw_image_bar(data.allMeans, imagefile, data.xticks, data.allConf, "Storage Systems", "usage")
        if homogen:
            rotation=45
        else:
            rotation=45
        draw_image_bar(data.allMeans, disImageFile, xticks, data.allConf, "Festplatten", "Auslastung", colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        #draw_img_fairness(files, None, imagefile)
        print("Changed:  " + imagefile + ".png")
    #else:
    #    print("Uptodate: " + imagefile + ".png")

def draw_img_fair_share_sf(homogen, extraDirName="", tasks=8, extraDirPreName=""):
    distri = Share
    path = "../data/fairness/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += extraDirPreName + distri + extraDirName +"/ausgaben/"

    f = ""
    if len(extraDirPreName) > 0:
        f += extraDirPreName + "_"
    f += "Fair_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        files = dict()
        for i in range(0,4):
            copy = 2**i
            thisfile = dict()
            for j in range(0,7):
                diskCopies = 5 * (j+1)
                cmf = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-256D_"
                if homogen:
                    cmf += "hom"
                else:
                    cmf += "het_0"
                cmf += ".disks-0--" + str(diskCopies) + "sf.csv"
                thisfile[diskCopies] = cmf
            files[copy] = thisfile
        xticks = []
        for i in range(0,7):
            sf = 5 * (i+1) * 8
            xticks.append("$" + str(sf) + "$")
        data = calc_fairness_data(files, imagefile)
        draw_image_bar(data.allMeans, imagefile, data.xticks, data.allConf, "Stretchfactor", "usage")
        if homogen:
            rotation=0
        else:
            rotation=45
        draw_image_bar(data.allMeans, disImageFile, xticks, data.allConf, "Dehnungsfaktor", "Auslastung", colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        #draw_img_fairness(files, None, imagefile)
        print("Changed:  " + imagefile + ".png")

def draw_img_fair_nn_nc(homogen, extraDirName="", tasks=8, extraDirPreName=""):
    distri = NearestNeighbour
    path = "../data/fairness/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += extraDirPreName + distri + extraDirName +"/ausgaben/"

    f = ""
    if len(extraDirPreName) > 0:
        f += extraDirPreName + "_"
    f += "Fair_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        files = dict()
        for i in range(0,4):
            copy = 2**i
            thisfile = dict()
            for j in range(0,10):
                diskCopies = 300 * (j+1)
                cmf = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-256D_"
                if homogen:
                    cmf += "hom"
                else:
                    cmf += "het_0"
                cmf += ".disks-0--" + str(diskCopies) + "DiskCopy"
                cmf += ".csv"
                thisfile[diskCopies] = cmf
            files[copy] = thisfile
        xticks = []
        for i in range(0,10):
            sf = 300 * (i+1) * 8
            xticks.append("$" + str(sf) + "$")
        data = calc_fairness_data(files, imagefile)
        draw_image_bar(data.allMeans, imagefile, data.xticks, data.allConf, "Placed Copies of each Disk", "usage")
        if homogen:
            rotation=45
        else:
            rotation=45
        draw_image_bar(data.allMeans, disImageFile, xticks, data.allConf, "Platzierte Kopien jeder Festplatte", "Auslastung", colors=DisColorChooser(), rotation=rotation, ytickfct=generate_german_dis_yticklabels)
        #draw_img_fairness(files, None, imagefile)
        print("Changed:  " + imagefile + ".png")

def draw_img_fairness_share_sf60_het():
    path = "../data/fairness/heterogen/ShareSF60/ausgaben/"
    files = dict()
    for j in range(0,1):
        copies = 2**j
        thisfile = dict()
        for i in range (1,2):
            disks = 128 * i
            thisfile[disks] = path + "CreateAndRun_Share_" + str(copies) + "K_8T-" + str(disks) + "D_het_0.disks-0--sf60.csv"
        files[copies] = thisfile
    draw_img_fairness(files, None, imagePath + "Fair_Het_ShareSF60")

def draw_img_fairness_share_sfasdisknum_het():
    path = "../data/fairness/homogen/ShareTest/ausgaben/"
    files = dict()
    for j in range(0,1):
        copies = 2**j
        thisfile = dict()
        for i in range (1,2):
            disks = 128 * i
            thisfile[disks] = path + "ShareTest.csv"
        files[copies] = thisfile
    draw_img_fairness(files, None, imagePath + "Fair_Hom_ShareTest")

if __name__ == "__main__":
    #Missing Data: Share hom for K=2..8 for 1024 Disks
    small = "Small"
    mid = "Mid"

    draw_img_fair(NearestNeighbour, True, extraDirPreName=small)
    draw_img_fair(NearestNeighbour, True, extraName="--StaticCopies", extraDirName="StaticCopies", extraDirPreName=small)
    draw_img_fair(NearestNeighbour, True, extraName="--Static2800", extraDirName="StaticCopies2800", extraDirPreName=small)
    draw_img_fair(Share, True, extraName="--5sf", extraDirName="SF5", extraDirPreName=small, tasks=24)
    draw_img_fair(RedundantShare, True, extraDirPreName=small)
    draw_img_fair(RedundantShare, True, extraDirName="Improved", extraDirPreName=small, tasks=4)
    draw_img_fair(RUSHp, True, extraDirPreName=small, tasks=24)
    #draw_img_fair(RUSHt, True, extraDirPreName=small, tasks=8)
    #draw_img_fair(RUSHr, True, extraDirPreName=small, tasks=8)
    #draw_img_fair(CRUSH, True, extraName="--l1r1", extraDirName="l1r1", tasks=24, extraDirPreName=small)
    #draw_img_fair(CRUSH, True, extraName="--l1r2", extraDirName="l1r2", tasks=24, extraDirPreName=small)
    #draw_img_fair(CRUSH, True, extraName="--l1r3", extraDirName="l1r3", tasks=8, extraDirPreName=small)
    #draw_img_fair(CRUSH, True, extraName="--l1r4", extraDirName="l1r4", tasks=8, extraDirPreName=small)

    draw_img_fair(NearestNeighbour, False, extraDirPreName=small)
    draw_img_fair(NearestNeighbour, False, extraName="--HetCopies", extraDirName="HetCopies", extraDirPreName=small)
    draw_img_fair(NearestNeighbour, False, extraName="--HetCopiesStatic", extraDirName="HetCopiesStatic", extraDirPreName=small)
    draw_img_fair(Share, False, extraName="--5sf", extraDirName="SF5", extraDirPreName=small, tasks=24)
    draw_img_fair(RedundantShare, False, extraDirPreName=small)
    draw_img_fair(RedundantShare, False, extraDirName="Improved", extraDirPreName=small)
    draw_img_fair(RUSHp, False, extraDirPreName=small)
    #draw_img_fair(RUSHt, False, extraDirPreName=small)
    #draw_img_fair(RUSHr, False, extraDirPreName=small)
    #draw_img_fair(CRUSH, False, extraName="--l1r1", extraDirName="l1r1", tasks=8, extraDirPreName=small)
    #draw_img_fair(CRUSH, False, extraName="--l1r2", extraDirName="l1r2", tasks=8, extraDirPreName=small)
    #draw_img_fair(CRUSH, False, extraName="--l1r3", extraDirName="l1r3", tasks=4, extraDirPreName=small)
    #draw_img_fair(CRUSH, False, extraName="--l1r4", extraDirName="l1r4", tasks=24, extraDirPreName=small)

    #draw_img_fair(NearestNeighbour, True, extraDirPreName=mid)
    #draw_img_fair(NearestNeighbour, True, extraName="-StaticCopies", extraDirName="Static", extraDirPreName=mid)
    #draw_img_fair(Share, True, extraDirPreName=mid)
    #draw_img_fair(Share, True, extraName="--5sf", extraDirName="SF5", extraDirPreName=mid)
    #draw_img_fair(Share, True, extraName="-StaticStretchfactor", extraDirName="StaticStretchfactor", extraDirPreName=mid)
    #draw_img_fair(RedundantShare, True, extraDirPreName=mid)

    #draw_img_fair(NearestNeighbour, False, extraDirPreName=mid)
    #draw_img_fair(NearestNeighbour, False, extraName="--HetCopies", extraDirName="HetCopies", extraDirPreName=mid)
    #draw_img_fair(NearestNeighbour, False, extraName="-HetCopiesStatic", extraDirName="HetCopiesStatic", extraDirPreName=mid)
    #draw_img_fair(Share, False, extraDirPreName=mid)
    #draw_img_fair(Share, False, extraName="--5sf", extraDirName="SF5", extraDirPreName=mid, tasks=24)
    #draw_img_fair(Share, False, extraDirPreName=mid, extraName="-StaticStretchfactor", extraDirName="StaticStretchfactor")
    #draw_img_fair(RedundantShare, False, extraDirPreName=mid, tasks=4)

    #draw_img_fair_nn_nc(True, extraDirName="DiskCopies", tasks=8, extraDirPreName=mid)
    #draw_img_fair_nn_nc(False, extraDirName="DiskCopies", tasks=8, extraDirPreName=mid)

    #draw_img_fair_share_sf(True, extraDirName="SFTest", tasks=8, extraDirPreName=mid)
    #draw_img_fair_share_sf(False, extraDirName="SFTest", tasks=8, extraDirPreName=mid)

    draw_img_fair(NearestNeighbour, True)
    draw_img_fair(NearestNeighbour, True, extraDirName="StaticCopies", extraName="-StaticCopies")
    draw_img_fair(Share, True)
    draw_img_fair(RedundantShare, True)
    #draw_img_fair(FastRedundantShare, True)

    draw_img_fair(NearestNeighbour, False)
    draw_img_fair(NearestNeighbour, False, extraDirName="HetCopies", extraName="--HetCopies")
    # Missing: HetCopiesStatic
    draw_img_fair(Share, False)
    draw_img_fair(RedundantShare, False)
    #draw_img_fair(FastRedundantShare, False, hetDiskNum=16)
    #draw_img_fairness_fastredundantshare_het()

    #draw_img_fairness_share_sfasdisknum_het()
    #draw_img_fairness_share_sf60_het()

    #draw_img_fair_file(RedundantShare, RedundantShare)
