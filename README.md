# Convolução, DGEMM e Computação Aproximada

Este projeto mede o desempenho de operações com matrizes e compara execuções exatas com execuções aproximadas.

Ele cobre:

- Convolução 2D com memória contígua (`conv_linear`)
- Convolução 2D com alocação por linhas (`conv_malloc`)
- Multiplicação densa de matrizes blocada (`dgemm`)
- Versões aproximadas com redução de precisão (`float`)
- Versões aproximadas com redução de operações (`skip_kernel` e `skip_k`)
- Coleta de métricas com `perf`
- Cálculo de erro numérico
- Geração de gráficos para análise experimental

## Estrutura

```text
Codigos_Linux/
  Makefile
  main_linear.c
  main_malloc.c
  dgemm_blocked.c
  conv_linear_approx.c
  conv_malloc_approx.c
  dgemm_approx.c
  run_all.sh
  analysis.py
  plot_metrics.py
```

## Requisitos no Ubuntu

```bash
sudo apt update
sudo apt install build-essential linux-tools-common linux-tools-generic python3 python3-pip
pip install pandas matplotlib
```

## Compilação

```bash
cd Codigos_Linux
make
```

Executáveis gerados:

```text
conv_linear
conv_malloc
dgemm
conv_linear_approx
conv_malloc_approx
dgemm_approx
```

Para limpar:

```bash
make clean
```

## Execução Manual

### Convolução exata

```bash
./conv_linear <N> <dist> <K> [seed]
./conv_malloc <N> <dist> <K> [seed]
```

Parâmetros:

- `N`: tamanho da matriz
- `dist`: `0` uniforme, `1` normal, `2` exponencial
- `K`: tamanho do kernel, positivo, impar e menor ou igual a `N`
- `seed`: semente opcional para reproduzir a mesma entrada

Exemplo:

```bash
./conv_linear 512 0 3 12346
./conv_malloc 512 0 3 12346
```

### Convolução aproximada

```bash
./conv_linear_approx <N> <dist> <K> <approx_type> <seed>
./conv_malloc_approx <N> <dist> <K> <approx_type> <seed>
```

Tipos de aproximação:

- `float`: executa a convolução usando precisão simples internamente
- `skip_kernel`: usa apenas parte do kernel e renormaliza os pesos usados

Exemplo:

```bash
./conv_linear_approx 512 0 3 float 12346
./conv_malloc_approx 512 0 3 skip_kernel 12346
```

Opcionalmente, os aproximados aceitam um último argumento:

- `measure`: executa apenas a aproximação, ideal para medir com `perf`
- `compare`: executa a aproximação e calcula erro contra a referência exata

O `run_all.sh` usa os dois modos automaticamente para evitar que o cálculo da referência exata contamine as métricas do `perf`.

### DGEMM exato

```bash
./dgemm <N> <BS> [seed]
```

Parâmetros:

- `N`: tamanho da matriz
- `BS`: tamanho do bloco
- `seed`: semente opcional

Exemplo:

```bash
./dgemm 512 32 12346
```

### DGEMM aproximado

```bash
./dgemm_approx <N> <BS> <approx_type> <seed>
```

Tipos de aproximação:

- `float`: multiplica usando precisão simples internamente
- `skip_k`: reduz operações pulando parte do somatório em `k`

Exemplo:

```bash
./dgemm_approx 512 32 float 12346
./dgemm_approx 512 32 skip_k 12346
```

Assim como nas convoluções aproximadas, `dgemm_approx` também aceita `measure` ou `compare` como último argumento opcional.

## Execução Automatizada

```bash
chmod +x run_all.sh
sudo ./run_all.sh
```

O script executa 10 repetições para cada combinação de:

- Programa
- Tamanho `N`
- Distribuição de entrada
- Kernel ou bloco
- Tipo de aproximação
- Seed padronizada

Ele gera:

```text
resultados.csv
```

Colunas do CSV:

```text
program,mode,approx_type,N,dist,K,seed,tempo,cycles,instructions,
cache_references,cache_misses,checksum,error_abs_mean,error_rel_mean,
rmse,error_max
```

Observação: no `dgemm`, a coluna `K` armazena o tamanho do bloco (`BS`) e `dist` recebe `-`.

## Análise Estatística

```bash
python3 analysis.py
```

Gera:

```text
resumo_estatistico.csv
```

O resumo inclui:

- Tempo médio
- Desvio padrão
- IPC médio
- Taxa média de cache miss
- Erro absoluto médio
- Erro relativo médio
- RMSE
- Erro máximo
- Speedup em relação a versão exata

## Gráficos

```bash
python3 plot_metrics.py
```

Os gráficos são salvos em:

```text
results/
```

Arquivos gerados:

```text
01_tempo_por_N.png
02_speedup_aproximado.png
03_erro_relativo_medio.png
04_erro_vs_speedup.png
05_ipc_por_N.png
06_cache_miss_rate_por_N.png
07_tempo_vs_erro.png
```

Esses gráficos foram pensados para apoiar a discussão do TCC:

- Tempo de execução por tamanho de matriz
- Ganho de desempenho das aproximações
- Erro relativo médio
- Relação entre erro e speedup
- IPC
- Cache miss rate
- Custo computacional vs erro aproximado

## Ideia Experimental

As versões exatas servem como referência.

As versões aproximadas medem o ganho de desempenho aceitando perda numérica controlada. O projeto registra essa troca usando métricas de erro e speedup:

```text
speedup = tempo_exato / tempo_aproximado
```

Quanto maior o speedup, maior o ganho de desempenho. Quanto menor o erro, mais próximo o resultado aproximado fica da referência exata.
