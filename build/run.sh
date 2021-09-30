#!/bin/bash
echo starting CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0
CPPDistTest -dt CRUSH -Tn 1 -Dif 8D_hom.disks -kn 1 -of CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.csv -en 1000000 -Mof CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.memdata > CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.std 2> CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.sterr &
pid=$!
state=0
rm CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.mem
touch CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.mem
while [ ${state} == "0" ] ; do
        ps -p ${pid} o rss h >> CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0.run.out.mem
        state=$?
        sleep 1
done
echo succeeded CreateAndRun_CRUSH_1K_1T-8D_hom.disks-0
