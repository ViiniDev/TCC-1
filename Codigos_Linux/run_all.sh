#!/bin/bash

echo "program,N,dist,K,tempo,cycles,instructions,cache_references,cache_misses" > resultados.csv

PROGRAMS=("conv_linear" "conv_malloc" "dgemm")

NS=(512 1024 2048)

DISTS=(0 1 2)

KS=(3 5 7)

BS_VALUES=(8 16 32 64 128)

for PROG in "${PROGRAMS[@]}"
do

for N in "${NS[@]}"
do

if [ "$PROG" == "dgemm" ]; then

for BS in "${BS_VALUES[@]}"
do

for REP in {1..10}
do

echo "Rodando $PROG N=$N BS=$BS REP=$REP"

OUTPUT=$(LC_ALL=C perf stat -e cycles,instructions,cache-references,cache-misses ./$PROG $N $BS 2>&1)

TEMPO=$(echo "$OUTPUT" | sed -n 's/.*Tempo=\([0-9.]*\).*/\1/p')

CYCLES=$(echo "$OUTPUT" | grep " cycles" | awk '{print $1}' | tr -d ',')
INSTR=$(echo "$OUTPUT" | grep " instructions" | awk '{print $1}' | tr -d ',')
CACHE_REF=$(echo "$OUTPUT" | grep " cache-references" | awk '{print $1}' | tr -d ',')
CACHE_MISS=$(echo "$OUTPUT" | grep " cache-misses" | awk '{print $1}' | tr -d ',')

if [ -z "$TEMPO" ]; then
TEMPO=0
fi

echo "$PROG,$N,-,$BS,$TEMPO,$CYCLES,$INSTR,$CACHE_REF,$CACHE_MISS" >> resultados.csv

done
done

else

for DIST in "${DISTS[@]}"
do

for K in "${KS[@]}"
do

for REP in {1..10}
do

echo "Rodando $PROG N=$N DIST=$DIST K=$K REP=$REP"

OUTPUT=$(LC_ALL=C perf stat -e cycles,instructions,cache-references,cache-misses ./$PROG $N $DIST $K 2>&1)

TEMPO=$(echo "$OUTPUT" | sed -n 's/.*Tempo=\([0-9.]*\).*/\1/p')

CYCLES=$(echo "$OUTPUT" | grep " cycles" | awk '{print $1}' | tr -d ',')
INSTR=$(echo "$OUTPUT" | grep " instructions" | awk '{print $1}' | tr -d ',')
CACHE_REF=$(echo "$OUTPUT" | grep " cache-references" | awk '{print $1}' | tr -d ',')
CACHE_MISS=$(echo "$OUTPUT" | grep " cache-misses" | awk '{print $1}' | tr -d ',')

if [ -z "$TEMPO" ]; then
TEMPO=0
fi

echo "$PROG,$N,$DIST,$K,$TEMPO,$CYCLES,$INSTR,$CACHE_REF,$CACHE_MISS" >> resultados.csv

done
done
done

fi

done
done