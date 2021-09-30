#!/bin/bash
d=(10000 100000 1000000 10000000 100000000)
k=(1 3 5 7 9)
for i in $(seq 0 ${#d[@]})
do
    echo d = $i 
    en=${d[$i]}
    ./CPPDistTest -dt CRUSH -Dn 200 -kn 3 -en $en -st  && python3 test.py > result/crush_$en.txt
    ./CPPDistTest -dt RandSlice -Dn 200 -kn 3 -en $en -st  && python3 test.py > result/randslice_$en.txt
    ./CPPDistTest -dt RedundantShare -Dn 200 -kn 3 -en $en -st  && python3 test.py > result/Kinesis_$en.txt
    ./CPPDistTest -dt Share -Dn 200 -kn 3 -en $en -st  && python3 test.py > result/DMORP_$en.txt
done

for i in $(seq 0 ${#k[@]})
do
    echo k = $i 
    kn=${k[$i]}
    ./CPPDistTest -dt CRUSH -Dn 200 -kn $kn -en 100000 -st  && python3 test.py > result/crush_k_$kn.txt
    ./CPPDistTest -dt RandSlice -Dn 200 -kn $kn -en 100000 -st  && python3 test.py > result/randslice_k_$kn.txt
    ./CPPDistTest -dt RedundantShare -Dn $kn -kn 3 -en 100000 -st  && python3 test.py > result/Kinesis_k_$kn.txt
    ./CPPDistTest -dt Share -Dn 200 -kn $kn -en 100000 -st  && python3 test.py > result/DMORP_k_$kn.txt
done
