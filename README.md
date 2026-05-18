# Convolucao, DGEMM e Computacao Aproximada

Este projeto mede o desempenho de operacoes com matrizes e compara execucoes exatas com execucoes aproximadas.

Ele cobre:

- Convolucao 2D com memoria contigua (`conv_linear`)
- Convolucao 2D com alocacao por linhas (`conv_malloc`)
- Multiplicacao densa de matrizes blocada (`dgemm`)
- Versoes aproximadas com reducao de precisao (`float`)
- Versoes aproximadas com reducao de operacoes (`skip_kernel` e `skip_k`)
- Coleta de metricas com `perf`
- Calculo de erro numerico
- Geracao de graficos para analise experimental

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

## Compilacao

```bash
cd Codigos_Linux
make
```

Executaveis gerados:

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

## Execucao Manual

### Convolucao exata

```bash
./conv_linear <N> <dist> <K> [seed]
./conv_malloc <N> <dist> <K> [seed]
```

Parametros:

- `N`: tamanho da matriz
- `dist`: `0` uniforme, `1` normal, `2` exponencial
- `K`: tamanho do kernel, positivo, impar e menor ou igual a `N`
- `seed`: semente opcional para reproduzir a mesma entrada

Exemplo:

```bash
./conv_linear 512 0 3 12346
./conv_malloc 512 0 3 12346
```

### Convolucao aproximada

```bash
./conv_linear_approx <N> <dist> <K> <approx_type> <seed>
./conv_malloc_approx <N> <dist> <K> <approx_type> <seed>
```

Tipos de aproximacao:

- `float`: executa a convolucao usando precisao simples internamente
- `skip_kernel`: usa apenas parte do kernel e renormaliza os pesos usados

Exemplo:

```bash
./conv_linear_approx 512 0 3 float 12346
./conv_malloc_approx 512 0 3 skip_kernel 12346
```

Opcionalmente, os aproximados aceitam um ultimo argumento:

- `measure`: executa apenas a aproximacao, ideal para medir com `perf`
- `compare`: executa a aproximacao e calcula erro contra a referencia exata

O `run_all.sh` usa os dois modos automaticamente para evitar que o calculo da referencia exata contamine as metricas do `perf`.

### DGEMM exato

```bash
./dgemm <N> <BS> [seed]
```

Parametros:

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

Tipos de aproximacao:

- `float`: multiplica usando precisao simples internamente
- `skip_k`: reduz operacoes pulando parte do somatorio em `k`

Exemplo:

```bash
./dgemm_approx 512 32 float 12346
./dgemm_approx 512 32 skip_k 12346
```

Assim como nas convolucoes aproximadas, `dgemm_approx` tambem aceita `measure` ou `compare` como ultimo argumento opcional.

## Execucao Automatizada

```bash
chmod +x run_all.sh
sudo ./run_all.sh
```

O script executa 10 repeticoes para cada combinacao de:

- Programa
- Tamanho `N`
- Distribuicao de entrada
- Kernel ou bloco
- Tipo de aproximacao
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

Observacao: no `dgemm`, a coluna `K` armazena o tamanho do bloco (`BS`) e `dist` recebe `-`.

## Analise Estatistica

```bash
python3 analysis.py
```

Gera:

```text
resumo_estatistico.csv
```

O resumo inclui:

- Tempo medio
- Desvio padrao
- IPC medio
- Taxa media de cache miss
- Erro absoluto medio
- Erro relativo medio
- RMSE
- Erro maximo
- Speedup em relacao a versao exata

## Graficos

```bash
python3 plot_metrics.py
```

Os graficos sao salvos em:

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

Esses graficos foram pensados para apoiar a discussao do TCC:

- Tempo de execucao por tamanho de matriz
- Ganho de desempenho das aproximacoes
- Erro relativo medio
- Relacao entre erro e speedup
- IPC
- Cache miss rate
- Custo computacional vs erro aproximado

## Ideia Experimental

As versoes exatas servem como referencia.

As versoes aproximadas medem o ganho de desempenho aceitando perda numerica controlada. O projeto registra essa troca usando metricas de erro e speedup:

```text
speedup = tempo_exato / tempo_aproximado
```

Quanto maior o speedup, maior o ganho de desempenho. Quanto menor o erro, mais proximo o resultado aproximado fica da referencia exata.
