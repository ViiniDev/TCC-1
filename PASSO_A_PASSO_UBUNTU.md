# Passo a Passo para Rodar no Ubuntu

Este guia comeca a partir do momento em que o projeto ja esta no GitHub e voce vai executar os experimentos no Ubuntu.

## 1. Baixar ou Atualizar o Projeto

Se ainda nao clonou o repositorio no Ubuntu:

```bash
git clone https://github.com/ViiniDev/TCC-1.git
cd TCC-1/Codigos_Linux
```

Se o projeto ja esta clonado:

```bash
cd TCC-1
git pull
cd Codigos_Linux
```

## 2. Instalar Dependencias

Atualize os pacotes:

```bash
sudo apt update
```

Instale compilador, make, perf, Python e pip:

```bash
sudo apt install build-essential linux-tools-common linux-tools-generic python3 python3-pip
```

Instale as bibliotecas Python usadas na analise e nos graficos:

```bash
pip install pandas matplotlib
```

Se o Ubuntu bloquear instalacao global via `pip`, use:

```bash
python3 -m pip install pandas matplotlib --break-system-packages
```

Ou crie um ambiente virtual:

```bash
python3 -m venv venv
source venv/bin/activate
pip install pandas matplotlib
```

## 3. Compilar o Projeto

Entre na pasta dos codigos:

```bash
cd TCC-1/Codigos_Linux
```

Limpe executaveis antigos:

```bash
make clean
```

Compile tudo:

```bash
make
```

Ao final, devem existir estes executaveis:

```text
conv_linear
conv_malloc
dgemm
conv_linear_approx
conv_malloc_approx
dgemm_approx
```

## 4. Fazer Testes Rapidos

Antes de rodar o experimento completo, execute alguns testes pequenos.

Convolucao exata:

```bash
./conv_linear 512 0 3 12346
```

Convolucao aproximada com `float`:

```bash
./conv_linear_approx 512 0 3 float 12346 compare
```

Convolucao aproximada com `skip_kernel`:

```bash
./conv_malloc_approx 512 0 3 skip_kernel 12346 compare
```

DGEMM exato:

```bash
./dgemm 512 32 12346
```

DGEMM aproximado com `skip_k`:

```bash
./dgemm_approx 512 32 skip_k 12346 compare
```

Se todos imprimirem `Tempo`, `Checksum` e, nos aproximados, metricas de erro, o projeto esta pronto para o experimento completo.

## 5. Testar o Perf Manualmente

Opcionalmente, teste o `perf` em uma execucao simples:

```bash
perf stat -e cycles,instructions,cache-references,cache-misses ./conv_linear 512 0 3 12346
```

Teste tambem em uma versao aproximada:

```bash
perf stat -e cycles,instructions,cache-references,cache-misses ./conv_linear_approx 512 0 3 float 12346 measure
```

Se houver erro de permissao, rode com `sudo`:

```bash
sudo perf stat -e cycles,instructions,cache-references,cache-misses ./conv_linear 512 0 3 12346
```

## 6. Rodar o Experimento Completo

Garanta permissao de execucao no script:

```bash
chmod +x run_all.sh
```

Execute:

```bash
sudo ./run_all.sh
```

O script roda automaticamente:

- versoes exatas;
- versoes aproximadas;
- 10 repeticoes;
- diferentes tamanhos de matriz;
- diferentes distribuicoes;
- diferentes kernels;
- diferentes blocos do DGEMM;
- coleta com `perf`;
- calculo de erro das aproximacoes.

Ao final, ele gera:

```text
resultados.csv
```

## 7. Gerar Resumo Estatistico

Depois que `resultados.csv` for criado, rode:

```bash
python3 analysis.py
```

Isso gera:

```text
resumo_estatistico.csv
```

Esse arquivo contem medias, desvios, IPC, cache miss rate, erros e speedup.

## 8. Gerar Graficos

Rode:

```bash
python3 plot_metrics.py
```

Os graficos serao salvos na pasta:

```text
results/
```

Graficos gerados:

```text
01_tempo_por_N.png
02_speedup_aproximado.png
03_erro_relativo_medio.png
04_erro_vs_speedup.png
05_ipc_por_N.png
06_cache_miss_rate_por_N.png
07_tempo_vs_erro.png
```

Esses arquivos podem ser usados no TCC.

## 9. Conferir Arquivos Finais

Depois de tudo, confira:

```bash
ls -lh resultados.csv resumo_estatistico.csv
ls -lh results/
```

Voce deve ter:

```text
resultados.csv
resumo_estatistico.csv
results/*.png
```

## 10. Observacoes Importantes para o TCC

Para resultados mais confiaveis:

- prefira rodar em Ubuntu instalado diretamente na maquina fisica;
- evite rodar em modo economia de energia;
- feche programas pesados durante os testes;
- mantenha o notebook ligado na tomada;
- evite usar a maquina enquanto o experimento roda;
- use sempre o mesmo ambiente para todos os testes.

Se rodar em VM, os resultados ainda podem ser usados, mas descreva no TCC que o ambiente era virtualizado e que a comparacao e relativa dentro do mesmo ambiente experimental.

## Sequencia Completa Resumida

```bash
git clone https://github.com/ViiniDev/TCC-1.git
cd TCC-1/Codigos_Linux

sudo apt update
sudo apt install build-essential linux-tools-common linux-tools-generic python3 python3-pip
pip install pandas matplotlib

make clean
make

./conv_linear 512 0 3 12346
./conv_linear_approx 512 0 3 float 12346 compare
./dgemm 512 32 12346

chmod +x run_all.sh
sudo ./run_all.sh

python3 analysis.py
python3 plot_metrics.py
```
