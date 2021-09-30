import numpy as np
import matplotlib
import matplotlib.pyplot as plt

__author__="fermat"
__date__ ="$17.09.2010 12:28:27$"

def generate_german_dis_yticklabels(tick):
    return "$" + str(tick).replace(".", "DELIM").replace(",", ".").replace("DELIM", ",") + "$"

class BWColorChooser:
    def get_error_colors(self, num=10):
        colors = ["k"] * num
        return colors

    def get_bar_colors(self, num=10):
        colorSteping = float(1) / float(num + 1)
        colors = []
        for i in xrange(num):
            colors.append((float(i + 1) * colorSteping, float(i + 1) * colorSteping, float(i + 1) * colorSteping))
        return colors

    def get_bar_range_colors(self, num=10):
        colors = ["w"] * num
        return colors

    def get_multi_range_colors(self, kinds=2, num=10):
        allColors = []
        for j in xrange(kinds):
            if (j % 2) == 0:
                colors = ["w"] * num
            else:
                colors = ["k"] * num
            allColors.append(colors)
        return allColors
    
class DisColorChooser(BWColorChooser):
    def __init__(self):
        self.baseColors=("#004aff", "#ffd500", "#ff0093", "#00ffc0", "#8000ff", "#7dff00", "#ff0000", "#00fcff")
        self.range1Colors=(("#d6e2ff", "#fff7cc", "#facce8", "#cdfced", "#e2c5ff", "#dbffb6", "#ffb9b9", "#c8ffff"),
                           ("#81a5fd", "#ffea7d", "#ff86cd", "#70eecb", "#be7bff", "#bfff82", "#ff8686", "#70dadb"))

    def get_bar_colors(self, num=10):
        if num <= len(self.baseColors):
            return self.baseColors[:num]
        print("WARNING: Requested " + str(num) + " colors, but I only have " + str(len(self.baseColors)) + " different")
        colors = []
        for i in xrange(num):
            colors.append(self.baseColors[i % len(self.baseColors)])
        return colors

    def get_bar_range_colors(self, num=10):
        if num <= len(self.range1Colors[0]):
            return self.range1Colors[0][:num]
        print("WARNING: Requested " + str(num) + " range colors, but I only have " + str(len(self.range1Colors[0])) + " different")
        colors = []
        for i in xrange(num):
            colors.append(self.range1Colors[0][i % len(self.range1Colors[0])])
        return colors

    def get_multi_range_colors(self, kinds=2, num=10):
        if kinds > len(self.range1Colors):
            print("WARNING: Requested " + str(kinds) + " different areas of colors, but I only have " + str(len(self.range1Colors)) + " different")
        allColors = []
        for i in xrange(kinds):
            myBaseArea = self.range1Colors[i % len(self.range1Colors)]
            if num <= len(myBaseArea):
                thisColors = myBaseArea[:num]
            else:
                print("WARNING: Requested " + str(num) + " different colors in kind " + str(i % len(self.range1Colors)) + ", but I only have " + str(len(myBaseArea)) + " different")
                thisColors=[]
                for j in xrange(num):
                    thisColors.append(myBaseArea[j % len(myBaseArea)])
            allColors.append(thisColors)
        return allColors

def draw_image_bar(values, filename=None, xticks=None, intervalls=None, xlabel=None, ylabel=None, label=None, legend=None, difLegend=None, legendIntervalls=None, distance=0.1, imgRanges=None, colors=BWColorChooser(), ytickfct=None, rotation=24):
    if legend is not None:
        matplotlib.rcParams['font.size'] = 12.0
    else:
        matplotlib.rcParams['font.size'] = 20.0
    ind = np.arange(len(values[0][0]))
    width = (float(1)-distance)/float(len(values))
    barColors=colors.get_bar_colors(len(values))
    myRangeColors=colors.get_bar_range_colors(len(values))
    if imgRanges is None:
        if (legendIntervalls is None) and (legend is None):
            fig = plt.figure(figsize=(8.0, 4.0))
            ax = fig.add_axes([0.15, 0.3, 0.8, 0.65])
        else:
            fig = plt.figure(figsize=(16.0, 4.0))
            ax = fig.add_axes([0.1, 0.3, 0.8, 0.6])
    else:
        fig = plt.figure(figsize=(8.0, 4.0))
        ax = fig.add_axes(imgRanges)
    rects = []
    difRec = None
    for i in xrange(len(values)):
        if (barColors[i]) == None:
            print("myColor [" + str(i) + "] was None")
        graph = ax.bar(ind + (float(i) * width), values[i][0], width, color=barColors[i])
        if legend != None:
            rects.append(graph[0])
        graph = ax.bar(ind + (float(i) * width), values[i][1], width, color=myRangeColors[i], bottom=values[i][0])
        difRec = graph[0]
    errorGraphs = []
    if intervalls != None:
        myErrorColors = colors.get_error_colors(len(intervalls))
        errorPos = width / float(len(intervalls))
        for i in xrange(len(intervalls)):
            graph = ax.bar(ind, values[0][0], 0, color=myErrorColors[i], linewidth=0)
            if legendIntervalls != None:
                rects.append(graph[0])
            myErrorGraphes = []
            intervall=intervalls[i]
            for j in xrange(len(intervall)):
                myErrorGraphes.append(ax.errorbar(ind + (float(j) * width) + (float(i) * errorPos) + (errorPos / float(2)), values[j][2], yerr=intervall[j], fmt=None, ecolor=(0, 0, 0)))
            errorGraphs.append(myErrorGraphes)
    ax.set_xticks(ind + ((float(1) - distance)/float(2)))
    ax.set_ylim(bottom=0)
    if xticks != None:
        ax.set_xticklabels(xticks, rotation=rotation)
    if ylabel != None:
        ax.set_ylabel(ylabel)
    if xlabel != None:
        ax.set_xlabel(xlabel)
    if label != None:
        ax.set_title(label)
    allLegend = []
    if legend != None:
        allLegend = legend
    if legendIntervalls != None:
        allLegend += legendIntervalls
    if difLegend is not None:
        rects.append(difRec)
        allLegend.append(difLegend)
    if (legend != None) or (legendIntervalls != None):
        fig.legend(rects, allLegend, 'lower center', ncol=6)
    if ytickfct is not None:
        yticks = ax.get_yticks()
        yticklabels = []
        for ytick in yticks:
            yticklabels.append(ytickfct(ytick))
        ax.set_yticklabels(yticklabels)
    if filename != None:
        fig.savefig(filename)
    return fig

def draw_image_bar_line_with_points(barValues, filename=None, xticks=None, barIntervalls=None, xlabel=None, barYlabel=None, lineYlabel=None, label=None, legend=None, difLegend=None, legendIntervalls=None, lineLegend=None, distance=0.1, imgRanges=None, colors=BWColorChooser(), ytickfct=None, rotation=45):
    if legend is not None:
        matplotlib.rcParams['font.size'] = 12.0
    else:
        matplotlib.rcParams['font.size'] = 20.0
    ind = np.arange(len(barValues[0][0]))
    width = (float(1)-distance)/float(len(barValues))
    myColors=colors.get_bar_colors(len(barValues))
    myRangeColors=colors.get_bar_range_colors(len(barValues))

    if imgRanges is None:
        if (lineLegend is None) and (legendIntervalls is None) and (legend is None):
            fig = plt.figure(figsize=(8.0, 4.0))
            ax = fig.add_axes([0.15, 0.2, 0.7, 0.75])
        else:
            fig = plt.figure(figsize=(16.0, 4.0))
            ax = fig.add_axes([0.15, 0.3, 0.7, 0.6])
    else:
        fig = plt.figure(figsize=(8.0, 4.0))
        ax = fig.add_axes(imgRanges)
    ax2 = ax.twinx()
    rects = []
    lines = []
    difRec = None
    line = None
    for i in xrange(len(barValues)):
        if (myColors[i]) == None:
            print("myColor [" + str(i) + "] was None")
        if len(barValues[i]) > 3:
            line, = ax2.plot(ind + ((float(i) + 0.5) * width), barValues[i][3], "wD")
            lines.append(line)
            ax2.errorbar(ind + ((float(i) + 0.5) * width), barValues[i][3], yerr=barValues[i][4], fmt=None, ecolor=(0,0,0))
        graph = ax.bar(ind + (float(i) * width), barValues[i][0], width, color=myColors[i])
        if legend != None:
            rects.append(graph[0])
        graph = ax.bar(ind + (float(i) * width), barValues[i][1], width, color=myRangeColors[i], bottom=barValues[i][0])
        difRec = graph[0]
    errorGraphs = []
    if barIntervalls != None:
        myErrorColors = colors.get_error_colors(len(barIntervalls))
        errorPos = width / float(len(barIntervalls))
        for i in xrange(len(barIntervalls)):
            myErrorGraphes = []
            intervall=barIntervalls[i]
            for j in xrange(len(intervall)):
                myErrorGraphes.append(ax.errorbar(ind + (float(j) * width) + (float(i) * errorPos) + (errorPos / float(2)), barValues[j][2], yerr=intervall[j], fmt=None, ecolor=myErrorColors[i]))
            errorGraphs.append(myErrorGraphes)
    ax2.set_ylim(bottom=0)
    ax.set_ylim(bottom=0)
    ax.set_xticks(ind + ((float(1) - distance)/float(2)))
    if xticks is not None:
        ax.set_xticklabels(xticks, rotation=rotation)
    if barYlabel is not None:
        ax.set_ylabel(barYlabel)
    if lineYlabel is not None:
        ax2.set_ylabel(lineYlabel)
    if xlabel is not None:
        ax.set_xlabel(xlabel)
    if label is not None:
        ax.set_title(label)
    allLegend = []
    if legend is not None:
        allLegend = legend
    if legendIntervalls is not None:
        allLegend += legendIntervalls
    if difLegend is not None:
        rects.append(difRec)
        allLegend.append(difLegend)
    if lineLegend is not None:
        rects.append(line)
        allLegend.append(lineLegend)
    if (legend is not None) or (legendIntervalls is not None):
        fig.legend(rects, allLegend, 'lower center', ncol=8)
    if ytickfct is not None:
        yticks = ax.get_yticks()
        yticklabels = []
        for ytick in yticks:
            yticklabels.append(ytickfct(ytick))
        ax.set_yticklabels(yticklabels)
        yticks = ax2.get_yticks()
        yticklabels = []
        for ytick in yticks:
            yticklabels.append(ytickfct(ytick))
        ax2.set_yticklabels(yticklabels)
    if filename is not None:
        fig.savefig(filename)
    return fig

def draw_image_bar_multi(values, filename=None, xticks=None, xlabel=None, ylabel=None, label=None, legend=None, difLegend=None, distance=0.1, imgRanges=None, colors=BWColorChooser(), ytickfct=None, rotation=45):
    if legend is not None:
        matplotlib.rcParams['font.size'] = 12.0
    else:
        matplotlib.rcParams['font.size'] = 20.0
    ind = np.arange(len(values[0][0]))
    width = (float(1)-distance)/float(len(values))
    myColors=colors.get_bar_colors(len(values))
    myRangeColors=colors.get_multi_range_colors(2, len(values))
    if imgRanges is None:
        if (legend is None):
            fig = plt.figure(figsize=(8.0, 4.0))
            ax = fig.add_axes([0.2, 0.2, 0.75, 0.75])
        else:
            fig = plt.figure(figsize=(16.0, 4.0))
            ax = fig.add_axes([0.1, 0.3, 0.8, 0.6])
    else:
        fig = plt.figure(figsize=(8.0, 4.0))
        ax = fig.add_axes(imgRanges)
    rects = []
    difRec = []
    first = True
    for i in xrange(len(values)):
        bottoms = [0] * len(values[i][0])
        valueline = values[i][0]
        if (myColors[i]) == None:
            print("myColor [" + str(i) + "] was None")
        graph = ax.bar(ind + (float(i) * width), values[i][0], width, color=myColors[i])
        if legend != None:
            rects.append(graph[0])
        for j in xrange(1, len(values[i])):
            for k in xrange(len(valueline)):
                bottoms[k] += valueline[k]
            valueline = values[i][j]
            graph = ax.bar(ind + (float(i) * width), valueline, width, color=myRangeColors[j-1][i], bottom=bottoms)
            if first:
                difRec.append(graph[0])
        first = False
    ax.set_ylim(bottom=0)
    ax.set_xticks(ind + ((float(1) - distance)/float(2)))
    if xticks != None:
        ax.set_xticklabels(xticks, rotation=rotation)
    if ylabel != None:
        ax.set_ylabel(ylabel)
    if xlabel != None:
        ax.set_xlabel(xlabel)
    if label != None:
        ax.set_title(label)
    allLegend = []
    if legend != None:
        allLegend = legend
    if difLegend is not None:
        rects += difRec
        allLegend += difLegend
    if (legend != None) or (difLegend != None):
        fig.legend(rects, allLegend, 'lower center', ncol=6)
    if ytickfct is not None:
        yticks = ax.get_yticks()
        yticklabels = []
        for ytick in yticks:
            yticklabels.append(ytickfct(ytick))
        ax.set_yticklabels(yticklabels)
    if filename != None:
        fig.savefig(filename)
    return fig
