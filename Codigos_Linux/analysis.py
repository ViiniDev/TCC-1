import pandas as pd

# Carregar dados
df = pd.read_csv("resultados.csv")

# Converter colunas numéricas
df["tempo"] = pd.to_numeric(df["tempo"], errors="coerce")
df["cycles"] = pd.to_numeric(df["cycles"], errors="coerce")
df["instructions"] = pd.to_numeric(df["instructions"], errors="coerce")
df["cache_misses"] = pd.to_numeric(df["cache_misses"], errors="coerce")
df["cache_miss_rate"] = df["cache_misses"] / df["cache_references"]

# Calcular IPC
df["IPC"] = df["instructions"] / df["cycles"]

# Agrupar por experimento
grouped = df.groupby(["program", "N", "dist", "K"])

summary = grouped.agg({
    "tempo": ["mean", "std"],
    "IPC": "mean",
    "cache_misses": "mean"
}).reset_index()

print(summary)

# Salvar resultado consolidado
summary.to_csv("resumo_estatistico.csv", index=False)