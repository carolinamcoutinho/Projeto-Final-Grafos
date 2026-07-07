#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <limits>
#include "../include/algoritmo_memetico.hpp"
#include "../include/utils_am.hpp"

using namespace std;

int main() {
    // Carrega os dados usando o módulo utilitário
    vector<vector<double>> matriz_km = carregar_matriz("dados/grafo_km.txt");
    vector<vector<double>> matriz_min = carregar_matriz("dados/grafo_min.txt");
    
    if (matriz_km.empty() || matriz_min.empty()) {
        cerr << "Erro nos arquivos de entrada em 'dados'." << endl;
        return 1;
    }

    struct Cenario {
        string nome;
        vector<int> indices;
    };
    
    vector<Cenario> cenarios = {
        {"48 cidades (1 a 48)", gerar_sequencia(0, 47)},
        {"36 cidades (1 a 36)", gerar_sequencia(0, 35)},
        {"24 cidades (1 a 24)", gerar_sequencia(0, 23)},
        {"12 cidades (1 a 12)", gerar_sequencia(0, 11)},
        {"7 cidades (1,7,8,9,10,11,12)", {0, 6, 7, 8, 9, 10, 11}},
        {"6 cidades (1 a 6)", gerar_sequencia(0, 5)}
    };
    
    int iteracoes = 20;

    ofstream arquivo_saida("resultados/resultados_memetico.txt");
    if (!arquivo_saida.is_open()) return 1;

    arquivo_saida << "=== RESULTADOS DO ALGORITMO MEMÉTICO ===\n";
    arquivo_saida << "Busca Local Híbrida Sequencial: 2-opt + Shift + Swap\n\n";
    
    cout << "Iniciando Algoritmo Memetico utilizando a busca local hibrida...\n\n";
    
    int num_problema = 1; 
    
    for (const auto& cenario : cenarios) {
        for (int m = 0; m < 2; ++m) {
            const auto& matriz_base = (m == 0) ? matriz_km : matriz_min;
            string tipo_medida = (m == 0) ? "Distancia (Km)" : "Tempo (Min)";
            
            vector<vector<double>> submatriz = extrair_submatriz(matriz_base, cenario.indices);
            
            arquivo_saida << "-> Problema " << num_problema << ": " << tipo_medida << " - " << cenario.nome << "\n";
            cout << "Processando Problema " << num_problema << "/12 (" << tipo_medida << " - " << cenario.nome << ")..." << endl;
            
            double menor_custo = numeric_limits<double>::max();
            double soma_custos = 0.0;
            double soma_tempos = 0.0;
            
            for (int i = 0; i < iteracoes; ++i) {
                auto inicio = chrono::high_resolution_clock::now();
                
                int tamanho_pop = (cenario.indices.size() < 12) ? 15 : 40;
                int geracoes = (cenario.indices.size() < 12) ? 30 : 100;
                double taxa_mutacao = 0.05;
                double prob_busca_local = 0.25; 
                
                AlgoritmoMemetico am(submatriz, tamanho_pop, geracoes, taxa_mutacao, prob_busca_local);
                double custo_encontrado = am.executar();
                
                auto fim = chrono::high_resolution_clock::now();
                chrono::duration<double> tempo_decorrido = fim - inicio;
                
                if (custo_encontrado < menor_custo) menor_custo = custo_encontrado;
                soma_custos += custo_encontrado;
                soma_tempos += tempo_decorrido.count();
            }
            
            arquivo_saida << fixed << setprecision(2);
            arquivo_saida << "Menor custo encontrado : " << menor_custo << "\n";
            arquivo_saida << "Custo medio (20 exec)  : " << soma_custos / iteracoes << "\n";
            arquivo_saida << "Tempo medio de execucao: " << soma_tempos / iteracoes << " segundos\n";
            arquivo_saida << "--------------------------------------------------------\n\n";
            
            num_problema++; 
        }
    }
    
    arquivo_saida.close();
    cout << "\nExecucao concluida! Verifique o arquivo gerado: resultados_memetico.txt" << endl;
    return 0;
}