#ifndef ALGORITMO_MEMETICO_HPP
#define ALGORITMO_MEMETICO_HPP

#include <vector>
#include <random>

using namespace std;

// Alias em que Rota representa um caminho, armazenando os IDs das cidades sequencialmente
typedef vector<int> Rota;

enum TipoBuscaLocal {
    SWAP_LOCAL, 
    TWO_OPT,    
    SHIFT    
};

class AlgoritmoMemetico {
private:
    vector<vector<double>> matriz; // Matriz de custo/distância entre as cidades
    mt19937 rng;                
    int num_cidades;            
    
    int tamanho_pop;             
    int num_geracoes;            
    double taxa_mutacao;         
    double prob_busca_local;    

    /*!
    * @brief Calcula a função de aptidão (distância total percorrida)
    * @param rota Guarda a sequência de cidades visitadas
    * @return custo Devolve o custo total do caminho
    */
    double calcular_custo(const Rota& rota);

    /*!
    * @brief Gera a população inical do problema
    * Cria uma rota inicial embaralhando aleatoriamente os índices das cidades
    * @return rota Devolve a rota criada e embaralhada
    */
    Rota criar_individuo_aleatorio();

    /*!
    * @brief Implementa a seleção por torneio
    * @param populacao Conjunto de todas as rotas existentes na geração atual do algoritmo
    * @param tam_torneio Quantidades de indivíduos sorteados para cada torneio
    * @return melhor_rota Devolve a rota vencedora, para ser usada como pai no cruzamento
    */
    Rota selecao_torneio(const vector<Rota>& populacao, int tam_torneio);

    /*!
    * @brief Implementa cruzamento OX para adquirir a nova geração de indivíduos
    * Uso do cruzamento OX para garantir que não haja cidades repetidas ou faltando
    * @param pa1 Rota genitora selecionada no torneio
    * @param pai2 Rota genitora selecionada no torneio
    * @return filho Retorna a nova rota gerada no cruzamento
    */
    Rota cruzamento_ox(const Rota& pai1, const Rota& pai2);

    /*!
    * @brief Realiza mutação por swap para introduzir alterações na rota
    * @param rota É o indivíduo (caminho) obtido após o cruzamento
    */
    void mutacao_swap(Rota& rota);

    /*!
    * @brief Implementação da busca local swap
    * @param rota É a rota do indivíduo que sofre busca local
    */
    void busca_local_swap(Rota& rota);

    /*!
    * @brief Implementa a busca local 2-opt
    * @param rota É a rota do indivíduo que sofre busca local
    */
    void busca_local_2opt(Rota& rota);

    /*!
    * @brief Implementação da busca local shift
    * @param rota É a rota do indivíduo que sofre busca local
    */
    void busca_local_shift(Rota& rota);

    /*!
    * @brief Gerenciador das buscas locais
    * Primeiro o 2-opt para remover grandes cruzamentos
    * Em seguida, shift para ajustar a ordem dos circuitos
    * Por fim, o swap para trocar cidades vizinhas de posição se necessário
    * @param rota Rota que precisa de otimização
    */
    void aplicar_busca_local(Rota& rota);

public:
    // Construtor da classe AlgoritmoMemetico
    AlgoritmoMemetico(const vector<vector<double>>& matriz_problema, int pop, int ger, double mut, double p_busca);

    /*!
    * @brief Executa as funções implementadas para buscar a melhor rota possível
    * @return melhor_custo_global
    */
    double executar();
};

#endif 