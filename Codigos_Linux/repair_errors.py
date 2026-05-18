import re
import subprocess
from pathlib import Path

import pandas as pd


INPUT = Path("resultados.csv")
OUTPUT = Path("resultados_corrigido.csv")

PATTERNS = {
    "checksum": re.compile(r"^Checksum\s+([0-9.eE+-]+)", re.MULTILINE),
    "error_abs_mean": re.compile(r"^ErrorAbsMean\s+([0-9.eE+-]+)", re.MULTILINE),
    "error_rel_mean": re.compile(r"^ErrorRelMean\s+([0-9.eE+-]+)", re.MULTILINE),
    "rmse": re.compile(r"^RMSE\s+([0-9.eE+-]+)", re.MULTILINE),
    "error_max": re.compile(r"^ErrorMax\s+([0-9.eE+-]+)", re.MULTILINE),
}


def extract(output, key):
    match = PATTERNS[key].search(output)
    if not match:
        raise RuntimeError(f"Nao foi possivel extrair {key} da saida:\n{output}")
    return float(match.group(1))


def run_compare(row):
    program = str(row["program"])
    n = str(int(row["N"]))
    k = str(int(row["K"]))
    approx_type = str(row["approx_type"])
    seed = str(int(row["seed"]))

    if program in {"conv_linear_approx", "conv_malloc_approx"}:
        dist = str(row["dist"])
        cmd = [f"./{program}", n, dist, k, approx_type, seed, "compare"]
    elif program == "dgemm_approx":
        cmd = ["./dgemm_approx", n, k, approx_type, seed, "compare"]
    else:
        return None

    result = subprocess.run(cmd, text=True, capture_output=True, check=False)

    if result.returncode != 0:
        raise RuntimeError(
            f"Comando falhou com codigo {result.returncode}: {' '.join(cmd)}\n"
            f"STDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}"
        )

    output = result.stdout + "\n" + result.stderr
    return {key: extract(output, key) for key in PATTERNS}


def main():
    if not INPUT.exists():
        raise SystemExit("Arquivo resultados.csv nao encontrado.")

    df = pd.read_csv(INPUT)
    approx_mask = df["mode"] == "approx"
    total = int(approx_mask.sum())

    print(f"Corrigindo metricas de erro em {total} linhas aproximadas...")

    for count, idx in enumerate(df[approx_mask].index, start=1):
        metrics = run_compare(df.loc[idx])

        for key, value in metrics.items():
            df.at[idx, key] = value

        if count % 25 == 0 or count == total:
            print(f"{count}/{total} linhas corrigidas")

    exact_mask = df["mode"] == "exact"
    for key in ["error_abs_mean", "error_rel_mean", "rmse", "error_max"]:
        df.loc[exact_mask, key] = 0.0

    df.to_csv(OUTPUT, index=False)
    print(f"Arquivo corrigido salvo em {OUTPUT}")
    print("Para usar os dados corrigidos nos graficos:")
    print("mv resultados.csv resultados_original.csv")
    print("mv resultados_corrigido.csv resultados.csv")
    print("python3 analysis.py")
    print("python3 plot_metrics.py")


if __name__ == "__main__":
    main()
