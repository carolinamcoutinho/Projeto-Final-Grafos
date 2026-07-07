#ifndef UTILS_AM_HPP
#define UTILS_AM_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// Carrega matriz de adjacência vinda do arquivo de texto
inline vector<vector<double>> carregar_matriz(const string& caminho) {
    ifstream arquivo(caminho);
    vector<vector<double>> matriz;
    
    if (!arquivo.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo " << caminho << endl;
        return matriz;
    }

    int n;
    arquivo >> n;

    matriz.resize(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            arquivo >> matriz[i][j];
        }
    }

    arquivo.close();
    return matriz;
}

// Cria uma sequência de inteiros para indexação de cenários
inline vector<int> gerar_sequencia(int inicio, int fim) {
    vector<int> seq;
    for (int i = inicio; i <= fim; ++i) seq.push_back(i);
    return seq;
}

// Recorta um subconjunto de cidades para rodar o problema
inline vector<vector<double>> extrair_submatriz(const vector<vector<double>>& matriz_completa, const vector<int>& indices) {
    int n = indices.size();
    vector<vector<double>> submatriz(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            submatriz[i][j] = matriz_completa[indices[i]][indices[j]];
        }
    }
    return submatriz;
}

#endif 