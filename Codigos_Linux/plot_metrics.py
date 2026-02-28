import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

# Carrega o CSV
df = pd.read_csv("results/results.csv")

# Garante diretório de saída
os.makedirs("results", exist_ok=True)

# Ordena inputs e threads de forma consistente
df = df.sort_values(by=["input", "threads"])

# Lista de cores e marcadores para diferenciar threads
colors = ['skyblue', 'orange', 'green', 'red', 'purple', 'brown', 'pink', 'gray']
markers = ['o', 's', '^', 'D', 'v', '*', 'P', 'X']

# Função para plotar métricas vs entradas
def plot_metric_vs_input(metric, ylabel, filename):
    plt.figure(figsize=(10,6))
    inputs = df["input"].unique()
    threads_list = sorted(df["threads"].unique())
    
    for i, threads in enumerate(threads_list):
        sub = df[df["threads"] == threads]
        # para cada input, pega a média de serial/paralelo (ou apenas paralelo se threads>1)
        values = [sub[sub["input"]==inp][metric].values[0] if threads in sub["threads"].values else np.nan for inp in inputs]
        plt.plot(inputs, values, marker=markers[i % len(markers)],
                 color=colors[i % len(colors)], label=f"{threads} thread{'s' if threads>1 else ''}")
    
    plt.xlabel("Input")
    plt.ylabel(ylabel)
    plt.title(f"{ylabel} por Input e Threads")
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')  # legenda fora do gráfico
    plt.savefig(f"results/{filename}", bbox_inches='tight')
    plt.close()

# -----------------------------
# 1. CPI, IPC e tempo
# -----------------------------
plot_metric_vs_input("cpi", "CPI", "cpi_vs_input.pdf")
plot_metric_vs_input("ipc", "IPC", "ipc_vs_input.pdf")
plot_metric_vs_input("time_sec", "Execution Time (s)", "time_vs_input.pdf")

# -----------------------------
# 2. Branch e cache miss rate
# -----------------------------
plot_metric_vs_input("branch_miss_rate", "Branch Miss Rate", "branch_miss_vs_input.pdf")
plot_metric_vs_input("cache_miss_rate", "Cache Miss Rate", "cache_miss_vs_input.pdf")

# -----------------------------
# 3. Cache L1/L3 e TLB misses
# -----------------------------
plot_metric_vs_input("L1_dcache_load_misses", "L1 DCache Load Misses", "L1_dcache_misses_vs_input.pdf")
plot_metric_vs_input("LLC_load_misses", "LLC Load Misses", "LLC_misses_vs_input.pdf")
plot_metric_vs_input("dTLB_load_misses", "Data TLB Load Misses", "dTLB_misses_vs_input.pdf")
plot_metric_vs_input("iTLB_load_misses", "Instruction TLB Load Misses", "iTLB_misses_vs_input.pdf")

# -----------------------------
# 4. Speedup por threads
# -----------------------------
plt.figure(figsize=(10,6))
inputs = df["input"].unique()
threads_list = sorted(df["threads"].unique())

for i, threads in enumerate(threads_list):
    if threads == 1:  # serial não gera speedup
        continue
    speedups = []
    for inp in inputs:
        serial_time = df[(df["input"]==inp) & (df["mode"]=="serial") & (df["threads"]==1)]["time_sec"].values[0]
        parallel_time = df[(df["input"]==inp) & (df["mode"]=="parallel") & (df["threads"]==threads)]["time_sec"].values[0]
        speedups.append(serial_time / parallel_time)
    plt.plot(inputs, speedups, marker=markers[i % len(markers)],
             color=colors[i % len(colors)], label=f"{threads} threads")

plt.xlabel("Input")
plt.ylabel("Speedup (T_serial / T_parallel)")
plt.title("Parallel Speedup por Input e Threads")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend(bbox_to_anchor=(1.05,1), loc='upper left')
plt.tight_layout()
plt.savefig("results/speedup_vs_input.pdf", bbox_inches='tight')
plt.close()

print("[INFO] All improved plots generated successfully in results/")
