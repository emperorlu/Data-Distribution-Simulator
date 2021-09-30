#! /opt/local/bin/python

# To change this template, choose Tools | Templates
# and open the template in the editor.

import os.path
import csv
import numpy as np
import scipy.stats
import matplotlib
#matplotlib.use("PDF")
import glob
import os
import matplotlib.pyplot as plt
import sys

def mean(data):
    """
    Calculate the Mean value of the given array.
    """
    a = 1.0 * np.array(data)
    return np.mean(a)

def mean_confidence_spread(data, confidence=0.95):
    """
    calculate the confidence intervall for the given array
    """
    a = 1.0*np.array(data)
    n = len(a)
    se = scipy.stats.sem(a)

    # calls the inverse CDF of the Student's t distribution, rotation=45
    h = se * scipy.stats.t._ppf((1+confidence)/2., n-1)
    return h

def mean_confidence_interval(data, confidence=0.95):
    """
    calculate the confidence intervall for the given array
    """
    a = 1.0*np.array(data)
    m = np.mean(a)
    # calls the inverse CDF of the Student's t distribution
    h = mean_confidence_spread(data, confidence)
    return m-h, m+h

def print_raw_Values(rawValues):
    print "We have " + str(len(rawValues)) + " Graphes"
    for i in xrange(len(rawValues)):
        print "Graph " + str(i) + " has " + str(len(rawValues[i])) + " x-Entries "
        for j in xrange(len(rawValues[i])):
            print "Graph "+ str(i) + " X-Entry " + str(j) + " has "+ str(len(rawValues[i][j])) + " values "
            for k in xrange(len(rawValues[i][j])):
                print str(i) + ", " + str(j) + ", " + str(k) + ", " + str(rawValues[i][j][k])

def build_picture(filename, kind = "bar", outFile = None):
    print "Will create Image out of file " + filename
    print "It will have the kind " + kind
    if outFile == None:
        print "it will be shown on screen"
    else:
        print "it will be stored to " + outFile
    colors = ['r', 'g', 'b', 'y', 'c', 'm', 'w']
    ecolors = len(colors) * ['k']
    label = None
    xLabel = None
    yLabel = None
    graphes = 0
    noLegend = 1
    legend = []
    xLength = 0
    xPoints = []
    rawGraphValues = []
    
    initializing = 1
    firstValLine = 1
    done = False;

    print "reading data"
    f = open(filename)
    r = csv.reader(f, delimiter=';')
    for row in r:
        if initializing == 1:
            if row[0] == "Values":
                initializing = 0
                firstValLine = 1
                if graphes == 0:
                    print "Error: Values defined before setting num of graphes"
                    sys.exit(-1)
            elif row[0] == "Label":
                label = row[1]
            elif row[0] == "Graphes":
                graphes = int(row[1])
            elif row[0] == "legend":
                if row[1] == "none":
                    noLegend = 1;
                else:
                    noLegend = 0;
                    g = int(row[1])
                    legend.append(row[2])
            elif row[0] == "X-bar":
                xLabel = row[1]
            elif row[0] == "Y-bar":
                yLabel = row[1]
        else:
            if firstValLine == 1:
                i = 0
                end = False
                firstValLine = 0
                for cell in row:
                    if not end:
                        if i == 0:
                            i += 1
                        else:
                            if cell != "":
                                xLength += 1
                                xPoints.append(cell)
                            else:
                                end = True
                if xLength == 0:
                    print "Error: Data Entries found"
                    sys.exit(-1)
                print "xlength after setting: " + str(xLength)
                print "graphes: " + str(graphes)
                for i in xrange(graphes):
                    graph = []
                    for j in xrange(xLength):
                        graph.append([])
                    rawGraphValues.append(graph)
            else:
                if row[0] == "":
                    done = True;
                else:
                    if not done:
                        g = int(row[0])
                        graph = rawGraphValues[g]
                        for i in xrange(xLength):
                            print "i: " + str(i)
                            cell = row[i+1]
                            graph[i].append(cell)
    f.close()

    print_raw_Values(rawGraphValues)

    print "calculating values"
    print "xlength: " + str(xLength)
    graphValues = []
    graphErrors = []
    for i in xrange(graphes):
        graph = []
        graphError = []
        rawGraph = rawGraphValues[i]
        for j in xrange(xLength):
            print "(" + str(i) + "," + str(j) + ")"
            ratios = np.zeros(len(rawGraph[j]), dtype=np.float)
            for k in xrange(len(rawGraph[j])):
                ratios[k] = rawGraph[j][k]
            graph.append(mean(ratios))
            graphError.append(mean_confidence_spread(ratios))
        graphValues.append(graph)
        graphErrors.append(graphError)

    print "creating image"
    plt.figure()
    barwidth = 0.9 / (1.0 * graphes)
    plotGraphes = []
    ind = np.arange(xLength)
    print "barwidth = " + str(barwidth)
    for i in xrange(graphes):
        print "Will add Graph " + str(i)
        gv = graphValues[i]
        ge = graphErrors[i]
        print "legth gv: " + str(len(gv))
        print "legth ge: " + str(len(ge))
        print "legth ind: " + str(len(ind))
        if kind == "bar":
            plotGraphes.append(plt.bar(ind + (i * barwidth), gv, barwidth, yerr = ge, color=colors[i], ecolor=ecolors[i]))
            plt.xticks(ind + 0.45, xPoints, rotation=45)
        elif kind == "line":
            plotGraphes.append(plt.errorbar(ind, gv, yerr = ge, color=colors[i], ecolor=ecolors[i]))
            plt.xticks(ind, xPoints, rotation=45)

    if xLabel != None:
        plt.xlabel(xLabel)

    if yLabel != None:
        plt.ylabel(yLabel)

    if label != None:
        plt.title(label)
    
    if noLegend == 0:
        rects = graphes * [0]
        for i in xrange(graphes):
            rects[i] = plotGraphes[i][0]
        plt.legend(rects, legend)

    print "will show image"
    if outFile == None:
        plt.show()
    else:
        plt.savefig(outFile)


if __name__ == "__main__":
    print "Hello World\n";
    #sys.exit(-1)
    if len(sys.argv) < 2 or len(sys.argv) > 4:
        print "createImagesMatfrye.py CSV-File [kind [outfile]]"
        sys.exit(-1)

    filename = sys.argv[1]
    kind = "bar"
    outfile = None

    if len(sys.argv) > 2:
        kind = sys.argv[2]

    if len(sys.argv) > 3:
        outfile = sys.argv[3]

    build_picture(filename, kind, outfile)
    
