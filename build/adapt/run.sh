#!/bin/bash
echo starting CRUSH_100_3.disks
CPPDistTest -dt CRUSH -Dn 100 -kn 3 -st -en 1000000 -Mof crush_100_3.run.out.memdata > crush_100_3.run.out.std 2> crush_100_3.run.out.sterr &
pid=$!
state=0
rm crush_100_3.run.out.mem
touch crush_100_3.run.out.mem
while [ ${state} == "0" ] ; do
        ps -p ${pid} o rss h >> crush_100_3.run.out.mem
        state=$?
        sleep 1
done
echo succeeded crush_100_3.disks
