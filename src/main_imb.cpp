#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <limits>
#include <algorithm>

// Define a macro para impedir que a main original de insercao_mais_barata.cpp seja compilada
#define GENERATING_MAIN_IMB
#include "insercao_mais_barata.cpp"

using namespace std;

// Estrutura para armazenar as informações de cenário
struct Cenario {
    string nome;
    vector<int> indices;
};

// Estrutura para armazenar resultados de cada problema
struct Resultado {
    int numero;
    string cenario_nome;
    string medida;
    double custo_construtivo;
    double custo_shift;
    double tempo_construcao;
    double tempo_shift;
    double tempo_total;
    string rota_final_str;
};

// Helper para gerar sequências de índices das cidades
vector<int> gerar_sequencia_imb(int inicio, int fim) {
    vector<int> seq;
    seq.reserve(fim - inicio + 1);
    for (int i = inicio; i <= fim; ++i) seq.push_back(i);
    return seq;
}

// Helper para criar subgrafo contendo apenas as cidades do cenário
Grafo extrair_subgrafo(const Grafo& grafo_completo, const vector<int>& indices) {
    Grafo subgrafo;
    subgrafo.n = (int)indices.size();
    subgrafo.custo.assign(subgrafo.n, vector<double>(subgrafo.n, 0.0));
    subgrafo.nomes.assign(subgrafo.n, "");
    for (int i = 0; i < subgrafo.n; ++i) {
        subgrafo.nomes[i] = grafo_completo.nomeDoVertice(indices[i]);
        for (int j = 0; j < subgrafo.n; ++j) {
            subgrafo.custo[i][j] = grafo_completo.custo[indices[i]][indices[j]];
        }
    }
    return subgrafo;
}

int main() {
    // 1. Carrega os dois grafos completos
    Grafo grafo_completo_km;
    Grafo grafo_completo_min;

    try {
        grafo_completo_km.carregarDeArquivo("dados/grafo_km.txt");
        grafo_completo_km.carregarNomesDeArquivo("dados/cidades.txt");
        grafo_completo_min.carregarDeArquivo("dados/grafo_min.txt");
        grafo_completo_min.carregarNomesDeArquivo("dados/cidades.txt");
    }
    catch (const exception& e) {
        cerr << "Erro ao carregar dados dos grafos: " << e.what() << endl;
        return 1;
    }

    // 2. Define os cenários
    vector<Cenario> cenarios = {
        {"48 cidades (1 a 48)",          gerar_sequencia_imb(0, 47)},
        {"36 cidades (1 a 36)",          gerar_sequencia_imb(0, 35)},
        {"24 cidades (1 a 24)",          gerar_sequencia_imb(0, 23)},
        {"12 cidades (1 a 12)",          gerar_sequencia_imb(0, 11)},
        {"7 cidades (1,7,8,9,10,11,12)", {0, 6, 7, 8, 9, 10, 11}},
        {"6 cidades (1 a 6)",            gerar_sequencia_imb(0, 5)}
    };

    vector<Resultado> resultados;
    resultados.reserve(12);

    int num_problema = 1;
    cout << "Executando o algoritmo de Insercao Mais Barata (Heuristica + Busca Local Shift)...\n";

    for (const auto& cenario : cenarios) {
        for (int m = 0; m < 2; ++m) {
            const auto& grafo_completo = (m == 0) ? grafo_completo_km : grafo_completo_min;
            string tipo_medida = (m == 0) ? "Km" : "min";

            Grafo subgrafo = extrair_subgrafo(grafo_completo, cenario.indices);

            cout << "\n------------------------------------------\n";
            cout << "Problema " << num_problema << "/12: " << cenario.nome
                 << " [" << tipo_medida << "]\n";
            cout << "------------------------------------------\n";

            // Etapa 1: Inserção Mais Barata
            auto t0 = chrono::high_resolution_clock::now();
            vector<int> rota_construtiva = construirRotaInsercaoMaisBarata(subgrafo);
            double custo_construtivo = subgrafo.custoRota(rota_construtiva);
            auto t1 = chrono::high_resolution_clock::now();
            double tempo_const = chrono::duration<double>(t1 - t0).count();

            cout << "Custo antes da busca local:   "
                 << fixed << setprecision(2) << custo_construtivo << " " << tipo_medida << "\n";

            // Etapa 2: Busca Local Shift
            auto t2 = chrono::high_resolution_clock::now();
            vector<int> rota_shift = buscaLocalShift(subgrafo, rota_construtiva);
            double custo_shift = subgrafo.custoRota(rota_shift);
            auto t3 = chrono::high_resolution_clock::now();
            double tempo_shift = chrono::duration<double>(t3 - t2).count();

            // Rotaciona as rotas para começar no vértice 0 (ANGICOS)
            auto it1 = find(rota_construtiva.begin(), rota_construtiva.end(), 0);
            if (it1 != rota_construtiva.end()) {
                rotate(rota_construtiva.begin(), it1, rota_construtiva.end());
            }

            auto it2 = find(rota_shift.begin(), rota_shift.end(), 0);
            if (it2 != rota_shift.end()) {
                rotate(rota_shift.begin(), it2, rota_shift.end());
            }

            double tempo_total = tempo_const + tempo_shift;
            string rota_final_str = imprimirRota(subgrafo, rota_shift);

            cout << "Custo apos busca local Shift: "
                 << fixed << setprecision(2) << custo_shift << " " << tipo_medida << "\n";
            cout << "Tempo total: " << setprecision(6) << tempo_total << " s\n";
            cout << "Rota final:\n" << rota_final_str << "\n";

            resultados.push_back({
                num_problema,
                cenario.nome,
                tipo_medida,
                custo_construtivo,
                custo_shift,
                tempo_const,
                tempo_shift,
                tempo_total,
                rota_final_str
            });

            num_problema++;
        }
    }

    // 3. Tabela resumo no terminal
    cout << "\n";
    cout << "=========================================================================================================\n";
    cout << "                             RESULTADOS FINAIS - INSERCAO MAIS BARATA                                    \n";
    cout << "=========================================================================================================\n";
    cout << left  << setw(5)  << "Prob"
         << setw(32) << "Cenario"
         << setw(8)  << "Medida"
         << right << setw(14) << "Custo Const."
         << setw(14) << "Custo Shift"
         << setw(14) << "Tempo Total(s)" << "\n";
    cout << "---------------------------------------------------------------------------------------------------------\n";

    for (const auto& r : resultados) {
        cout << left  << setw(5)  << r.numero
             << setw(32) << r.cenario_nome
             << setw(8)  << r.medida
             << right << setw(14) << fixed << setprecision(2) << r.custo_construtivo
             << setw(14) << r.custo_shift
             << setw(14) << setprecision(6) << r.tempo_total << "\n";
    }
    cout << "=========================================================================================================\n\n";

    // 4. Grava no arquivo
    ofstream saida("resultados/resultado_insercao_mais_barata.txt");
    if (saida.is_open()) {
        saida << "RESULTADOS - Heuristica da Insercao Mais Barata + Busca Local Shift\n";
        saida << "======================================================================\n\n";

        for (const auto& r : resultados) {
            saida << "======================================================================\n";
            saida << "PROBLEMA " << r.numero << " - Cenario: " << r.cenario_nome << " - Medida: " << r.medida << "\n";
            saida << "======================================================================\n";
            saida << "Instancia (arquivo de grafo): " << (r.medida == "Km" ? "dados/grafo_km.txt" : "dados/grafo_min.txt") << "\n";
            saida << "Vertice inicial: ANGICOS\n\n";

            saida << "Custo da rota construtiva (antes da busca local): " << fixed << setprecision(2) << r.custo_construtivo << "\n";
            saida << "Tempo de construcao: " << fixed << setprecision(7) << r.tempo_construcao << " s\n\n";

            saida << "Custo da rota final (apos busca local Shift): " << fixed << setprecision(2) << r.custo_shift << "\n";
            saida << "Tempo de busca local: " << fixed << setprecision(7) << r.tempo_shift << " s\n";
            saida << "Tempo total de execucao: " << fixed << setprecision(7) << r.tempo_total << " s\n\n";

            saida << "Rota final:\n" << r.rota_final_str << "\n\n";
        }
        saida.close();
        cout << "Resultados de todos os 12 problemas salvos em resultados/resultado_insercao_mais_barata.txt" << endl;
    } else {
        cerr << "Erro ao abrir o arquivo resultados/resultado_insercao_mais_barata.txt para escrita." << endl;
    }

    return 0;
}
