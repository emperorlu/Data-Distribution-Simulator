#! /usr/bin/python

# To change this template, choose Tools | Templates
# and open the template in the editor.

__author__="fermat"
__date__ ="$21.09.2010 13:25:29$"

import sys
import os
import os.path

if __name__ == "__main__":
    i = 1;
    datafiles = []
    dirname = "."
    pre = ""
    post = ""
    i = 1
    while i < len(sys.argv):
        if sys.argv[i] == "-d":
            i += 1
            if i == len(sys.argv):
                print "Option -d needs a parameter"
                sys.exit()
            dirname = sys.argv[i]
        elif sys.argv[i] == "--pre":
            i += 1
            if i == len(sys.argv):
                print "Option --pre needs a parameter"
                sys.exit()
            pre = sys.argv[i]
        elif sys.argv[i] == "--post":
            i += 1
            if i == len(sys.argv):
                print "Option --post needs a parameter"
                sys.exit()
            post = sys.argv[i]
        else:
            datafiles.append(sys.argv[i])
        i += 1

    if not os.path.isdir(dirname):
        print "call unpack_test directory filename [filename2, ...]"
        print dirname + " is not an existing directory"
        sys.exit()

    testnum = 0
    for datafile in datafiles:
        if os.path.isfile(datafile):
            fin = open(datafile, "r")
            for line in fin:
                line = line.strip()
                if (not line.startswith("#")) and (len(line.split(";")) > 1):
                    splited = line.split(";")
                    if splited[0] == "c":
                        fout = open(dirname + "/" + pre + str(testnum).zfill(5) + "-compile-"+ splited[1] + "-" + datafile + post, "w")
                        fout.write(line + "\n")
                        fout.close()
                        testnum += 1
                    elif splited[0] == "t":
                        fout = open(dirname + "/" + pre + str(testnum).zfill(5) + "-test-"+ splited[1] + "-" + datafile + post, "w")
                        fout.write(line + "\n\n")
                        fout.close()
                        testnum += 1
                    else:
                        print "WARN: ignoring unknown testline: " + line
                        print "      in file " + datafile
        else:
            print "WARN: " + datafile + " is not a file"
    