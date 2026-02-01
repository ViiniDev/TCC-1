# Convolução de Matrizes com Distribuições Aleatórias

Este projeto implementa **convolução de matrizes** em C, com geração de valores aleatórios em diferentes distribuições:

- **Uniforme**
- **Exponencial**
- **Normal (Box-Muller)**

O objetivo é avaliar **eficiência, tempo e comportamento das distribuições** para matrizes de tamanhos grandes (1000, 10.000, 30.000 e 50.000), utilizando o `perf` no Linux ou o medidor de tempo de alta precisão no Windows.

---

## 📂 Estrutura do Projeto

---

## 🔧 Como Compilar e Rodar

### 🐧 Linux (Ubuntu/Debian)

1. Instale compilador e ferramentas:
   ```bash
   sudo apt update
   sudo apt install build-essential linux-tools-common linux-tools-generic -y
   ```

2. **Compilar**
   ```bash
   gcc -O2 -o conv main.c -lm
   ```

3. **Executar**
   ```bash
   ./conv <tamanho_matriz> <tipo_distribuicao> <tamanho_kernel>
   ```

   Onde:
   - `tamanho_matriz` → dimensão da matriz quadrada (ex: 1000, 10000, 30000, 50000)
   - `tipo_distribuicao` →  
     - 0 → Uniforme  
     - 1 → Exponencial  
     - 2 → Normal  
   - `tamanho_kernel` → dimensão do kernel quadrado (ex: 3, 5, 7)

   **Exemplo:**
   ```bash
   ./conv 1000 0 3
   perf stat ./conv 1000 0 3
   ```

---

### 🪟 Windows (MinGW ou Visual Studio)

1. Instale um compilador C:  
   - MinGW-w64  
   - ou o compilador do Visual Studio  

2. Compile (com MinGW):
   ```bash
   gcc -O2 -o conv.exe main.c -lm
   ```

3. Execute:
   ```bash
   .\conv.exe <tamanho_matriz> <tipo_distribuicao> <tamanho_kernel>
   ```

   Exemplo:
   ```bash
   .\conv.exe 1000 2 5
   ```

⚠️ **Atenção**: No Windows, o programa mede apenas o tempo total de execução com `QueryPerformanceCounter`.  
Ele não mostra métricas de CPU/caches como o `perf` no Linux.

---

## 📊 Distribuições Geradas

- **Uniforme** → valores igualmente distribuídos em [0,1].  
- **Exponencial** → maioria dos valores perto de 0, decaindo exponencialmente.  
- **Normal (Box-Muller)** → valores em forma de sino (Gauss).  

---

## 📌 Exemplos de Saída

### Linux com perf
```bash
$ perf stat ./conv 1000 0 3

 Performance counter stats for './conv 1000 0 3':

    1.234567 task-clock                # 1.23 s
        123456 cycles                    
        987654 instructions              
            1234 cache-misses              

    1.234567891 seconds time elapsed
```

### Windows
```powershell
> .\conv.exe 1000 2 5
    Tempo total: 1234.56 ms
```

