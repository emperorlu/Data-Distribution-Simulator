#! /usr/bin/python

import os
import os.path
import glob

# To change this template, choose Tools | Templates
# and open the template in the editor.

__author__="fermat"
__date__ ="$02.08.2010 13:34:47$"

if __name__ == "__main__":
    print "Hello World";
    imagepath = "/Users/fermat/Uni/Dissertation/Messergebnisse/images/"
    albumpath = "/Users/fermat/tmp/distpaperalbum/"
    images = glob.glob(imagepath + "*.png")
    f = open(albumpath + "album.html", "w")
    f.write("""<html>
<head><title>Album</title></head>
<body>
""")
    for image in images:
        entries = image.split("/")
        filename = entries[len(entries) - 1]
        f.write('<img src="' + image + '" width="70%"/><br/>\n')
        f.write(filename + "<br/>\n")
    f.write("""</body>
</html>
""")