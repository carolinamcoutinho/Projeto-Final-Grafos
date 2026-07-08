#include "../include/algoritmo_memetico.hpp"
#include <numeric>
#include <algorithm>
#include <limits>
#include <chrono>

using namespace std;

// Construtor da classe AlgoritmoMemetico
AlgoritmoMemetico::AlgoritmoMemetico(const vector<vector<double>>& matriz_problema, int pop, int ger, double mut, double p_busca)
    : matriz(matriz_problema), num_cidades(matriz_problema.size()), tamanho_pop(pop), 
      num_geracoes(ger), taxa_mutacao(mut), prob_busca_local(p_busca) {
    
    uint64_t seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(seed);
}

double AlgoritmoMemetico::calcular_custo(const Rota& rota) {
    double custo = 0.0;

    // Calcula o custo da cidade atual até a cidade seguinte seguinte
    for (int i = 0; i < num_cidades - 1; ++i) {
        custo += matriz[rota[i]][rota[i+1]];
    }

    // Fecha o ciclo, calculando o custo da última cidade até a primeira
    custo += matriz[rota[num_cidades - 1]][rota[0]];
    return custo;
}

Rota AlgoritmoMemetico::criar_individuo_aleatorio() {
    Rota rota(num_cidades);
    iota(rota.begin(), rota.end(), 0);
    shuffle(rota.begin(), rota.end(), rng); 
    return rota;
}

Rota AlgoritmoMemetico::selecao_torneio(const vector<Rota>& populacao, int tam_torneio) {
    uniform_int_distribution<int> dist(0, populacao.size() - 1);
    
    // Sorteia o primeiro competidor do torneio
    Rota melhor_rota = populacao[dist(rng)];
    double melhor_custo = calcular_custo(melhor_rota);
    
    // Testa os competidores restantes
    for (int i = 1; i < tam_torneio; ++i) {
        Rota competidor = populacao[dist(rng)];
        double custo_competidor = calcular_custo(competidor);
        
        if (custo_competidor < melhor_custo) {
            melhor_custo = custo_competidor;
            melhor_rota = competidor;
        }
    }
    
    return melhor_rota;
}

Rota AlgoritmoMemetico::cruzamento_ox(const Rota& pai1, const Rota& pai2) {
    // Inicialização e sorteio dos cortes
    Rota filho(num_cidades, -1); 
    uniform_int_distribution<int> dist(0, num_cidades - 1);
    
    int corte1 = dist(rng);
    int corte2 = dist(rng);
    if (corte1 > corte2) swap(corte1, corte2); 
    
    // Coloca o trecho de cidades entre os cortes no filho e anota as cidades herdadas para evitar duplicatas
    vector<bool> cidade_ja_no_filho(num_cidades, false);
    
    for (int i = corte1; i <= corte2; ++i) {
        filho[i] = pai1[i];
        cidade_ja_no_filho[pai1[i]] = true;
    }
    
    int ponteiro_pai2 = 0;

    // Preenche as lacunas do filho com cidades que não foram herdadas do pai2
    for (int i = 0; i < num_cidades; ++i) {
        if (filho[i] == -1) {
            while (cidade_ja_no_filho[pai2[ponteiro_pai2]]) {
                ponteiro_pai2++;
            }

            filho[i] = pai2[ponteiro_pai2];
            cidade_ja_no_filho[pai2[ponteiro_pai2]] = true;
        }
    }

    return filho;
}

void AlgoritmoMemetico::mutacao_swap(Rota& rota) {
    // Sorteador de probabilidade
    uniform_real_distribution<double> probabilidade(0.0, 1.0);
    
    // Validação para garantir que a mutação só ocorra raramente
    if (probabilidade(rng) < taxa_mutacao) {
        uniform_int_distribution<int> dist(0, num_cidades - 1);
        swap(rota[dist(rng)], rota[dist(rng)]); 
    }
}

void AlgoritmoMemetico::busca_local_swap(Rota& rota) {
    // Inicialização do laço de otimização
    bool melhorou = true;
    double melhor_custo = calcular_custo(rota);
    
    // Testa a melhora do custo para todos os pares possíveis de cidades
    while (melhorou) {
        melhorou = false;
        
        for (int i = 0; i < num_cidades - 1; ++i) {
            for (int j = i + 1; j < num_cidades; ++j) {
                swap(rota[i], rota[j]);
                double novo_custo = calcular_custo(rota);
        
                if (novo_custo < melhor_custo) {
                    melhor_custo = novo_custo;
                    melhorou = true;
                    goto recomecar_swap; 
                } else {
                    swap(rota[i], rota[j]); 
                }
            }
        }
        recomecar_swap:; 
    }
}

void AlgoritmoMemetico::busca_local_2opt(Rota& rota) {
    // Inicialização do laço de otimização
    bool melhorou = true;
    double melhor_custo = calcular_custo(rota);

    while (melhorou) {
        melhorou = false;
        for (int i = 1; i < num_cidades - 1; ++i) {
            for (int j = i + 1; j < num_cidades; ++j) {
                // Inverte a ordem das cidades e calcula o custo total para avaliar o resultado
                reverse(rota.begin() + i, rota.begin() + j + 1); 
                double novo_custo = calcular_custo(rota);
                
                if (novo_custo < melhor_custo) {
                    melhor_custo = novo_custo;
                    melhorou = true;
                    goto recomecar_2opt; 
                } else {
                    reverse(rota.begin() + i, rota.begin() + j + 1);
                }
            }
        }
        recomecar_2opt:;
    }
}

void AlgoritmoMemetico::busca_local_shift(Rota& rota) {
    // Inicialização do laço de otimização
    bool melhorou = true;
    double melhor_custo = calcular_custo(rota);

    while (melhorou) {
        melhorou = false;

        for (int i = 0; i < num_cidades; ++i) {
            for (int j = 0; j < num_cidades; ++j) {
                if (i == j) continue;

                // Deslocamento
                if (i < j) {
                    rotate(rota.begin() + i, rota.begin() + i + 1, rota.begin() + j + 1);
                } else {
                    rotate(rota.begin() + j, rota.begin() + i, rota.begin() + i + 1);
                }

                double novo_custo = calcular_custo(rota);

                // Avaliação de custo
                if (novo_custo < melhor_custo - 1e-6) {
                    melhor_custo = novo_custo;
                    melhorou = true;
                    goto recomecar_shift; 
                }

                // Volta a rota para a posição inicial, caso necessário
                if (i < j) {
                    rotate(rota.begin() + i, rota.begin() + j, rota.begin() + j + 1);
                } else {
                    rotate(rota.begin() + j, rota.begin() + j + 1, rota.begin() + i + 1);
                }
            }
        }
        recomecar_shift:;
    }
}

void AlgoritmoMemetico::aplicar_busca_local(Rota& rota) {
    busca_local_2opt(rota);
    busca_local_shift(rota);
    busca_local_swap(rota);
}

double AlgoritmoMemetico::executar() {
    // Inicialização da população
    vector<Rota> populacao;
    
    for (int i = 0; i < tamanho_pop; ++i) {
        Rota ind = criar_individuo_aleatorio();
        populacao.push_back(ind);
    }
    
    double melhor_custo_global = numeric_limits<double>::max();
    uniform_real_distribution<double> dist_prob(0.0, 1.0);
    
    // Pega a melhor rota da geração e aplica busca local para melhora-lo
    for (int geracao = 0; geracao < num_geracoes; ++geracao) {
        vector<Rota> nova_populacao;
        Rota melhor_da_geracao = populacao[0];
        double custo_melhor_atual = calcular_custo(melhor_da_geracao);
        
        for (const auto& ind : populacao) {
            double custo = calcular_custo(ind);
            if (custo < custo_melhor_atual) {
                custo_melhor_atual = custo;
                melhor_da_geracao = ind;
            }
        }
        
        aplicar_busca_local(melhor_da_geracao);
        custo_melhor_atual = calcular_custo(melhor_da_geracao); 
        
        if (custo_melhor_atual < melhor_custo_global) {
            melhor_custo_global = custo_melhor_atual;
        }
        
        nova_populacao.push_back(melhor_da_geracao);
        
        while (nova_populacao.size() < tamanho_pop) {
            Rota pai1 = selecao_torneio(populacao, 3); // Torneio k=3
            Rota pai2 = selecao_torneio(populacao, 3);
            
            Rota filho = cruzamento_ox(pai1, pai2);   
            mutacao_swap(filho);               
            
            // Sorteia se o filho receberá refinamento de busca local antes de ser colocado na nova população
            if (dist_prob(rng) < prob_busca_local) { 
                aplicar_busca_local(filho);
            }
            nova_populacao.push_back(filho);
        }
        populacao = nova_populacao; 
    }
    return melhor_custo_global;
}