#!/bin/bash
d=(100 101 105 110 120 130 150 180 200)
e=1000000
#f="NearestNeighbour CRUSH RandSlice RedundantShare Share"
#f="RandSlice"

for i in $(seq 0 ${#d[@]})
do
    echo starting d = $i
    dn=${d[$i]}
    echo begin dn = $dn
    CPPDistTest -dt RandSlice -Dn $dn -en $e -kn 3 -st -dof $diskfile -Tn 8
    #for fn in $f
    #do
    #    echo begin $dn
    #    diskfile={$fn}_$dn.disk
    #    CPPDistTest -dt $fn -Dn $dn -en $e -kn 3 -st -dof $diskfile -Tn 8
    #done
done
