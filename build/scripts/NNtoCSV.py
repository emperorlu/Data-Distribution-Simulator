#!/usr/bin/python

# To change this template, choose Tools | Templates
# and open the template in the editor.

import sys
import xml.sax
import csv
import xml.sax.handler

class xmlHandler(xml.sax.handler.ContentHandler):
    def __init__(self, outfile):
        self.w = csv.writer(outfile, delimiter=';')
        self.pos = None
        
    def startElement(self, name, attrs):
        if name == "diskPosition":
            if self.pos != None:
                raise "diskPosition Element in diskPosition Element !?!"
            myPos = attrs.getValue("position")
            if (myPos == None):
                raise "No position attribute in diskPosition Element !?!"
            self.pos = myPos
        elif name == "Disk":
            if self.pos == None:
                raise "Disk without position !?!"
            myId = attrs.getValue("id")
            if myId == None:
                raise "Disk without id !?!"
            myCapacity = attrs.getValue("capacity")
            if myCapacity == None:
                raise "Disk without capacity !?!"
            self.w.writerow([self.pos, myId, myCapacity])
            self.pos = None

if __name__ == "__main__":
    print "Will convert a NearestNeighbour dist file to csv";
    if len(sys.argv) != 3:
        print "Needing input and putput file names, nothing else..."
    else:
        parser = xml.sax.make_parser()
        f = open(sys.argv[2], "w")
        parser.setContentHandler(xmlHandler(f))
        parser.parse(sys.argv[1])
        f.close()
