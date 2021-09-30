#!/usr/bin/python
import csv
import sys

def read_csv_file(filename, rotated=False, cellDelimiter=';'):
    f = open(filename)
    r = csv.reader(f, delimiter=cellDelimiter)
    data = [[c for c in row] for row in r]
    f.close()
    if rotated:
        rdata = []
        for i in xrange(len(data[0])):
            rdata.append([])
        for i in xrange(len(data)):
            for j in xrange(len(data[i])):
                rdata[j].append(data[i][j])
        data = rdata
    return data

def store_csv_file(filename, data, rotated=False, cellDelimiter=';'):
    if (rotated):
        oData = []
        for i in xrange(len(data[0])):
            oData.append([])
        for i in xrange(len(data)):
            for j in xrange(len(data[i])):
                oData[j].append(data[i][j])
    else:
        oData = data
    f = open(filename, "w")
    w = csv.writer(f, delimiter=cellDelimiter);
    for row in oData:
        w.writerow(row)
    f.close();

if __name__ == "__main__":
    print "Hello World"
    if len(sys.argv) < 2:
        print "No Options..."
    else:
        if sys.argv[1] == "rotate":
            if len(sys.argv) != 4:
                print "rotate needs an in and an outfile"
            else:
                print "rotating " + sys.argv[2] + " into " + sys.argv[3]
                store_csv_file(sys.argv[3], read_csv_file(sys.argv[2], True))
        else:
            print "Unknown action: " + sys.argv[1]
            