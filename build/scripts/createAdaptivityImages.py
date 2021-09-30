#!/usr/bin/python

import os.path
import os
from disAnalysis.analysis import calc_adaptivity_data
from disAnalysis.imageFactory import draw_image_bar_multi, generate_german_dis_yticklabels, DisColorChooser
from disAnalysis.DistributorData import read_movement_data
from createDistributor import NearestNeighbour, Share, RedundantShare, FastRedundantShare, RUSHp, CRUSH, RUSHt, RUSHr
import numpy as np
import matplotlib.pyplot as plt

__author__="fermat"
__date__ ="$19.07.2010 14:42:24$"

imagePath = "../data/images/"
disImagePath = "../data/disImages/"

adap_add_disks = (1, 2, 3, 5, 7, 11, 13)

def draw_img_adap(distri, homogen, extraName="", extraDirName="", tasks=8, baseDisks=128):
    path = "../data/adaptivitaet/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "Adap_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        movemanetfiles = dict()
        for j in xrange(0,4):
            copies = 2**j
            thismovementfile = dict()
            for i in xrange (0,7):
                disks = baseDisks + adap_add_disks[i]
                f = path + distri + "_" + str(copies) + "K_" + str(tasks) + "T-" + str(disks) + "D_het_0.disks-0" + extraName + "_movements.csv"
                thismovementfile[adap_add_disks[i]] = f
            movemanetfiles[copies] = thismovementfile
        xticks = []
        for i in xrange (0,7):
            xticks.append("$" + str(adap_add_disks[i]) + "$")
        result = calc_adaptivity_data(movemanetfiles, imagefile)
        draw_image_bar_multi(result.allData, imagefile, result.xticks, "added Storage Systems", "moved Data")
        draw_image_bar_multi(result.allData, disImageFile, xticks, u"hinzugef\u00fcgte Festplatten", "verschobene Daten", colors=DisColorChooser(), rotation=0, ytickfct=generate_german_dis_yticklabels)
        #draw_img_adaptivity(movemanetfiles, None, imagefile)
        print("Changed:  " + imagefile + ".png")

def draw_img_rs_copy(homogen=True, extraName="", extraDirName="CopyTest", tasks=8, baseDisks=128):
    distri = RedundantShare
    path = "../data/adaptivitaet/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "Adap_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    disks = baseDisks + 1
    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        values = []
        for i in xrange(0,7):
            copies = 2**i
            datafilename = path + distri + "_" + str(copies) + "K_" + str(tasks) + "T-" + str(disks) + "D_het_0.disks-0" + extraName + "_movements.csv"
            data = read_movement_data(datafilename)
            movedData = data[1][7]
            print copies.zfill(width)
            values.append(movedData)
        ind = np.arange(7)
        fig = plt.figure()
        ax = fig.add_axes()
        ax.plot(ind, values, "o-")
        fig.savefig(disImageFile)
        fig.savefig(imagefile)

if __name__ == "__main__":

    draw_img_adap(NearestNeighbour, True)
    draw_img_adap(NearestNeighbour, True, extraDirName="Static", extraName="StaticCopies")
    draw_img_adap(Share, True)
    draw_img_adap(Share, True, extraDirName="Static", extraName="StaticStretchfactor")
    draw_img_adap(Share, True, extraDirName="SF5", extraName="-5sf")
    draw_img_adap(Share, True, extraDirName="Static35", extraName="StaticStretchfactor35")
    draw_img_adap(RedundantShare, True)
    draw_img_adap(RedundantShare, True, extraDirName="_friendly", extraName="Friendly")
    draw_img_adap(RedundantShare, True, extraDirName="Improved")
    draw_img_adap(RedundantShare, True, extraDirName="Improved_friendly", extraName="Friendly")
    draw_img_adap(RUSHp, True)
    draw_img_adap(RUSHt, True)
    #draw_img_adap(RUSHr, True)
    draw_img_adap(CRUSH, True, extraDirName="l1r1", extraName="-l1r1")
    draw_img_adap(CRUSH, True, extraDirName="l1r2", extraName="-l1r2")
    draw_img_adap(CRUSH, True, extraDirName="l1r3", extraName="-l1r3")
    draw_img_adap(CRUSH, True, extraDirName="l1r4", extraName="-l1r4")
    draw_img_adap(FastRedundantShare, True, baseDisks=16)



    draw_img_adap(NearestNeighbour, False)
    draw_img_adap(NearestNeighbour, False, extraDirName="Static", extraName="StaticCopies")
    draw_img_adap(NearestNeighbour, False, extraDirName="HetCopies", extraName="-HetCopies")
    draw_img_adap(NearestNeighbour, False, extraDirName="HetCopiesStatic", extraName="-HetCopiesStatic")
    draw_img_adap(Share, False)
    draw_img_adap(Share, False, extraDirName="Static", extraName="StaticStretchfactor")
    draw_img_adap(Share, False, extraDirName="SF5", extraName="-5sf", tasks=24)
    draw_img_adap(Share, False, extraDirName="Static35", extraName="StaticStretchfactor35")
    draw_img_adap(RedundantShare, False)
    draw_img_adap(RedundantShare, False, extraDirName="Improved")
    draw_img_adap(RUSHp, False)
    draw_img_adap(RUSHt, False)
    #draw_img_adap(RUSHr, False)
    draw_img_adap(CRUSH, False, extraDirName="l1r1", extraName="-l1r1", tasks=4)
    draw_img_adap(CRUSH, False, extraDirName="l1r2", extraName="-l1r2")
    draw_img_adap(CRUSH, False, extraDirName="l1r3", extraName="-l1r3")
    draw_img_adap(CRUSH, False, extraDirName="l1r4", extraName="-l1r4")
    draw_img_adap(FastRedundantShare, False, baseDisks=16)
