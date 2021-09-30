#! /usr/bin/python

import numpy as np
import matplotlib
import matplotlib.pyplot as plt

matplotlib.rcParams['font.size'] = 12.0
matplotlib.rc('text', usetex=True)

__author__="fermat"
__date__ ="$18.10.2010 16:24:41$"

valuesColor = "#004aff" # Color 1
errorColor  = "#ffd500" # Color 2

disks = 10

values              = [0] * disks
errorNormValues     = [0] * disks
errorAttachedValues = [0] * disks

def generate_german_dis_yticklabels(tick):
    return "$" + str(tick).replace(".", "DELIM").replace(",", ".").replace("DELIM", ",") + "$"

if __name__ == "__main__":
    available = 0
    xticks = []
    for i in xrange(disks):
        print i
        values[disks - 1 - i] = float(1) / float(i + 1)
        if (i > 7) or (i < 2) or (i == 6) or (i == 3) or (i== 4):
            available += 1
            errorNormValues[disks - 1 - i] = values[disks - 1 - i]
            errorAttachedValues[disks - 1 - i] = float(1) / float(available) - values[disks - 1 - i]
        else:
            errorNormValues [disks - 1 - i] = 0
            errorAttachedValues [disks - 1 - i] = 0
        xticks.append("$" + str(i+1) + "$")
    width = 0.8

    fig = plt.figure(figsize=(2.0, 2.0))
    ax = fig.add_axes([0.3, 0.2, 0.65, 0.75])
    ind = np.arange(10)
    graph = ax.bar(ind, values, width, color=valuesColor)
    ax.set_xticks(ind + float(0.45))
    ax.set_xticklabels(xticks)
    ax.set_ylabel(r"$\check{c}_i$")
    ax.set_xlabel("$i$")
    yticks = ax.get_yticks()
    yticklabels = []
    for ytick in yticks:
        yticklabels.append(generate_german_dis_yticklabels(ytick))
    ax.set_yticklabels(yticklabels)
    fig.savefig("/Users/fermat/tmp/images/peerRS_complete.png", dpi=600)

    height = 2.0

    fig = plt.figure(figsize=(2.0, 2.0))
    ax = fig.add_axes([0.3, 0.2, 0.65, 0.75])
    rects = []
    graph = ax.bar(ind, errorNormValues, width, color=valuesColor)
    rects.append(graph[0])
    graph = ax.bar(ind, errorAttachedValues, width, color=errorColor, bottom=errorNormValues)
    rects.append(graph[0])
    ax.set_xticks(ind + float(0.45))
    ax.set_xticklabels(xticks)
    ax.set_ylabel(r"$\check{c}'_i$")
    ax.set_xlabel("$i$")
    yticks = ax.get_yticks()
    yticklabels = []
    for ytick in yticks:
        yticklabels.append(generate_german_dis_yticklabels(ytick))
    ax.set_yticklabels(yticklabels)
    #fig.legend(rects, ("complete view", "failure"), 'lower center', ncol=2)
    fig.savefig("/Users/fermat/tmp/images/peerRS_failure.png", dpi=600)

    fig = plt.figure(figsize=(4.0, 0.5))
    fig.legend(rects, ("Komplette Sicht", "Fehler"), 'lower center', ncol=2)
    fig.savefig("/Users/fermat/tmp/images/peerRS_legend.png", dpi=600)