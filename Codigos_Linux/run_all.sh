#!/bin/bash

set -u

CSV="resultados.csv"

# Coluna K:
# - convolucoes: tamanho do kernel
# - DGEMM: tamanho do bloco (BS)
echo "program,mode,approx_type,N,dist,K,seed,tempo,cycles,instructions,cache_references,cache_misses,checksum,error_abs_mean,error_rel_mean,rmse,error_max" > "$CSV"

CONV_EXACT_PROGRAMS=("conv_linear" "conv_malloc")
CONV_APPROX_PROGRAMS=("conv_linear_approx" "conv_malloc_approx")
CONV_APPROX_TYPES=("float" "skip_kernel")
DGEMM_APPROX_TYPES=("float" "skip_k")

NS=(512 1024 2048)
DISTS=(0 1 2)
KS=(3 5 7)
BS_VALUES=(8 16 32 64 128)
REPETITIONS=10
SEED_BASE=12345

extract_metric() {
    local output="$1"
    local pattern="$2"
    local value

    value=$(echo "$output" | sed -n "$pattern" | head -n 1)

    if [ -z "$value" ]; then
        value=0
    fi

    echo "$value"
}

extract_last_metric() {
    local output="$1"
    local pattern="$2"
    local value

    value=$(echo "$output" | sed -n "$pattern" | tail -n 1)

    if [ -z "$value" ]; then
        value=0
    fi

    echo "$value"
}

extract_perf_counter() {
    local output="$1"
    local event="$2"
    local value

    value=$(echo "$output" | grep " $event" | awk '{print $1}' | tr -d ',' | head -n 1)

    if [ -z "$value" ] || [ "$value" = "<not" ]; then
        value=0
    fi

    echo "$value"
}

write_row() {
    local program="$1"
    local mode="$2"
    local approx_type="$3"
    local n="$4"
    local dist="$5"
    local k="$6"
    local seed="$7"
    local output="$8"

    local tempo
    local cycles
    local instructions
    local cache_references
    local cache_misses
    local checksum
    local error_abs_mean
    local error_rel_mean
    local rmse
    local error_max

    tempo=$(extract_metric "$output" 's/.*Tempo=\([0-9.eE+-]*\).*/\1/p')
    cycles=$(extract_perf_counter "$output" "cycles")
    instructions=$(extract_perf_counter "$output" "instructions")
    cache_references=$(extract_perf_counter "$output" "cache-references")
    cache_misses=$(extract_perf_counter "$output" "cache-misses")
    checksum=$(extract_last_metric "$output" 's/^Checksum \([0-9.eE+-]*\).*/\1/p')
    error_abs_mean=$(extract_last_metric "$output" 's/^ErrorAbsMean \([0-9.eE+-]*\).*/\1/p')
    error_rel_mean=$(extract_last_metric "$output" 's/^ErrorRelMean \([0-9.eE+-]*\).*/\1/p')
    rmse=$(extract_last_metric "$output" 's/^RMSE \([0-9.eE+-]*\).*/\1/p')
    error_max=$(extract_last_metric "$output" 's/^ErrorMax \([0-9.eE+-]*\).*/\1/p')

    if [ "$mode" = "exact" ]; then
        error_abs_mean=0
        error_rel_mean=0
        rmse=0
        error_max=0
    fi

    echo "$program,$mode,$approx_type,$n,$dist,$k,$seed,$tempo,$cycles,$instructions,$cache_references,$cache_misses,$checksum,$error_abs_mean,$error_rel_mean,$rmse,$error_max" >> "$CSV"
}

run_with_perf() {
    LC_ALL=C perf stat -e cycles,instructions,cache-references,cache-misses "$@" 2>&1
}

for N in "${NS[@]}"; do
    for DIST in "${DISTS[@]}"; do
        for K in "${KS[@]}"; do
            for REP in $(seq 1 "$REPETITIONS"); do
                SEED=$((SEED_BASE + REP))

                for PROG in "${CONV_EXACT_PROGRAMS[@]}"; do
                    echo "Rodando $PROG exact N=$N DIST=$DIST K=$K REP=$REP SEED=$SEED"
                    OUTPUT=$(run_with_perf "./$PROG" "$N" "$DIST" "$K" "$SEED")
                    write_row "$PROG" "exact" "none" "$N" "$DIST" "$K" "$SEED" "$OUTPUT"
                done

                for PROG in "${CONV_APPROX_PROGRAMS[@]}"; do
                    for APPROX in "${CONV_APPROX_TYPES[@]}"; do
                        echo "Rodando $PROG approx=$APPROX N=$N DIST=$DIST K=$K REP=$REP SEED=$SEED"
                        PERF_OUTPUT=$(run_with_perf "./$PROG" "$N" "$DIST" "$K" "$APPROX" "$SEED" "measure")
                        ERROR_OUTPUT=$("./$PROG" "$N" "$DIST" "$K" "$APPROX" "$SEED" "compare" 2>&1)
                        OUTPUT="$PERF_OUTPUT
$ERROR_OUTPUT"
                        write_row "$PROG" "approx" "$APPROX" "$N" "$DIST" "$K" "$SEED" "$OUTPUT"
                    done
                done
            done
        done
    done
done

for N in "${NS[@]}"; do
    for BS in "${BS_VALUES[@]}"; do
        for REP in $(seq 1 "$REPETITIONS"); do
            SEED=$((SEED_BASE + REP))

            echo "Rodando dgemm exact N=$N BS=$BS REP=$REP SEED=$SEED"
            OUTPUT=$(run_with_perf ./dgemm "$N" "$BS" "$SEED")
            write_row "dgemm" "exact" "none" "$N" "-" "$BS" "$SEED" "$OUTPUT"

            for APPROX in "${DGEMM_APPROX_TYPES[@]}"; do
                echo "Rodando dgemm_approx approx=$APPROX N=$N BS=$BS REP=$REP SEED=$SEED"
                PERF_OUTPUT=$(run_with_perf ./dgemm_approx "$N" "$BS" "$APPROX" "$SEED" "measure")
                ERROR_OUTPUT=$(./dgemm_approx "$N" "$BS" "$APPROX" "$SEED" "compare" 2>&1)
                OUTPUT="$PERF_OUTPUT
$ERROR_OUTPUT"
                write_row "dgemm_approx" "approx" "$APPROX" "$N" "-" "$BS" "$SEED" "$OUTPUT"
            done
        done
    done
done

echo "Resultados salvos em $CSV"
