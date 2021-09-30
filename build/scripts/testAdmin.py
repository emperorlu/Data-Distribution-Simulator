#!/usr/bin/python
# To change this template, choose Tools | Templates
# and open the template in the editor.

import os
import glob
import time
import subprocess
import os.path
import rpyc

__author__="fermat"
__date__ ="$13.09.2010 15:55:57$"

testDescDir = "/home/fermat/distTest/descriptions/"
testResultDir = "/home/fermat/distTest/results/"

class Server:
    def __init__(self, name):
        self.available = True
        self.name = name
        self.test_running = False
        self.placedTest = None
        self.placedTestName = None
        self.needCompile = False
        self.placedAsyncResult = None
        self.con = rpyc.connect(name, 20000)
        self._place_test = rpyc.async(self.con.root.run_test)

    def ready(self):
        if self.available:
            if self.needCompile:
                self.con.root.compile()
                self.needCompile = False
            return True
        if self.placedAsyncResult.ready:
            scpCommand = ["scp", self.name + ":/home/fermat/distTest/test/" + self.placedTestName + ".tar.bz2", testResultDir]
            subprocess.call(scpCommand)
            self.available = True
            self.test_running = False
            self.placedTest = None
            self.placedTestName = None
            if self.needCompile:
                self.con.root.compile()
                self.needCompile = False
            return True
        if self.placedAsyncResult.error:
            print "WARN: test " + str(self.placedTest) + " returned error"
            self.available = True
            self.test_running = False
            self.placedTest = None
            self.placedTestName = None
            if self.needCompile:
                self.con.root.compile()
                self.needCompile = False
            return True
        if self.placedAsyncResult.expired:
            print "WARN: test " + str(self.placedTest) + " expired"
            self.available = True
            self.test_running = False
            self.placedTest = None
            self.placedTestName = None
            if self.needCompile:
                self.con.root.compile()
                self.needCompile = False
            return True
        return False

    def place_test(self, testname, test):
        if not self.ready():
            return False
        self.available = False
        self.test_running = True
        self.placedTest = test
        self.placedTestName = testname
        if self.needCompile:
                self.con.root.compile()
                self.needCompile = False
        self.placedAsyncResult = self._place_test(self.placedTestName, self.placedTest)
        return True

if __name__ == "__main__":
    if not os.path.exists(testDescDir):
        os.makedirs(testDescDir)
    if not os.path.exists(testResultDir):
        os.makedirs(testResultDir)
    servers = []
    servers.append(Server("sprite"))
    servers.append(Server("drpepper"))
    servers.append(Server("redbull"))
    servers.append(Server("bitterlemon"))
    servers.append(Server("bionade"))

    print "place tests in " + testDescDir
    while (True):
        host_running = False
        all_host_running = True
        for server in servers:
            if not server.ready():
                host_running = True
            else:
                all_host_running = False
        while all_host_running:
            print "INFO: Sleeping for 10 secs awaiting Server"
            time.sleep(10)
            host_running = False
            for server in servers:
                if not server.ready():
                    host_running = True
                else:
                    all_host_running = False
        testfiles = glob.glob(testDescDir + "*")
        while len(testfiles) == 0:
            time.sleep(10)
            if host_running:
                host_running = False;
                for server in servers:
                    if not server.ready():
                        host_running = True
            testfiles = glob.glob(testDescDir + "*")
        testfiles = sorted(testfiles)
        testfile = testfiles[0]
        print "DEBUG: Processing File " + testfile
        f = open(testfile, "r")
        for line in f:
            line = line.strip()
            if line.startswith("#"):
                test = [line, ]
            else:
                test = line.split(";")
            if len(test) > 1:
                kind = test[0]
                if kind == "c":
                    cServer = test[1]
                    if cServer == "ALL":
                        for server in servers:
                            server.needCompile = True
                    else:
                        found = False
                        for server in servers:
                            if server.name == cServer:
                                found = true
                                server.needCompile = True
                        if not found:
                            print "WARN: Server " + cServer + " unknown, so it can not compile"
                elif kind == "t":
                    testName = test[1]
                    testDef = test[2]
                    placed = False
                    while not placed:
                        for server in servers:
                            if not placed:
                                if server.ready():
                                    print "DEBUG: Will place Test " + testName + " on Server " + server.name
                                    placed = True
                                    server.place_test(testName, testDef)
                        if not placed:
                            print "INFO: sleeping 10 secs awaitng server while processing " + testfile
                            time.sleep(10)
        os.remove(testfile)
        print "DEBUG: Done processing file " + testfile
