# Problema do Caixeiro Viajante (PCV) - Algoritmos de Otimização

O projeto visa construir implementações robustas em C++ para resolver o Problema do Caixeiro Viajante utilizando abordagens heurísticas e meta-heurísticas. O projeto realiza uma análise comparativa em 12 cenários de teste, divididos de forma equitativa em matrizes de distâncias (quilômetros) e tempos de deslocamento (minutos).

## 📁 Estrutura do Repositório

```
Problema-Caixeiro-Viajante/
│
├── dados/                      # Arquivos de dados e matrizes de adjacência
│   ├── cidades.csv
│   ├── cidades.txt
│   ├── grafo_km.txt            # Matriz de distâncias completas (Km)
│   └── grafo_min.txt           # Matriz de tempos completos (Min)
│
├── include/                    # Cabeçalhos e declarações das classes (.hpp)
│   ├── Grafo.hpp
│   ├── algoritmo_memetico.hpp  # Estrutura do Algoritmo Memético
│   └── utils_am.hpp            # Funções utilitárias do Algoritmo Memético
│
├── resultados/                 # Relatórios gerados
│   ├── resultado_insercao_mais_barata.txt
│   ├── resultado_vizinho_mais_proximo.txt
│   ├── resultados_AG.txt      
│   └── resultados_memeticos.txt
│
├── src/                        # Implementações lógicas dos códigos (.cpp)
│   ├── algoritmo_genetico.cpp  
│   ├── algoritmo_memetico.cpp  
│   ├── insercao_mais_barata.cpp
│   ├── main.cpp                
│   └── vizinho_mais_proxim.cpp
│
├── .gitignore
└── Makefile                    
```

## 🚀 Algoritmos Implementados

O sistema mapeia o espaço de soluções do PCV utilizando quatro estratégias distintas de otimização:
- Heurística do Vizinho Mais Próximo + Busca Local: Abordagem construtiva gulosa seguida por refinamento determinístico.
- Heurística da Inserção Mais Barata + Busca Local: Construção incremental focada no menor aumento de custo de ciclo.
- Algoritmo Genético (AG): Meta-heurística populacional puramente estocástica utilizando Cruzamento por Ordem (OX), Mutação por Troca (Swap) e Elitismo explícito.
- Algoritmo Memético (AM): Abordagem híbrida que combina os operadores globais do AG com uma Busca Local Híbrida Sequencial operando sob uma probabilidade de 25% aplicada a três vizinhanças: 2-opt -> Shift -> Swap.

## ▶️ Como Compilar e Executar

### Requisitos Mínimos
- Compilador GCC (g++) com suporte nativo ao padrão C++11 ou superior.
- Utilitário make instalado.

### Executando os Testes
O projeto utiliza um arquivo Makefile para automatizar o build com flags de otimização agressiva (-O3) para garantir máxima eficiência de CPU. Abra o terminal na raiz do projeto e utilize os comandos abaixo:

```
make clean
make

.\main.exe
```

## Resultados

Após a execução dos testes, os resultados estarão disponíveis na pasta resultados.
