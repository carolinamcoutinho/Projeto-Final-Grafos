/*
 * vizinho_mais_proximo.cpp
 * ---------------------------------------------------------------------
 * IMPLEMENTAÇÃO 1: Heurística Construtiva do Vizinho Mais Próximo + Busca Local
 *
 * Descrição geral do algoritmo:
 *
 * 1) Heurística do Vizinho Mais Próximo (construtiva, gulosa):
 *    Partindo de uma cidade inicial, a cada passo o algoritmo escolhe
 *    visitar a cidade NÃO VISITADA mais próxima da cidade atual. O
 *    processo se repete até que todas as cidades tenham sido visitadas,
 *    e então retorna-se à cidade inicial, fechando o ciclo.
 *
 * 2) Busca Local (melhoria, determinística):
 *    Repetidamente, o algoritmo tenta remover 2 arestas da rota atual e
 *    reconectar os 2 caminhos resultantes de outra forma (invertendo um
 *    dos trechos). Se essa troca reduz o custo total da rota, a troca é
 *    aceita. O processo se repete até que nenhuma troca 2-opt melhore a
 *    rota (ótimo local em relação à vizinhança 2-opt).
 *
 * Como a Heurística do Vizinho Mais Próximo e a Busca Local 2-opt são
 * determinísticas(dado um vértice inicial fixo, sempre produzem o mesmo
 * resultado), este programa é executado apenas uma vez.
 *
 * ENTRADA:
 *   argv[1] = caminho do arquivo de grafo (ex.: dados/grafo_km.txt)
 *   argv[2] = caminho do arquivo de nomes das cidades
 *   argv[3] = índice do vértice inicial (opcional, padrão = 0)
 *
 * SAÍDA:
 *   - Impressão no terminal com a rota encontrada, o custo antes e
 *     depois da busca local, e o tempo de execução.
 *   - Um arquivo texto "resultados/resultado_vizinho_mais_proximo.txt"
 *     com o resumo dos resultados.
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

/*
 * construirRotaVizinhoMaisProximo
 * --------------------------------
 * Constrói uma rota (ciclo hamiltoniano) usando a heurística gulosa do
 * Vizinho Mais Próximo.
 *
 * Entrada:
 *   grafo         - instância do grafo já carregado (matriz de custos).
 *   verticeInicial- índice do vértice de onde a rota começa (0..n-1).
 *
 * Saída:
 *   Um vetor de inteiros de tamanho n, contendo a ordem de visita das
 *   cidades (uma permutação de 0..n-1), começando em verticeInicial.
 *   O retorno ao vértice inicial (fechamento do ciclo) é implícito: quem
 *   usa a rota deve considerar que o último vértice se liga ao primeiro.
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

        // melhorVizinho nunca deve ficar -1 num grafo completo com n > 0
        visitado[melhorVizinho] = true;
        rota.push_back(melhorVizinho);
        atual = melhorVizinho;
    }

    return rota;
}

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
 * Estratégia de busca: best improvement a cada
 * passada, isto é, em cada iteração completa avalia-se todas as trocas
 * 2-opt possíveis e aplica-se a que mais reduz o custo. Repete-se até que
 * nenhuma troca consiga melhorar a rota (ótimo local).
 *
 * Entrada:
 *   grafo - instância do grafo (matriz de custos).
 *   rota  - rota inicial (será copiada; a função não altera o parâmetro
 *           original, retorna uma nova rota otimizada).
 *
 * Saída:
 *   Uma nova rota (vetor de inteiros, permutação de 0..n-1) com custo
 *   igual ou menor que a rota de entrada.
 */
std::vector<int> buscaLocal2opt(const Grafo &grafo, std::vector<int> rota)
{
    int n = (int)rota.size();
    bool melhorou = true;

    while (melhorou)
    {
        melhorou = false;
        double melhorGanho = 0.0; // maior redução de custo encontrada nesta passada
        int melhorI = -1, melhorJ = -1;

        // percorre todos os pares (i, j) de arestas possíveis de trocar
        for (int i = 0; i < n - 1; i++)
        {
            int a = rota[i];
            int b = rota[i + 1];
            for (int j = i + 2; j < n; j++)
            {
                // evita remover a mesma aresta duas vezes (caso do último com o primeiro)
                if (i == 0 && j == n - 1)
                    continue;

                int c = rota[j];
                int d = rota[(j + 1) % n];

                double custoAntigo = grafo.custo[a][b] + grafo.custo[c][d];
                double custoNovo = grafo.custo[a][c] + grafo.custo[b][d];
                double ganho = custoAntigo - custoNovo; // ganho > 0 significa melhoria

                if (ganho > melhorGanho + 1e-9)
                {
                    melhorGanho = ganho;
                    melhorI = i;
                    melhorJ = j;
                }
            }
        }

        // se encontrou alguma troca que melhora, aplica a melhor delas
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
 * Formata uma rota (vetor de índices) como texto legível "cidade1 ->
 * cidade2 -> ... -> cidade1", usando os nomes do grafo quando
 * disponíveis.
 *
 * Entrada : grafo (para obter nomes) e a rota a ser formatada.
 * Saída   : string com a rota formatada.
 */
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <caminho_grafo.txt> [caminho_cidades.txt] [vertice_inicial]\n";
        return 1;
    }

    std::string caminhoGrafo = argv[1];
    std::string caminhoCidades = (argc >= 3) ? argv[2] : "";
    int verticeInicial = (argc >= 4) ? std::stoi(argv[3]) : 0;

    // --- Carrega o grafo ---
    Grafo grafo;
    grafo.carregarDeArquivo(caminhoGrafo);
    if (!caminhoCidades.empty())
    {
        grafo.carregarNomesDeArquivo(caminhoCidades);
    }

    if (verticeInicial < 0 || verticeInicial >= grafo.n)
    {
        std::cerr << "Vertice inicial invalido. Deve estar entre 0 e " << grafo.n - 1 << "\n";
        return 1;
    }

    std::cout << "Grafo carregado com " << grafo.n << " cidades.\n";
    std::cout << "Vertice inicial: " << grafo.nomeDoVertice(verticeInicial) << "\n\n";

    // --- Etapa 1: Heurística do Vizinho Mais Próximo ---
    auto inicio = std::chrono::high_resolution_clock::now();

    std::vector<int> rotaConstrutiva = construirRotaVizinhoMaisProximo(grafo, verticeInicial);
    double custoConstrutivo = grafo.custoRota(rotaConstrutiva);

    auto fimConstrucao = std::chrono::high_resolution_clock::now();

    // --- Etapa 2: Busca Local 2-opt (pós-processamento, determinístico) ---
    std::vector<int> rotaFinal = buscaLocal2opt(grafo, rotaConstrutiva);
    double custoFinal = grafo.custoRota(rotaFinal);

    auto fimBuscaLocal = std::chrono::high_resolution_clock::now();

    double tempoConstrucao = std::chrono::duration<double>(fimConstrucao - inicio).count();
    double tempoBuscaLocal = std::chrono::duration<double>(fimBuscaLocal - fimConstrucao).count();
    double tempoTotal = tempoConstrucao + tempoBuscaLocal;

    // --- Impressão dos resultados no terminal ---
    std::cout << "== Heuristica do Vizinho Mais Proximo ==\n";
    std::cout << "Custo (antes da busca local): " << custoConstrutivo << "\n";
    std::cout << "Tempo de construcao: " << tempoConstrucao << " s\n\n";

    std::cout << "== Apos Busca Local 2-opt ==\n";
    std::cout << "Custo (depois da busca local): " << custoFinal << "\n";
    std::cout << "Tempo de busca local: " << tempoBuscaLocal << " s\n";
    std::cout << "Tempo total: " << tempoTotal << " s\n\n";

    std::cout << "Rota final:\n"
              << imprimirRota(grafo, rotaFinal) << "\n";

    // --- Geração do arquivo de resultados ---
    std::ofstream saida("resultados/resultado_vizinho_mais_proximo.txt");
    if (saida.is_open())
    {
        saida << "RESULTADOS - Heuristica do Vizinho Mais Proximo + Busca Local 2-opt\n";
        saida << "======================================================================\n";
        saida << "Instancia (arquivo de grafo): " << caminhoGrafo << "\n";
        saida << "Vertice inicial: " << grafo.nomeDoVertice(verticeInicial) << "\n\n";

        saida << "Custo da rota construtiva (antes da busca local): " << custoConstrutivo << "\n";
        saida << "Tempo de construcao: " << tempoConstrucao << " s\n\n";

        saida << "Custo da rota final (apos busca local 2-opt): " << custoFinal << "\n";
        saida << "Tempo de busca local: " << tempoBuscaLocal << " s\n";
        saida << "Tempo total de execucao: " << tempoTotal << " s\n\n";

        saida << "Rota final:\n"
              << imprimirRota(grafo, rotaFinal) << "\n";
        saida.close();
        std::cout << "\nResumo salvo em resultados/resultado_vizinho_mais_proximo.txt\n";
    }
    else
    {
        std::cerr << "Aviso: nao foi possivel salvar o arquivo de resultados "
                     "(verifique se a pasta 'resultados' existe).\n";
    }

    return 0;
}
