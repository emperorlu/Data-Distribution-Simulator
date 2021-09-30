#!/bin/bash
d=(100 200 300 400 500)
e=(100000000)
#(10000 100000 1000000 10000000 100000000)
#f=("0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "a" "b" "c" "e" "e" "f")
f="NearestNeighbour CRUSH RandSlice RedundantShare Share"

#for i in $(seq 0 ${#d[@]})
#do
#    echo starting d = $i
#    dn=${d[$i]}
#    for fn in $f
#    do
#        echo starting f = $fn, d = $i
#        memfile=Dmem_{$fn}_$dn.memdata
#        timefile=Dtime_{$fn}_$dn.std
#        CPPDistTest -dt $fn -Dn $dn -kn 3 -st -en 1000000 -Mof $memfile > $timefile  && python3 ca.py $memfile $timefile > result/D_{$fn}_{$dn}.txt
#    done
#done

for i in $(seq 0 ${#e[@]})
do
    echo starting e = $i
    en=${e[$i]}
    for fn in $f
    do
        memfile=Emem_{$fn}_$en.memdata
        timefile=Etime_{$fn}_$en.std
        #python3 ca.py $memfile $timefile > result/E_{$fn}_{$en}.txt
        CPPDistTest -dt $fn -Dn 200 -kn 3 -st -en $en -Mof $memfile > $timefile  && python3 ca.py $memfile $timefile > result/E_{$fn}_{$en}.txt
    done
done
