#!/bin/bash
d=(101 105 110 120 130 150 180 200)
e=1000000
f="NearestNeighbour CRUSH RandSlice RedundantShare Share"

for i in $(seq 0 ${#d[@]})
do
    echo starting d = $i
    dn=${d[$i]}
    for fn in $f
    do
        diskfile={$fn}_$dn.disklist
        disk={$fn}_$dn.disk
        mofile=result/M_{$fn}_$dn.csv
        echo begin $fn $dn
        CPPDistCompare -dlf $diskfile -ocf $mofile -en 1000000 -Tn 8
        python3 test.py >> $mofile
    done
done
