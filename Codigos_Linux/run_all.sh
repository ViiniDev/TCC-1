#!/bin/bash

echo "program,N,dist,K,tempo,cycles,instructions,cache_misses" > resultados.csv

PROGRAMS=("conv_linear" "conv_malloc" "dgemm")
NS=(500 1000)
DISTS=(0 1 2)
KS=(3 5 7)

for PROG in "${PROGRAMS[@]}"
do
  for N in "${NS[@]}"
  do
    if [ "$PROG" == "dgemm" ]; then
      for REP in {1..5}
      do
        # Executa UMA vez só com perf
        OUTPUT=$(LC_ALL=C perf stat -e cycles,instructions,cache-misses ./$PROG $N 2>&1)

        # Extrai tempo do próprio programa
        TEMPO=$(echo "$OUTPUT" | sed -n 's/.*Tempo=\([0-9.]*\).*/\1/p')

        # Extrai métricas do perf
        CYCLES=$(echo "$OUTPUT" | grep " cycles" | awk '{print $1}')
        INSTR=$(echo "$OUTPUT" | grep " instructions" | awk '{print $1}')
        CACHE=$(echo "$OUTPUT" | grep " cache-misses" | awk '{print $1}')

        echo "$PROG,$N,-,-,$TEMPO,$CYCLES,$INSTR,$CACHE" >> resultados.csv
      done
    else
      for DIST in "${DISTS[@]}"
      do
        for K in "${KS[@]}"
        do
          for REP in {1..5}
          do
            # Executa UMA vez só com perf
            OUTPUT=$(LC_ALL=C perf stat -e cycles,instructions,cache-misses ./$PROG $N $DIST $K 2>&1)

            # Extrai tempo
            TEMPO=$(echo "$OUTPUT" | sed -n 's/.*Tempo=\([0-9.]*\).*/\1/p')

            # Extrai métricas
            CYCLES=$(echo "$OUTPUT" | grep " cycles" | awk '{print $1}')
            INSTR=$(echo "$OUTPUT" | grep " instructions" | awk '{print $1}')
            CACHE=$(echo "$OUTPUT" | grep " cache-misses" | awk '{print $1}')

            echo "$PROG,$N,$DIST,$K,$TEMPO,$CYCLES,$INSTR,$CACHE" >> resultados.csv
          done
        done
      done
    fi
  done
done
