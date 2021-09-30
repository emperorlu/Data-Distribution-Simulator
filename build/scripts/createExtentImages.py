#!/usr/bin/python

from disAnalysis.imageFactory import DisColorChooser
#import matplotlib
import os
import os.path
from disAnalysis.analysis import calc_fairness_data
from disAnalysis.imageFactory import draw_image_bar, DisColorChooser, generate_german_dis_yticklabels
from createDistributor import NearestNeighbour, Share, RedundantShare, FastRedundantShare, RUSHp, CRUSH

#matplotlib.rc('text', usetex=True)

__author__="fermat"
__date__ ="$10.09.2010 15:22:44$"

imagePath = "../data/images/"
disImagePath = "../data/disImages/"

def draw_img(distri, homogen, extraName="", extraDirName="", tasks=8):
    path = "../data/extent/" 
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "Extent_" 
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        files = dict()
        for j in xrange(0,4):
            copies = 2**j
            thisfile = dict()
            for i in xrange (0,5):
                numExtents = 25 * (10 ** i)
                f = path + "CreateAndRun_" + distri + "_" + str(copies) + "K_" + str(tasks) + "T-64D_"
                if homogen:
                    f += "hom"
                else:
                    f += "het_0"
                f += ".disks-0-" + str(numExtents) + "Extents" + extraName + ".csv"
                thisfile[numExtents] = f
            files[copies] = thisfile
        xticks = []
        for i in xrange (0,5):
            xticks.append("$25\cdot10^{"+str(i)+"}$")
        data = calc_fairness_data(files, imagefile)
        draw_image_bar(data.allMeans, imagefile, xticks, data.allConf, "Extents", "usage")
        draw_image_bar(data.allMeans, disImageFile, xticks, data.allConf, u"Bl\u00f6cke pro Festplatte", "Auslastung", colors=DisColorChooser(), rotation=0, ytickfct=generate_german_dis_yticklabels)
        print("Changed:  " + imagefile + ".png")
    
if __name__ == "__main__":
    draw_img(NearestNeighbour, True)
    draw_img(Share, True)
    draw_img(Share, True, extraDirName="SF5", extraName="--5sf")
    draw_img(RedundantShare, True)
    draw_img(RUSHp, True, tasks=4)
    draw_img(CRUSH, True, extraDirName="l1r1", extraName="--l1r1")
    draw_img(CRUSH, True, extraDirName="l1r2", extraName="--l1r2")
    draw_img(CRUSH, True, extraDirName="l1r3", extraName="--l1r3", tasks=4)
    draw_img(CRUSH, True, extraDirName="l1r4", extraName="--l1r4")
    
    draw_img(NearestNeighbour, False)
    draw_img(NearestNeighbour, False, extraDirName="HetCopies", extraName="--HetCopies")
    draw_img(Share, False)
    draw_img(Share, False, extraDirName="SF5", extraName="--5sf")
    draw_img(RedundantShare, False)
    draw_img(RUSHp, False)
    draw_img(CRUSH, False, extraDirName="l1r1", extraName="--l1r1")
    draw_img(CRUSH, False, extraDirName="l1r2", extraName="--l1r2")
    draw_img(CRUSH, False, extraDirName="l1r3", extraName="--l1r3")
    draw_img(CRUSH, False, extraDirName="l1r4", extraName="--l1r4", tasks=4)
