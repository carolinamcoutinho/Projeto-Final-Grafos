#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <numeric>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;

// Define que "Rota" é um vetor de inteiros para facilitar a leitura
typedef vector<int> Rota;

// Função para ler os arquivos de dados gerados
static vector<vector<double>> carregar_matriz(const string& caminho) {
    ifstream arquivo(caminho);
    vector<vector<double>> matriz;
    
    if (!arquivo.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo " << caminho << endl;
        return matriz;
    }

    int n;
    arquivo >> n; // A primeira linha do arquivo com o número de cidades

    matriz.resize(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            arquivo >> matriz[i][j];
        }
    }

    arquivo.close();
    return matriz;
}

// Calcula o custo total da rota (distância ou tempo) garantindo o retorno à origem
double calcular_custo(const Rota& rota, const vector<vector<double>>& matriz) {
    double custo = 0.0;
    int n = rota.size();
    
    for (int i = 0; i < n - 1; ++i) {
        custo += matriz[rota[i]][rota[i+1]];
    }
    // O caixeiro viajante deve retornar à origem
    custo += matriz[rota[n - 1]][rota[0]];
    
    return custo;
}

// Cria uma rota inicial totalmente aleatória
Rota criar_individuo_aleatorio(int num_cidades, mt19937& rng) {
    Rota rota(num_cidades);
    iota(rota.begin(), rota.end(), 0); // Preenche de 0 até num_cidades-1
    shuffle(rota.begin(), rota.end(), rng); // Embaralha a ordem
    return rota;
}

// Seleção por Torneio: Escolhe 'k' indivíduos aleatórios e pega o melhor
Rota selecao_torneio(const vector<Rota>& populacao, const vector<vector<double>>& matriz, int k, mt19937& rng) {
    uniform_int_distribution<int> dist(0, populacao.size() - 1);
    
    Rota melhor_rota = populacao[dist(rng)];
    double melhor_custo = calcular_custo(melhor_rota, matriz);
    
    for (int i = 1; i < k; ++i) {
        Rota competidor = populacao[dist(rng)];
        double custo_competidor = calcular_custo(competidor, matriz);
        
        if (custo_competidor < melhor_custo) {
            melhor_custo = custo_competidor;
            melhor_rota = competidor;
        }
    }
    return melhor_rota;
}

// Cruzamento de Ordem (Order Crossover - OX)
// Evita que as cidades se repitam ou que falte alguma cidade
Rota cruzamento_ox(const Rota& pai1, const Rota& pai2, mt19937& rng) {
    int n = pai1.size();
    Rota filho(n, -1); // Rota preenchida com -1 temporariamente
    
    uniform_int_distribution<int> dist(0, n - 1);
    int corte1 = dist(rng);
    int corte2 = dist(rng);
    if (corte1 > corte2) swap(corte1, corte2); //corte1 menor que corte2
    
    vector<bool> cidade_ja_no_filho(n, false);
    
    // Copia o miolo do Pai 1 para o Filho
    for (int i = corte1; i <= corte2; ++i) {
        filho[i] = pai1[i];
        cidade_ja_no_filho[pai1[i]] = true;
    }
    
    // Preenche os buracos restantes com os genes do Pai 2 (na ordem que aparece)
    int ponteiro_pai2 = 0;
    for (int i = 0; i < n; ++i) {
        if (filho[i] == -1) {
            while (cidade_ja_no_filho[pai2[ponteiro_pai2]]) {
                ponteiro_pai2++; // Pula cidades que já estão no filho
            }
            filho[i] = pai2[ponteiro_pai2];
            cidade_ja_no_filho[pai2[ponteiro_pai2]] = true;
        }
    }
    return filho;
}

// Mutação por Troca (Swap)
void mutacao_swap(Rota& rota, double taxa_mutacao, mt19937& rng) {
    uniform_real_distribution<double> probabilidade(0.0, 1.0);
    
    // Se sortear um valor menor que a taxa, ocorre a mutação
    if (probabilidade(rng) < taxa_mutacao) {
        uniform_int_distribution<int> dist(0, rota.size() - 1);
        int idx1 = dist(rng);
        int idx2 = dist(rng);
        swap(rota[idx1], rota[idx2]); // Troca duas cidades de lugar
    }
}

// Executa uma bateria completa do Algoritmo Genético
double executar_ag(const vector<vector<double>>& matriz, int tamanho_pop, int num_geracoes, double taxa_mutacao) {
    random_device rd;
    mt19937 rng(rd()); // Gerador de números aleatórios rápido do C++
    
    int num_cidades = matriz.size();
    vector<Rota> populacao;
    
    // 1. Gera População Inicial
    for (int i = 0; i < tamanho_pop; ++i) {
        populacao.push_back(criar_individuo_aleatorio(num_cidades, rng));
    }
    
    double melhor_custo_global = numeric_limits<double>::max();
    
    // Evolução das Gerações
    for (int geracao = 0; geracao < num_geracoes; ++geracao) {
        vector<Rota> nova_populacao;
        
        // ELITISMO: Encontra o melhor da geração atual para nunca perdê-lo
        Rota melhor_da_geracao = populacao[0];
        double custo_melhor_atual = calcular_custo(melhor_da_geracao, matriz);
        
        for (const auto& ind : populacao) {
            double custo = calcular_custo(ind, matriz);
            if (custo < custo_melhor_atual) {
                custo_melhor_atual = custo;
                melhor_da_geracao = ind;
            }
        }
        
        // Atualiza a melhor solução global encontrada até agora
        if (custo_melhor_atual < melhor_custo_global) {
            melhor_custo_global = custo_melhor_atual;
        }
        
        // Salva o melhor indivíduo diretamente na próxima geração
        nova_populacao.push_back(melhor_da_geracao);
        
        // Cruzamento e Mutação para preencher o resto da população
        while (nova_populacao.size() < tamanho_pop) {
            Rota pai1 = selecao_torneio(populacao, matriz, 3, rng);
            Rota pai2 = selecao_torneio(populacao, matriz, 3, rng);
            
            Rota filho = cruzamento_ox(pai1, pai2, rng);
            mutacao_swap(filho, taxa_mutacao, rng);
            
            nova_populacao.push_back(filho);
        }
        
        populacao = nova_populacao; // Avança para a próxima geração
    }
    
    return melhor_custo_global;
}

// FUNÇÕES AUXILIARES PARA EXTRAIR OS 12 SUBPROBLEMAS 

// Gera sequência de índices contínuos.
static vector<int> gerar_sequencia(int inicio, int fim) {
    vector<int> seq;
    for (int i = inicio; i <= fim; ++i) {
        seq.push_back(i);
    }
    return seq;
}

// Recorta a matriz principal para formar o subgrafo apenas com as cidades escolhidas
static vector<vector<double>> extrair_submatriz(const vector<vector<double>>& matriz_completa, const vector<int>& indices) {
    int n = indices.size();
    vector<vector<double>> submatriz(n, vector<double>(n, 0.0));
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            submatriz[i][j] = matriz_completa[indices[i]][indices[j]];
        }
    }
    return submatriz;
}

