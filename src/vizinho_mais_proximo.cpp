/*
 * vizinho_mais_proximo.cpp
 * ---------------------------------------------------------------------
 * IMPLEMENTAÇÃO 1: Heurística Construtiva do Vizinho Mais Próximo + Busca Local 2-opt
 *
 * Descrição geral do algoritmo:
 *
 * 1) Heurística do Vizinho Mais Próximo (construtiva, gulosa):
 *    Partindo de uma cidade inicial, a cada passo o algoritmo escolhe
 *    visitar a cidade não visitada mais próxima da cidade atual. O
 *    processo se repete até que todas as cidades tenham sido visitadas,
 *    e então retorna-se à cidade inicial, fechando o ciclo.
 *
 * 2) Busca Local 2-opt (melhoria, determinística):
 *    Repetidamente, o algoritmo tenta remover 2 arestas da rota atual e
 *    reconectar os 2 caminhos resultantes de outra forma (invertendo um
 *    dos trechos). Se essa troca reduz o custo total da rota, a troca é
 *    aceita. O processo se repete até que nenhuma troca 2-opt melhore a
 *    rota (ótimo local em relação à vizinhança 2-opt).
 *
 * ENTRADA:
 *   Os arquivos de dados são lidos
 *   diretamente de "dados/grafo_km.txt", "dados/grafo_min.txt" e
 *   "dados/cidades.txt".
 *
 * SAÍDA:
 *   - Impressão no terminal do progresso e de uma tabela-resumo final.
 *   - Arquivo texto "resultados/resultado_vizinho_mais_proximo.txt"
 *     com o resumo detalhado dos 12 problemas.
 * ---------------------------------------------------------------------
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <fstream>
#include "../include/Grafo.hpp"

// =====================================================================
// HEURÍSTICA CONSTRUTIVA: VIZINHO MAIS PRÓXIMO
// =====================================================================
/*
 * construirRotaVizinhoMaisProximo
 * --------------------------------
 * Constrói uma rota (ciclo hamiltoniano) usando a heurística gulosa do
 * Vizinho Mais Próximo.
 *
 * Entrada:
 *   grafo          - instância do grafo já carregado (matriz de custos).
 *   verticeInicial - índice do vértice de onde a rota começa (0..n-1).
 *
 * Saída:
 *   Um vetor de inteiros de tamanho n, contendo a ordem de visita das
 *   cidades (uma permutação de 0..n-1), começando em verticeInicial.
 *   O retorno ao vértice inicial (fechamento do ciclo) é implícito.
 */
std::vector<int> construirRotaVizinhoMaisProximo(const Grafo &grafo, int verticeInicial)
{
    int n = grafo.n;
    std::vector<bool> visitado(n, false);
    std::vector<int> rota;
    rota.reserve(n);

    int atual = verticeInicial;
    visitado[atual] = true;
    rota.push_back(atual);

    // A cada passo, escolhe o vértice não visitado mais próximo do atual
    for (int passo = 1; passo < n; passo++)
    {
        int melhorVizinho = -1;
        double menorCusto = std::numeric_limits<double>::max();

        for (int candidato = 0; candidato < n; candidato++)
        {
            if (!visitado[candidato] && grafo.custo[atual][candidato] < menorCusto)
            {
                menorCusto = grafo.custo[atual][candidato];
                melhorVizinho = candidato;
            }
        }

        visitado[melhorVizinho] = true;
        rota.push_back(melhorVizinho);
        atual = melhorVizinho;
    }

    return rota;
}

// =====================================================================
// BUSCA LOCAL: 2-OPT (Best Improvement)
// =====================================================================
/*
 * buscaLocal2opt
 * --------------
 * Aplica busca local com vizinhança 2-opt sobre uma rota já construída,
 * tentando reduzir seu custo total até atingir um ótimo local.
 *
 * Movimento 2-opt: dados dois índices i < j da rota, remove-se as arestas
 * (rota[i-1], rota[i]) e (rota[j], rota[j+1]), e reconecta-se invertendo
 * o trecho rota[i..j].
 *
 * Estratégia: best improvement a cada passada (avalia todas as trocas
 * possíveis e aplica a melhor), repetindo até nenhuma troca melhorar a
 * rota (ótimo local).
 *
 * Entrada:
 *   grafo - instância do grafo (matriz de custos).
 *   rota  - rota inicial.
 *
 * Saída:
 *   Nova rota (vetor de inteiros, permutação de 0..n-1) com custo igual
 *   ou menor que a rota de entrada.
 */
std::vector<int> buscaLocal2opt(const Grafo &grafo, std::vector<int> rota)
{
    int n = (int)rota.size();
    bool melhorou = true;

    while (melhorou)
    {
        melhorou = false;
        double melhorGanho = 0.0;
        int melhorI = -1, melhorJ = -1;

        for (int i = 0; i < n - 1; i++)
        {
            int a = rota[i];
            int b = rota[i + 1];
            for (int j = i + 2; j < n; j++)
            {
                if (i == 0 && j == n - 1)
                    continue;

                int c = rota[j];
                int d = rota[(j + 1) % n];

                double custoAntigo = grafo.custo[a][b] + grafo.custo[c][d];
                double custoNovo = grafo.custo[a][c] + grafo.custo[b][d];
                double ganho = custoAntigo - custoNovo;

                if (ganho > melhorGanho + 1e-9)
                {
                    melhorGanho = ganho;
                    melhorI = i;
                    melhorJ = j;
                }
            }
        }

        if (melhorI != -1)
        {
            std::reverse(rota.begin() + melhorI + 1, rota.begin() + melhorJ + 1);
            melhorou = true;
        }
    }

    return rota;
}

/*
 * imprimirRota
 * ------------
 * Formata uma rota como texto legível "cidade1 -> cidade2 -> ... ->
 */
static std::string imprimirRota(const Grafo &grafo, const std::vector<int> &rota)
{
    std::string resultado;
    for (int v : rota)
    {
        resultado += grafo.nomeDoVertice(v) + " -> ";
    }
    resultado += grafo.nomeDoVertice(rota.front());
    return resultado;
}

/*
 * gerar_sequencia_vmp
 * --------------------
 * Gera uma sequência contínua de índices [inicio, fim] (inclusive),
 * usada para selecionar os primeiros N vértices de cada cenário.
 */
std::vector<int> gerar_sequencia_vmp(int inicio, int fim)
{
    std::vector<int> seq;
    seq.reserve(fim - inicio + 1);
    for (int i = inicio; i <= fim; ++i)
        seq.push_back(i);
    return seq;
}

/*
 * extrair_subgrafo
 * -----------------
 * Recorta um subgrafo contendo apenas as cidades cujos índices estão em
 * "indices", preservando nomes e custos entre elas.
 *
 * Entrada : grafo_completo (48 cidades) e a lista de índices desejados.
 * Saída   : novo objeto Grafo apenas com essas cidades (reindexadas de
 *           0 a indices.size()-1).
 */
Grafo extrair_subgrafo(const Grafo &grafo_completo, const std::vector<int> &indices)
{
    Grafo subgrafo;
    subgrafo.n = (int)indices.size();
    subgrafo.custo.assign(subgrafo.n, std::vector<double>(subgrafo.n, 0.0));
    subgrafo.nomes.assign(subgrafo.n, "");
    for (int i = 0; i < subgrafo.n; ++i)
    {
        subgrafo.nomes[i] = grafo_completo.nomeDoVertice(indices[i]);
        for (int j = 0; j < subgrafo.n; ++j)
        {
            subgrafo.custo[i][j] = grafo_completo.custo[indices[i]][indices[j]];
        }
    }
    return subgrafo;
}

// Estrutura com a definição de cada um dos 6 cenários de tamanho
struct Cenario
{
    std::string nome;
    std::vector<int> indices;
};

// Estrutura para armazenar o resultado de cada um dos 12 problemas
struct Resultado
{
    int numero;
    std::string cenario_nome;
    std::string medida;
    double custo_construtivo;
    double custo_2opt;
    double tempo_construcao;
};

