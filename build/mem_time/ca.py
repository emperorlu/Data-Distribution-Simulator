import sys 

#def main(argv):
memfiles = sys.argv[1]
timefiles = sys.argv[2]
#"crush_100_3.run.out.memdata"
#print(memfiles)
file = open(memfiles)
for line in file:
    if line.startswith("VmRSS:"):
        entries = line.split()
        mem = int(entries[len(entries) - 2])
    if line.startswith("VmPeak:"):
        entries = line.split()
        maxmem = int(entries[len(entries) - 2])

print("mem: ",mem," ;maxmem:",maxmem)

file2 = open(timefiles)
for line in file2:
    if line.startswith("Sum time of placing single Extents:"):
        entries = line.split(":")
        time = int(entries[1])

print("time:",time)
        
