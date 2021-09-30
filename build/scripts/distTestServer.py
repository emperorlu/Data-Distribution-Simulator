#! /usr/bin/python

# To change this template, choose Tools | Templates
# and open the template in the editor.

from rpyc.utils.server import ThreadedServer
import rpyc
import subprocess
import os
import shutil
import os.path
from createDistributor import *

__author__="fermat"
__date__ ="$13.09.2010 09:37:49$"

basepath = "/home/fermat/distTest/test/"

class DistTestService(rpyc.Service):
    def exposed_compile(self):
        print "DEBUG: starting compile"
        os.chdir("/home/fermat")
        result = subprocess.call(["./compile", ])
        os.chdir(basepath)
        print "DEBUG: succeeded compile"
        return result

    def exposed_exec(self, line):
        return os.system(line)

    def exposed_run_test(self, filename, test):
        print "DEBUG: starting test " + filename
        print "DEBUG: to execute: " + test
        os.chdir(basepath)
        testpath = basepath + filename + "/ausgaben/";
        if os.path.exists(testpath):
            shutil.rmtree(testpath)
        if os.path.exists(basepath + filename + ".tar.bz2"):
            os.remove(basepath + filename + ".tar.bz2")
        os.makedirs(testpath)
        os.chdir(testpath)
        eval(test)
        os.chdir(basepath + filename)
        createTar = ["tar", "-cjf", "../" + filename + ".tar.bz2", "ausgaben"]
        subprocess.call(createTar)
        os.chdir(basepath)
        print "DEBUG: succeded test " + filename

if __name__ == "__main__":
    if not os.path.exists(basepath):
        os.makedirs(basepath)
    print "starting server on port 20000"
    s = ThreadedServer(DistTestService, port = 20000)
    s.start()
