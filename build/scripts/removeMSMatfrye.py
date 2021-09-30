#! /usr/bin/python
import csv
import sys

# To change this template, choose Tools | Templates
# and open the template in the editor.

__author__="fermat"
__date__ ="$17.05.2010 11:50:34$"

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "This skript needs an in and an outfile"
    else:
        inFileName = sys.argv[1]
        outFileName = sys.argv[2]
        inFile = open(inFileName)
        outFile = open(outFileName, "w")
        inCSV = csv.reader(inFile, delimiter=',')
        outCSV = csv.writer(outFile, delimiter=';', lineterminator='\n');
        i = 0
        for row in inCSV:
            if (i < 3):
                outCSV.writerow(row)
                i += 1
            else:
                outRow = []
                j = 0
                for cell in row:
                    if j == 0:
                        j += 1
                    else:
                        data = str(cell)
                        if j == 1 and data[-2:] == "ms":
                            data = data[:-2].strip()
                        outRow.append(data)
                        j += 1
                i += 1
                outCSV.writerow(outRow)
        inFile.close()
        outFile.close()
