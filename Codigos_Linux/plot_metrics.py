import pandas as pd
import matplotlib.pyplot as plt
import os

# carregar CSV
df = pd.read_csv("resultados.csv")

# garantir pasta de saída
os.makedirs("results", exist_ok=True)

# converter colunas
numeric_cols = ["N","dist","K","tempo","cycles","instructions","cache_references","cache_misses"]
for col in numeric_cols:
    df[col] = pd.to_numeric(df[col], errors="coerce")

# métricas derivadas
df["IPC"] = df["instructions"] / df["cycles"]
df["cache_miss_rate"] = df["cache_misses"] / df["cache_references"]

############################################
# 1 Tempo vs tamanho da matriz
############################################

plt.figure()

for prog in df["program"].unique():
    sub = df[df["program"] == prog]
    means = sub.groupby("N")["tempo"].mean()
    plt.plot(means.index, means.values, marker='o', label=prog)

plt.xlabel("Matrix Size (N)")
plt.ylabel("Execution Time (s)")
plt.title("Tempo vs Tamanho da Matriz")
plt.legend()
plt.grid()

plt.savefig("results/time_vs_N.png")
plt.close()

############################################
# 2 IPC vs N
############################################

plt.figure()

for prog in df["program"].unique():
    sub = df[df["program"] == prog]
    means = sub.groupby("N")["IPC"].mean()
    plt.plot(means.index, means.values, marker='o', label=prog)

plt.xlabel("Matrix Size (N)")
plt.ylabel("IPC")
plt.title("IPC vs Tamanho da Matriz")
plt.legend()
plt.grid()

plt.savefig("results/ipc_vs_N.png")
plt.close()

############################################
# 3 Cache miss rate
############################################

plt.figure()

for prog in df["program"].unique():
    sub = df[df["program"] == prog]
    means = sub.groupby("N")["cache_miss_rate"].mean()
    plt.plot(means.index, means.values, marker='o', label=prog)

plt.xlabel("Matrix Size (N)")
plt.ylabel("Cache Miss Rate")
plt.title("Cache Miss Rate vs N")
plt.legend()
plt.grid()

plt.savefig("results/cache_miss_rate_vs_N.png")
plt.close()

############################################
# 4 Tempo vs Kernel (apenas convolução)
############################################

conv = df[df["program"] == "conv_malloc"]

if not conv.empty:

    plt.figure()

    for K in conv["K"].unique():
        sub = conv[conv["K"] == K]
        means = sub.groupby("N")["tempo"].mean()
        plt.plot(means.index, means.values, marker='o', label=f"K={K}")

    plt.xlabel("Matrix Size (N)")
    plt.ylabel("Execution Time (s)")
    plt.title("Convolução: Tempo vs Kernel Size")
    plt.legend()
    plt.grid()

    plt.savefig("results/convolution_kernel_vs_time.png")
    plt.close()

############################################
# 5 Tempo vs Distribuição
############################################

if not conv.empty:

    plt.figure()

    dist_names = {0:"Uniform",1:"Normal",2:"Exponential"}

    for d in conv["dist"].unique():
        sub = conv[conv["dist"] == d]
        means = sub.groupby("N")["tempo"].mean()
        plt.plot(means.index, means.values, marker='o', label=dist_names.get(d,str(d)))

    plt.xlabel("Matrix Size (N)")
    plt.ylabel("Execution Time (s)")
    plt.title("Tempo vs Distribuição de Dados")
    plt.legend()
    plt.grid()

    plt.savefig("results/distribution_vs_time.png")
    plt.close()

print("Gráficos gerados na pasta results/")