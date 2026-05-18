import os

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


RESULTS_DIR = "results"
os.makedirs(RESULTS_DIR, exist_ok=True)

plt.rcParams.update(
    {
        "figure.figsize": (11, 6.5),
        "figure.dpi": 120,
        "savefig.dpi": 300,
        "axes.grid": True,
        "grid.alpha": 0.25,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "font.size": 11,
        "axes.titlesize": 15,
        "axes.labelsize": 12,
        "legend.fontsize": 9,
    }
)


def load_data():
    df = pd.read_csv("resultados.csv")

    numeric_cols = [
        "N",
        "K",
        "seed",
        "tempo",
        "cycles",
        "instructions",
        "cache_references",
        "cache_misses",
        "checksum",
        "error_abs_mean",
        "error_rel_mean",
        "rmse",
        "error_max",
    ]

    for col in numeric_cols:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    df["IPC"] = df["instructions"] / df["cycles"]
    df["cache_miss_rate"] = df["cache_misses"] / df["cache_references"]
    df["program_base"] = df["program"].str.replace("_approx", "", regex=False)
    df["series"] = df.apply(make_series_name, axis=1)

    exact = (
        df[df["mode"] == "exact"]
        .groupby(["program_base", "N", "dist", "K"], dropna=False)["tempo"]
        .mean()
        .reset_index()
        .rename(columns={"tempo": "tempo_exact"})
    )

    df = df.merge(exact, on=["program_base", "N", "dist", "K"], how="left")
    df["speedup_vs_exact"] = df["tempo_exact"] / df["tempo"]
    df.loc[df["mode"] == "exact", "speedup_vs_exact"] = 1.0

    return df


def make_series_name(row):
    if row["mode"] == "exact":
        return f"{row['program_base']} exact"

    return f"{row['program_base']} {row['approx_type']}"


def save_plot(name):
    plt.tight_layout()
    plt.savefig(os.path.join(RESULTS_DIR, name), bbox_inches="tight")
    plt.close()


def plot_time_by_n(df):
    grouped = (
        df.groupby(["series", "N"], dropna=False)["tempo"]
        .mean()
        .reset_index()
        .sort_values(["series", "N"])
    )

    plt.figure()

    for series, sub in grouped.groupby("series"):
        plt.plot(sub["N"], sub["tempo"], marker="o", linewidth=2, label=series)

    plt.xlabel("Tamanho da matriz (N)")
    plt.ylabel("Tempo medio (s)")
    plt.title("Tempo de execucao por tamanho da matriz")
    plt.legend(ncol=2)
    save_plot("01_tempo_por_N.png")


def plot_speedup(df):
    approx = df[df["mode"] == "approx"]

    grouped = (
        approx.groupby(["series", "N"], dropna=False)["speedup_vs_exact"]
        .mean()
        .reset_index()
        .sort_values(["series", "N"])
    )

    plt.figure()

    for series, sub in grouped.groupby("series"):
        plt.plot(sub["N"], sub["speedup_vs_exact"], marker="o", linewidth=2, label=series)

    plt.axhline(1.0, color="black", linestyle="--", linewidth=1)
    plt.xlabel("Tamanho da matriz (N)")
    plt.ylabel("Speedup medio vs exato")
    plt.title("Ganho de desempenho das versoes aproximadas")
    plt.legend(ncol=2)
    save_plot("02_speedup_aproximado.png")


def plot_relative_error(df):
    approx = df[df["mode"] == "approx"]

    grouped = (
        approx.groupby(["series", "N"], dropna=False)["error_rel_mean"]
        .mean()
        .reset_index()
        .sort_values(["series", "N"])
    )

    plt.figure()

    for series, sub in grouped.groupby("series"):
        plt.plot(sub["N"], sub["error_rel_mean"], marker="o", linewidth=2, label=series)

    plt.xlabel("Tamanho da matriz (N)")
    plt.ylabel("Erro relativo medio")
    plt.title("Erro medio das versoes aproximadas")
    plt.legend(ncol=2)
    save_plot("03_erro_relativo_medio.png")


def plot_error_vs_speedup(df):
    approx = df[df["mode"] == "approx"]

    grouped = (
        approx.groupby(["series"], dropna=False)
        .agg(speedup=("speedup_vs_exact", "mean"), error=("error_rel_mean", "mean"))
        .reset_index()
    )

    plt.figure()
    plt.scatter(grouped["error"], grouped["speedup"], s=90)

    for _, row in grouped.iterrows():
        plt.annotate(row["series"], (row["error"], row["speedup"]), xytext=(6, 5), textcoords="offset points")

    plt.axhline(1.0, color="black", linestyle="--", linewidth=1)
    plt.xlabel("Erro relativo medio")
    plt.ylabel("Speedup medio vs exato")
    plt.title("Troca entre erro e desempenho")
    save_plot("04_erro_vs_speedup.png")


def plot_ipc(df):
    grouped = (
        df.groupby(["series", "N"], dropna=False)["IPC"]
        .mean()
        .reset_index()
        .sort_values(["series", "N"])
    )

    plt.figure()

    for series, sub in grouped.groupby("series"):
        plt.plot(sub["N"], sub["IPC"], marker="o", linewidth=2, label=series)

    plt.xlabel("Tamanho da matriz (N)")
    plt.ylabel("IPC medio")
    plt.title("Instrucoes por ciclo por tamanho da matriz")
    plt.legend(ncol=2)
    save_plot("05_ipc_por_N.png")


def plot_cache_miss_rate(df):
    grouped = (
        df.groupby(["series", "N"], dropna=False)["cache_miss_rate"]
        .mean()
        .reset_index()
        .sort_values(["series", "N"])
    )

    plt.figure()

    for series, sub in grouped.groupby("series"):
        plt.plot(sub["N"], sub["cache_miss_rate"], marker="o", linewidth=2, label=series)

    plt.xlabel("Tamanho da matriz (N)")
    plt.ylabel("Taxa media de cache miss")
    plt.title("Taxa de cache miss por tamanho da matriz")
    plt.legend(ncol=2)
    save_plot("06_cache_miss_rate_por_N.png")


def plot_time_vs_error_by_program(df):
    approx = df[df["mode"] == "approx"]

    grouped = (
        approx.groupby(["program_base", "approx_type"], dropna=False)
        .agg(tempo=("tempo", "mean"), error=("error_rel_mean", "mean"))
        .reset_index()
    )

    plt.figure()
    plt.scatter(grouped["tempo"], grouped["error"], s=90)

    for _, row in grouped.iterrows():
        label = f"{row['program_base']} {row['approx_type']}"
        plt.annotate(label, (row["tempo"], row["error"]), xytext=(6, 5), textcoords="offset points")

    plt.xlabel("Tempo medio (s)")
    plt.ylabel("Erro relativo medio")
    plt.title("Custo computacional vs erro aproximado")
    save_plot("07_tempo_vs_erro.png")


df = load_data()

plot_time_by_n(df)
plot_speedup(df)
plot_relative_error(df)
plot_error_vs_speedup(df)
plot_ipc(df)
plot_cache_miss_rate(df)
plot_time_vs_error_by_program(df)

print(f"Graficos gerados em {RESULTS_DIR}/")
