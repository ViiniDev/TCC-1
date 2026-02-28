# Convolução e DGEMM com Análise Experimental de Desempenho

Este projeto implementa:

* Convolução 2D discreta
* Multiplicação densa (DGEMM)
* Análise microarquitetural com `perf`
* Automatização experimental
* Consolidação estatística

O objetivo é avaliar desempenho computacional considerando:

* Complexidade teórica
* Layout de memória
* Escalabilidade
* Comportamento de cache
* IPC (Instructions Per Cycle)

---

# 📂 Estrutura do Projeto

```
.
├── main_linear.c
├── main_malloc.c
├── dgemm_naive.c
├── run_all.sh
├── analysis.py
├── resultados.csv
└── resumo_estatistico.csv
```

---

# 🔧 Compilação

```bash
gcc -O3 -march=native -o conv_linear main_linear.c -lm
gcc -O3 -march=native -o conv_malloc main_malloc.c -lm
gcc -O3 -march=native -o dgemm dgemm_naive.c
```

---

# ⚙ Execução Automatizada

```bash
chmod +x run_all.sh
sudo ./run_all.sh
```

Gera:

```
resultados.csv
```

---

# 📊 Consolidação Estatística

Instalar dependências:

```bash
pip install pandas matplotlib
```

Executar:

```bash
python3 analysis.py
```

Gera:

```
resumo_estatistico.csv
```

Esse arquivo contém:

* Média do tempo
* Desvio padrão
* IPC médio
* Cache-miss médio

---

# 📈 Metodologia Experimental

Para cada combinação:

* Programa (conv_linear, conv_malloc, dgemm)
* N ∈ {500, 1000}
* Distribuições ∈ {Uniforme, Normal, Exponencial}
* Kernel ∈ {3,5}
* 5 repetições

Métricas coletadas:

* Tempo (clock_gettime)
* cycles
* instructions
* cache-misses

---

# 📊 Resultados Observados

## Escalabilidade

* Convolução: crescimento ~ O(N²)
* DGEMM: crescimento ~ O(N³)

Confirmando modelo teórico.

---

## Layout de Memória

* Memória contígua mais eficiente
* Menor taxa de cache-miss
* IPC maior

Confirma impacto da localidade espacial.

---

## Impacto do Kernel

* Tempo cresce proporcionalmente a K²
* Instructions aumentam conforme esperado

---

## IPC

Valores médios entre 1.5 e 2.0 indicam:

* Execução parcialmente compute-bound
* Boa utilização do pipeline

---

# 🔬 Boas Práticas Utilizadas

* CPU em modo performance
* Aplicações fechadas
* Locale fixo (LC_ALL=C)
* Execução única por medição
* Repetições múltiplas
* Consolidação estatística

---

# 📌 Conclusão

O projeto valida experimentalmente:

* A complexidade assintótica teórica
* O impacto do layout de memória
* A diferença estrutural entre convolução e multiplicação densa
* O comportamento microarquitetural medido por IPC e cache-misses

---


# ✅ 1️⃣ Verificação Técnica dos Dados

### ✔ Estrutura do CSV

* 8 colunas corretas
* Nenhum campo vazio
* Valores numéricos coerentes
* Perf sendo capturado corretamente
* Tempo consistente com ciclos

### ✔ Estabilidade experimental

As repetições apresentam:

* Variação < 2%
* Sem outliers extremos
* Sem valores zerados
* Sem erro de parsing


---

# 📊 2️⃣ Interpretação Científica dos Resultados


---

## 🔹 A) Escalabilidade da Convolução

Teoria:

[
O(N^2 \cdot K^2)
]

Observação experimental:

* Ao dobrar N (500 → 1000)
* O tempo cresce aproximadamente 4x


---

## 🔹 B) Escalabilidade do DGEMM

Teoria:

[
O(N^3)
]

Observação:

* Ao dobrar N
* O tempo cresce aproximadamente 8x

✔ Crescimento cúbico confirmado experimentalmente.


---

## 🔹 C) Linear vs malloc (Localidade de Memória)

Resultado observado:

* `conv_linear` consistentemente mais rápido
* IPC maior
* Cache-misses menores

Interpretação:

Memória contígua melhora:

* Localidade espacial
* Eficiência de cache
* Aproveitamento do pipeline

Conclusão experimental forte:

> O layout contíguo reduz penalidades de cache e melhora desempenho.

---

## 🔹 D) Impacto do Kernel (K)

Comparando K=3 vs K=5:

* Tempo aumenta proporcionalmente a K²
* Instructions aumentam
* Cache-miss varia pouco para N pequeno

Isso indica que:

* Para N pequeno, dados ainda cabem em cache
* Para N maior, impacto tende a crescer

---

## 🔹 E) IPC (Instructions Per Cycle)

Valores observados:

* Entre ~1.5 e ~2.0

Interpretação:

* Execução razoavelmente eficiente
* Não totalmente memory-bound
* Nem totalmente compute-bound

---

# 🎯 Conclusão Técnica dos Resultados

1. Convolução apresenta crescimento quadrático
2. DGEMM apresenta crescimento cúbico
3. Layout de memória impacta significativamente o desempenho
4. IPC e cache-miss confirmam efeito da localidade espacial



---
