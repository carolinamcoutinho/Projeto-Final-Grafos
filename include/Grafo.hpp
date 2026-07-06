/*
 * Grafo.hpp
 * ---------------------------------------------------------------------
 * Estrutura de dados do Problema do Caixeiro-Viajante (PCV).
 *
 *
 * Representação escolhida: matriz de adjacência (grafo completo, não
 * direcionado, com pesos reais >= 0). Essa representação foi escolhida
 * porque os dados de origem (planilha de distâncias/tempos entre cidades
 * do RN) formam um grafo COMPLETO (existe custo entre todo par de
 * cidades), o que torna a matriz de adjacência mais simples e eficiente
 * (acesso a custo(i,j) em O(1)) do que uma lista de adjacência.
 *
 * FORMATO DO ARQUIVO DE ENTRADA (gerado por scripts/converter_planilha.py):
 *   linha 1........: N (quantidade de vértices/cidades)
 *   linhas 2..N+1...: N valores double por linha (matriz de custos),
 *                      separados por espaço. matriz[i][j] = custo(i,j).
 *                      A diagonal (i == j) vale 0.0.
 *
 * O arquivo cidades.txt (formato "indice;nome") serve
 * para dar nomes legíveis aos vértices nos relatórios de saída.
 * ---------------------------------------------------------------------
 */

#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>

class Grafo
{
public:
    int n;                                  // número de vértices (cidades)
    std::vector<std::vector<double>> custo; // matriz de custos n x n
    std::vector<std::string> nomes;         // nomes dos vértices

    Grafo() : n(0) {}

    /*
     * carregarDeArquivo
     * -----------------
     * Entrada : caminho de um arquivo no formato descrito no cabeçalho
     *           deste arquivo (primeira linha = N, seguido de N linhas
     *           com N valores double cada, representando a matriz de
     *           custos).
     * Saída   : preenche os atributos n e custo desta instância de Grafo.
     *           Não há retorno; lança std::runtime_error caso o arquivo
     *           não possa ser aberto ou esteja mal formatado.
     */
    void carregarDeArquivo(const std::string &caminho)
    {
        std::ifstream arquivo(caminho);
        if (!arquivo.is_open())
        {
            throw std::runtime_error("Não foi possível abrir o arquivo: " + caminho);
        }

        arquivo >> n;
        custo.assign(n, std::vector<double>(n, 0.0));

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (!(arquivo >> custo[i][j]))
                {
                    throw std::runtime_error("Arquivo de grafo mal formatado em (" +
                                             std::to_string(i) + "," + std::to_string(j) + ")");
                }
            }
        }
        arquivo.close();
    }

    /*
     * carregarNomesDeArquivo
     * -----------------------
     * Entrada : caminho de um arquivo no formato "indice;nome" (uma
     *           cidade por linha, indice começando em 0).
     * Saída   : preenche o vetor "nomes" desta instância de Grafo.
     */
    void carregarNomesDeArquivo(const std::string &caminho)
    {
        std::ifstream arquivo(caminho);
        if (!arquivo.is_open())
        {
            return;
        }
        nomes.assign(n, "");
        std::string linha;
        while (std::getline(arquivo, linha))
        {
            if (linha.empty())
                continue;
            size_t pos = linha.find(';');
            if (pos == std::string::npos)
                continue;
            int idx = std::stoi(linha.substr(0, pos));
            std::string nome = linha.substr(pos + 1);
            if (idx >= 0 && idx < n)
                nomes[idx] = nome;
        }
        arquivo.close();
    }

    /*
     * nomeDoVertice
     * -------------
     * Entrada : índice de um vértice (0..n-1)
     * Saída   : nome do vértice, se conhecido; caso contrário, uma string
     *           com o próprio índice.
     */
    std::string nomeDoVertice(int i) const
    {
        if (i >= 0 && i < (int)nomes.size() && !nomes[i].empty())
        {
            return nomes[i];
        }
        return "V" + std::to_string(i);
    }

    /*
     * custoRota
     * ---------
     * Calcula o custo total de um ciclo hamiltoniano (rota que passa por
     * todos os vértices exatamente uma vez e retorna ao início).
     *
     * Entrada : rota, um vetor de tamanho n contendo uma permutação dos
     *           índices [0, n-1], representando a ordem de visita das
     *           cidades.
     * Saída   : soma dos custos de cada aresta da rota (incluindo a
     *           aresta de retorno do último vértice para o primeiro).
     */
    double custoRota(const std::vector<int> &rota) const
    {
        double total = 0.0;
        int tamanho = (int)rota.size();
        for (int i = 0; i < tamanho; i++)
        {
            int atual = rota[i];
            int proximo = rota[(i + 1) % tamanho]; // último vértice volta ao primeiro
            total += custo[atual][proximo];
        }
        return total;
    }
};

#endif
