import pandas as pd


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

group_cols = ["program_base", "program", "mode", "approx_type", "N", "dist", "K"]

summary = (
    df.groupby(group_cols, dropna=False)
    .agg(
        tempo_mean=("tempo", "mean"),
        tempo_std=("tempo", "std"),
        ipc_mean=("IPC", "mean"),
        cache_miss_rate_mean=("cache_miss_rate", "mean"),
        cache_misses_mean=("cache_misses", "mean"),
        checksum_mean=("checksum", "mean"),
        error_abs_mean=("error_abs_mean", "mean"),
        error_rel_mean=("error_rel_mean", "mean"),
        rmse_mean=("rmse", "mean"),
        error_max_mean=("error_max", "mean"),
    )
    .reset_index()
)

exact = summary[summary["mode"] == "exact"][
    ["program_base", "N", "dist", "K", "tempo_mean"]
].rename(columns={"tempo_mean": "tempo_exact_mean"})

summary = summary.merge(exact, on=["program_base", "N", "dist", "K"], how="left")
summary["speedup_vs_exact"] = summary["tempo_exact_mean"] / summary["tempo_mean"]
summary.loc[summary["mode"] == "exact", "speedup_vs_exact"] = 1.0

print(summary)
summary.to_csv("resumo_estatistico.csv", index=False)
