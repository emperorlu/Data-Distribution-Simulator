#! /usr/bin/python

import numpy as np
import matplotlib
import matplotlib.pyplot as plt

#matplotlib.rcParams['font.size'] = 12.0

__author__="fermat"
__date__ ="$15.10.2010 12:27:40$"

baseColors=("#004aff", "#ffd500", "#ff0093", "#00ffc0", "#8000ff", "#7dff00", "#ff0000", "#00fcff")

optimalColor     = "#004aff" # Color 1
alphaColor       = "#ffd500" # Color 2
adapOptimalColor = "#ff0093" # Color 3
measuredColor    = "#00ffc0" # Color 4

hardDiskMeasured = (2.4732, 3.2642, 3.7422, 10.2508, 18.3810, 27.0744)
hardDiskOptimal  = (2.4732, 2.4732, 3.9572,  7.1949, 13.7641, 20.3512)
legendHardDisk   = ("optimal", "gemessen")

noRedMeassures = (9.0005,  8.8281, 15.7890, 29.7534, 53.7951,  62.4240)
noRedOptimal   = (9.0005, 12.0006, 20.5725, 38.4020, 74.3265, 110.3038)
noRedAlpha     = (9.0005,  8.8805, 15.2237, 28.4175, 55.0016,  81.6248)
legendNoRed    = ("optimal", "$\\alpha=0,74$", "gemessen")

raid1Meassures = (9.0000, 5.4833, 11.7391, 20.7636, 33.5985, 42.1726)
raid1Optimal   = (9.0000, 9.0000, 14.4000, 26.1818, 50.0870, 74.0571)
raid1Alpha     = (9.0000, 6.3000, 10.0800, 18.3273, 35.0609, 51.8400)
legendRaid1    = ("optimal", "$\\alpha=0,7$", "gemessen")

raid5Meassures = (9.0000,  4.8495,  8.0957, 14.4446, 25.4957, 29.1283)
raid5Optimal   = (9.0000, 11.0769, 18.5806, 34.3881, 66.3022, 98.2749)
raid5Adap      = (9.0000,  6.5455, 12.0000, 23.0400, 45.1765, 67.3247)
raid5Alpha     = (9.0000,  4.7435,  7.5014, 13.5815, 25.9345, 38.3240)
legendRaid5    = ("optimal", "optimal mit Stripe", "$\\alpha=0,56$ mit Stripe", "gemessen")


xValues = ("$1$", "$2$", "$4$", "$8$", "$16$", "$24$")
xLabel  = ("Speicherknoten")
yLabel  = ("MByte/s")

def generate_german_dis_yticklabels(tick):
    return "$" + str(tick).replace(".", "DELIM").replace(",", ".").replace("DELIM", ",") + "$"

def draw_sb_image(optimal, measured, alpha, adapOptimal, legend, filename):
    print filename
    bars = 0
    if optimal is not None:
        bars += 1
    if measured is not None:
        bars += 1
    if alpha is not None:
        bars += 1
    if adapOptimal is not None:
        bars += 1
    width = 0.9 / float(bars)
    fig = plt.figure(figsize=(4.0, 4.0))
    if bars < 3:
        ax = fig.add_axes([0.2, 0.3, 0.75, 0.65])
    if bars == 3:
        ax = fig.add_axes([0.2, 0.4, 0.75, 0.55])
    if bars > 3:
        ax = fig.add_axes([0.2, 0.5, 0.75, 0.45])
    #ax = fig.add_axes()
    rects = []
    ind = np.arange(6)
    bar = 0
    print (len(ind))
    print len(optimal)
    if optimal is not None:
        graph = ax.bar(ind + (float(bar) * width), optimal, width, color=optimalColor)
        rects.append(graph[0])
        bar += 1
    if adapOptimal is not None:
        graph = ax.bar(ind + (float(bar) * width), adapOptimal, width, color=adapOptimalColor)
        rects.append(graph[0])
        bar += 1
    if alpha is not None:
        graph = ax.bar(ind + (float(bar) * width), alpha, width, color=alphaColor)
        rects.append(graph[0])
        bar += 1
    if measured is not None:
        graph = ax.bar(ind + (float(bar) * width), measured, width, color=measuredColor)
        rects.append(graph[0])
        bar += 1
    ax.set_xticks(ind + float(0.45))
    ax.set_xticklabels(xValues)
    ax.set_ylabel(yLabel)
    ax.set_xlabel(xLabel)
    yticks = ax.get_yticks()
    yticklabels = []
    for ytick in yticks:
        yticklabels.append(generate_german_dis_yticklabels(ytick))
    ax.set_yticklabels(yticklabels)
    #fig.legend(rects, legend, 'upper left')
    fig.legend(rects, legend, 'lower center')
    fig.savefig(filename, dpi=600)

if __name__ == "__main__":
    draw_sb_image(hardDiskOptimal, hardDiskMeasured, None,       None,      legendHardDisk, "/Users/fermat/tmp/images/sb_hd.png")
    draw_sb_image(noRedOptimal,    noRedMeassures,   noRedAlpha, None,      legendNoRed,    "/Users/fermat/tmp/images/sb_noRaid.png")
    draw_sb_image(raid1Optimal,    raid1Meassures,   raid1Alpha, None,      legendRaid1,    "/Users/fermat/tmp/images/sb_raid1.png")
    draw_sb_image(raid5Optimal,    raid5Meassures,   raid5Alpha, raid5Adap, legendRaid5,    "/Users/fermat/tmp/images/sb_raid5.png")
