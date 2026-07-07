/*
 * insercao_mais_barata.cpp
 * ---------------------------------------------------------------------
 * Heurística Construtiva da Inserção Mais Barata + Busca Local Shift
 * ---------------------------------------------------------------------
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <algorithm>
#include <fstream>
#include "../include/Grafo.hpp"

// =====================================================================
// HEURÍSTICA CONSTRUTIVA: INSERÇÃO MAIS BARATA
// =====================================================================
std::vector<int> construirRotaInsercaoMaisBarata(const Grafo &grafo)
{
    int n = grafo.n;

    // --- Inicialização: subtour com as 2 cidades mais distantes ---
    // Encontra o par (a, b) com maior custo(a, b)
    int cidadeA = 0, cidadeB = 1;
    double maiorCusto = grafo.custo[0][1];
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (grafo.custo[i][j] > maiorCusto)
            {
                maiorCusto = grafo.custo[i][j];
                cidadeA = i;
                cidadeB = j;
            }
        }
    }

    // Tour inicial: ciclo A -> B -> A  (representado como lista [A, B])
    // A aresta de fechamento B -> A é implícita
    std::vector<int> tour;
    tour.reserve(n);
    tour.push_back(cidadeA);
    tour.push_back(cidadeB);

    std::vector<bool> inserido(n, false);
    inserido[cidadeA] = true;
    inserido[cidadeB] = true;

    // --- Iterações: insere uma cidade por vez até completar o tour ---
    for (int iter = 2; iter < n; iter++)
    {
        int tamTour = (int)tour.size();

        // Para cada cidade não inserida, calcula seu menor delta de inserção
        // e qual posição (aresta) do tour a receberia com menor custo.
        double melhorDeltaGlobal = std::numeric_limits<double>::max();
        int melhorCidade   = -1;
        int melhorPosicao  = -1; // inserir APÓS tour[melhorPosicao]

        for (int k = 0; k < n; k++)
        {
            if (inserido[k])
                continue;

            // Calcula o menor delta de inserção de k em qualquer aresta do tour
            double menorDeltaK = std::numeric_limits<double>::max();
            int melhorPosK = -1;

            for (int pos = 0; pos < tamTour; pos++)
            {
                int i = tour[pos];
                int j = tour[(pos + 1) % tamTour];
                double delta = grafo.custo[i][k] + grafo.custo[k][j] - grafo.custo[i][j];
                if (delta < menorDeltaK)
                {
                    menorDeltaK = delta;
                    melhorPosK  = pos;
                }
            }

            // A cidade com menor delta entre todas as não inseridas é escolhida
            if (menorDeltaK < melhorDeltaGlobal)
            {
                melhorDeltaGlobal = menorDeltaK;
                melhorCidade      = k;
                melhorPosicao     = melhorPosK;
            }
        }

        // Insere melhorCidade logo após tour[melhorPosicao]
        tour.insert(tour.begin() + melhorPosicao + 1, melhorCidade);
        inserido[melhorCidade] = true;
    }

    return tour;
}

// =====================================================================
// BUSCA LOCAL: SHIFT (Or-opt, segmento de tamanho 1) - Best Improvement
// =====================================================================
std::vector<int> buscaLocalShift(const Grafo &grafo, std::vector<int> rota)
{
    int n = (int)rota.size();
    bool melhorou = true;

    while (melhorou)
    {
        melhorou = false;
        double melhorGanho = 0.0;
        int melhorP = -1; // posição de onde o vértice será removido
        int melhorQ = -1; // posição após a qual o vértice será inserido

        // Testa remover o vértice na posição p
        for (int p = 0; p < n; p++)
        {
            int prev_p = (p - 1 + n) % n;
            int next_p = (p + 1) % n;

            int vp   = rota[p];
            int vPrev = rota[prev_p];
            int vNext = rota[next_p];

            // Custo de remover rota[p] (fechar o buraco)
            double custoRemocao = grafo.custo[vPrev][vp] + grafo.custo[vp][vNext]
                                - grafo.custo[vPrev][vNext];

            // Testa inserir rota[p] entre rota[q] e rota[(q+1)%n]
            for (int q = 0; q < n; q++)
            {
                // Não inserir na mesma posição original
                // (q == prev_p insere de volta no mesmo lugar; q == p idem)
                if (q == prev_p || q == p)
                    continue;

                int vq     = rota[q];
                int vqNext = rota[(q + 1) % n];

                // Custo de inserir vp entre vq e vqNext
                double custoInsercao = grafo.custo[vq][vp] + grafo.custo[vp][vqNext]
                                     - grafo.custo[vq][vqNext];

                double ganho = custoRemocao - custoInsercao;

                if (ganho > melhorGanho + 1e-9)
                {
                    melhorGanho = ganho;
                    melhorP     = p;
                    melhorQ     = q;
                }
            }
        }

        if (melhorP != -1)
        {
            // Aplica o movimento: remove rota[melhorP] e reinsere após rota[melhorQ]
            int vertice = rota[melhorP];
            rota.erase(rota.begin() + melhorP);

            // Após a remoção, se melhorQ > melhorP, o índice decrementou
            int posInsercao = melhorQ;
            if (melhorQ > melhorP)
                posInsercao--;

            // Insere após posInsercao (ou seja, na posição posInsercao + 1)
            rota.insert(rota.begin() + posInsercao + 1, vertice);

            melhorou = true;
        }
    }

    return rota;
}

// =====================================================================
// UTILITÁRIOS
// =====================================================================
std::string imprimirRota(const Grafo &grafo, const std::vector<int> &rota)
{
    std::string resultado;
    for (int v : rota)
    {
        resultado += grafo.nomeDoVertice(v) + " -> ";
    }
    resultado += grafo.nomeDoVertice(rota.front());
    return resultado;
}

// =====================================================================
// MAIN
// =====================================================================
#ifndef GENERATING_MAIN_IMB
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <caminho_grafo.txt> [caminho_cidades.txt]\n";
        return 1;
    }

    std::string caminhoGrafo  = argv[1];
    std::string caminhoCidades = (argc >= 3) ? argv[2] : "";

    // --- Carrega o grafo ---
    Grafo grafo;
    try
    {
        grafo.carregarDeArquivo(caminhoGrafo);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro ao carregar grafo: " << e.what() << "\n";
        return 1;
    }

    if (!caminhoCidades.empty())
    {
        grafo.carregarNomesDeArquivo(caminhoCidades);
    }

    std::cout << "Grafo carregado com " << grafo.n << " cidades.\n\n";

    // ----------------------------------------------------------------
    // ETAPA 1: Heurística da Inserção Mais Barata
    // ----------------------------------------------------------------
    auto tInicio = std::chrono::high_resolution_clock::now();

    std::vector<int> rotaConstrutiva = construirRotaInsercaoMaisBarata(grafo);
    double custoConstrutivo = grafo.custoRota(rotaConstrutiva);

    auto tFimConstrucao = std::chrono::high_resolution_clock::now();
    double tempoConstrucao = std::chrono::duration<double>(tFimConstrucao - tInicio).count();

    std::cout << "== Heuristica da Insercao Mais Barata ==\n";
    std::cout << "Custo (antes da busca local): " << custoConstrutivo << "\n";
    std::cout << "Tempo de construcao: " << tempoConstrucao << " s\n\n";

    // ----------------------------------------------------------------
    // ETAPA 2: Busca Local Shift
    // ----------------------------------------------------------------
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<int> rotaFinal = buscaLocalShift(grafo, rotaConstrutiva);
    auto t1 = std::chrono::high_resolution_clock::now();
    double tempoShift = std::chrono::duration<double>(t1 - t0).count();
    double custoFinal = grafo.custoRota(rotaFinal);

    // Rotaciona a rota final para começar no vértice 0 (ANGICOS), se presente
    auto it = std::find(rotaFinal.begin(), rotaFinal.end(), 0);
    if (it != rotaFinal.end()) {
        std::rotate(rotaFinal.begin(), it, rotaFinal.end());
    }

    std::cout << "== Apos Busca Local Shift ==\n";
    std::cout << "Custo (apos shift): " << custoFinal << "\n";
    std::cout << "Tempo de busca (shift): " << tempoShift << " s\n\n";

    double tempoTotal = tempoConstrucao + tempoShift;

    std::cout << "== Resumo ==\n";
    std::cout << "Custo final: " << custoFinal << "\n";
    std::cout << "Tempo total: " << tempoTotal << " s\n\n";
    std::cout << "Rota final:\n" << imprimirRota(grafo, rotaFinal) << "\n";

    // ----------------------------------------------------------------
    // Geração do arquivo de resultados
    // ----------------------------------------------------------------
    std::ofstream saida("resultados/resultado_insercao_mais_barata.txt");
    if (saida.is_open())
    {
        saida << "RESULTADOS - Heuristica da Insercao Mais Barata + Busca Local Shift\n";
        saida << "==================================================================\n";
        saida << "Instancia (arquivo de grafo): " << caminhoGrafo << "\n";
        saida << "Vertice inicial: " << grafo.nomeDoVertice(0) << "\n\n";

        saida << "Custo da rota construtiva (antes da busca local): " << custoConstrutivo << "\n";
        saida << "Tempo de construcao: " << tempoConstrucao << " s\n\n";

        saida << "Custo apos busca local Shift: " << custoFinal << "\n";
        saida << "Tempo de busca local (shift): " << tempoShift << " s\n\n";

        saida << "Custo da rota final: " << custoFinal << "\n";
        saida << "Tempo total de execucao: " << tempoTotal << " s\n\n";

        saida << "Rota final:\n" << imprimirRota(grafo, rotaFinal) << "\n";
        saida.close();
        std::cout << "\nResumo salvo em resultados/resultado_insercao_mais_barata.txt\n";
    }
    else
    {
        std::cerr << "Aviso: nao foi possivel salvar o arquivo de resultados.\n";
    }

    return 0;
}
#endif


