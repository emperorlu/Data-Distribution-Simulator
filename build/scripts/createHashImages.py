#!/usr/bin/python

import os.path
import os
import matplotlib
from disAnalysis.analysis import calc_hash_data, hashes
from disAnalysis.imageFactory import draw_image_bar_line_with_points, generate_german_dis_yticklabels, DisColorChooser
from createDistributor import NearestNeighbour, Share, RedundantShare, FastRedundantShare, RUSHp, CRUSH


__author__="fermat"
__date__ ="$13.07.2010 09:44:35$"

matplotlib.rcParams['font.size'] = 20.0

imagePath = "../data/images/"
disImagePath = "../data/disImages/"

def draw_img(distri, homogen, extraName="", extraDirName="", tasks=1, newName=True, disks=64):
    path = "../data/hashTests/"
    if homogen:
        path += "homogen/"
    else:
        path += "heterogen/"
    path += distri + extraDirName +"/ausgaben/"

    f = "Hash_"
    if homogen:
        f += "Hom_"
    else:
        f += "Het_"
    f += distri + extraDirName

    imagefile = imagePath + f
    disImageFile = disImagePath + f

    if (not os.path.exists(imagefile + ".png")) or (os.path.getmtime(imagefile + ".png") <= os.path.getmtime(path)) or (not os.path.exists(disImageFile + ".png")) or (os.path.getmtime(disImageFile + ".png") <= os.path.getmtime(path)):
        if newName:
            own2 = "own2"
        else:
            own2 = "ownXor"
        files = dict()
        my_hashes = hashes
        speedFiles = dict()
        for i in my_hashes:
            hashFiles = dict()
            hashSpeeds = dict()
            for j in xrange(0,4):
                copy = 2**j
                base = path + "CreateAndRun_" + distri + "_" + str(copy) + "K_" + str(tasks) + "T-" + str(disks) + "D_"
                if homogen:
                    base += "hom"
                else:
                    base += "het_0"
                base += ".disks-0-" + extraName
                if (i is None) or (i == 0):
                    hashFiles[copy] = base + "own.csv"
                    hashSpeeds[copy] = base + "own.run.out.std"
                elif i == 4000:
                    hashFiles[copy] = base + own2 + ".csv"
                    hashSpeeds[copy] = base + own2 + ".run.out.std"
                elif i > 4002:
                    h = i - 4000
                    hashFiles[copy] = base + "own" + str(h) + ".csv"
                    hashSpeeds[copy] = base + "own" + str(h) + ".run.out.std"
                else:
                    hashFiles[copy] = base + "gcrypt" + str(i) + ".csv"
                    hashSpeeds[copy] = base + "gcrypt" + str(i) + ".run.out.std"
            files[i] = hashFiles
            speedFiles[i] = hashSpeeds
        xticks = []
        for i in xrange(0,4):
            xticks.append("$" + str(i) + "$")
        data = calc_hash_data(files, speedFiles, imagefile)
        draw_image_bar_line_with_points(data.allMeans, imagefile, data.xticks, data.allConf, "copies", "usage", "time ($\mu$s)")
        draw_image_bar_line_with_points(data.allMeans, disImageFile, xticks, data.allConf, "copies", "usage", "time ($\mu$s)", colors=DisColorChooser(), rotation=0, ytickfct=generate_german_dis_yticklabels)
        #draw_img_hash(files, speedFiles, None, imagefile)
        print "Changed:  " + imagefile + ".png"
#    else:
#        print "Unchanged " + imagefile + ".png"

if __name__ == "__main__":
    draw_img(NearestNeighbour, True, newName=False)
    draw_img(Share, True, newName=False)
    draw_img(Share, True, extraName="-5sf", extraDirName="SF5")
    draw_img(RedundantShare, True, newName=False)
    draw_img(RedundantShare, True, extraDirName="Improved")
    draw_img(FastRedundantShare, True, disks=32, newName=False)
    draw_img(RUSHp, True)
    draw_img(CRUSH, True, extraDirName="l1r1", extraName="-l1r1")

    draw_img(NearestNeighbour, False, newName=False)
    draw_img(NearestNeighbour, False, newName=False, extraName="-HetCopies", extraDirName="HetCopies")
    draw_img(Share, False, newName=False)
    draw_img(Share, False, extraName="-5sf", extraDirName="SF5")
    draw_img(RedundantShare, False, newName=False)
    draw_img(RedundantShare, False, extraDirName="Improved")
    draw_img(FastRedundantShare, False, disks=32, newName=False)
    draw_img(RUSHp, False)
    draw_img(CRUSH, False, extraDirName="l1r1", extraName="-l1r1")
