#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "../include/Grafo.hpp"
#include "../include/algoritmo_memetico.hpp"

extern std::vector<int> construirRotaVizinhoMaisProximo(const Grafo &grafo, int verticeInicial);
extern std::vector<int> buscaLocal2opt(const Grafo &grafo, std::vector<int> rota);
extern std::vector<int> construirRotaInsercaoMaisBarata(const Grafo &grafo);
extern std::vector<int> buscaLocalShift(const Grafo &grafo, std::vector<int> rota);
extern double executar_ag(const std::vector<std::vector<double>> &matriz, int tamanho_pop, int num_geracoes, double taxa_mutacao);

using Rota = std::vector<int>;

struct Cenario
{
    std::string nome;
    std::vector<int> indices;
};

static std::vector<Cenario> criarCenarios()
{
    return {
        {"48 cidades (1 a 48)", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47}},
        {"36 cidades (1 a 36)", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35}},
        {"24 cidades (1 a 24)", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}},
        {"12 cidades (1 a 12)", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
        {"7 cidades (1,7,8,9,10,11,12)", {0, 6, 7, 8, 9, 10, 11}},
        {"6 cidades (1 a 6)", {0, 1, 2, 3, 4, 5}}
    };
}

static std::vector<std::vector<double>> carregarMatriz(const std::string &caminho)
{
    std::ifstream arquivo(caminho);
    std::vector<std::vector<double>> matriz;
    if (!arquivo.is_open())
    {
        std::cerr << "Erro: nao foi possivel abrir o arquivo " << caminho << "\n";
        return matriz;
    }

    int n = 0;
    arquivo >> n;
    if (n <= 0)
    {
        std::cerr << "Erro: formato invalido do arquivo " << caminho << "\n";
        return matriz;
    }

    matriz.assign(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            arquivo >> matriz[i][j];
        }
    }

    return matriz;
}

static std::vector<std::vector<double>> extrairSubmatriz(const std::vector<std::vector<double>> &matriz_completa, const std::vector<int> &indices)
{
    int n = static_cast<int>(indices.size());
    std::vector<std::vector<double>> submatriz(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            submatriz[i][j] = matriz_completa[indices[i]][indices[j]];
        }
    }
    return submatriz;
}

static std::string rotaParaString(const Grafo &grafo, const Rota &rota)
{
    std::string texto;
    for (size_t i = 0; i < rota.size(); ++i)
    {
        texto += grafo.nomeDoVertice(rota[i]);
        if (i + 1 < rota.size())
            texto += " -> ";
    }
    texto += " -> ";
    texto += grafo.nomeDoVertice(rota.front());
    return texto;
}

static void mostrarMenu()
{
    std::cout << "\n=== Selecionar algoritmo ===\n";
    std::cout << "1) Vizinho Mais Proximo\n";
    std::cout << "2) Insercao Mais Barata\n";
    std::cout << "3) Algoritmo Genetico\n";
    std::cout << "4) Algoritmo Memetico\n";
    std::cout << "5) Sair\n";
    std::cout << "Escolha: ";
}

static int lerEscolha()
{
    int opcao;
    if (!(std::cin >> opcao))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    return opcao;
}

static void executarVizinhoMaisProximo(const Grafo &grafo_km, const Grafo &grafo_min)
{
    std::cout << "\n>> Executando Vizinho Mais Proximo + 2-opt...\n";
    auto cenarios = criarCenarios();

    for (const auto &cenario : cenarios)
    {
        for (int tipo = 0; tipo < 2; ++tipo)
        {
            const Grafo &grafo = (tipo == 0) ? grafo_km : grafo_min;
            const std::string medida = (tipo == 0) ? "Distancia (Km)" : "Tempo (Min)";
            Grafo subgrafo = grafo;
            subgrafo = Grafo();
            subgrafo.n = static_cast<int>(cenario.indices.size());
            subgrafo.custo.assign(subgrafo.n, std::vector<double>(subgrafo.n, 0.0));
            subgrafo.nomes.assign(subgrafo.n, "");
            for (int i = 0; i < subgrafo.n; ++i)
            {
                subgrafo.nomes[i] = grafo.nomeDoVertice(cenario.indices[i]);
                for (int j = 0; j < subgrafo.n; ++j)
                {
                    subgrafo.custo[i][j] = grafo.custo[cenario.indices[i]][cenario.indices[j]];
                }
            }

            auto t0 = std::chrono::high_resolution_clock::now();
            Rota rotaConstrutiva = construirRotaVizinhoMaisProximo(subgrafo, 0);
            double custoConstrutivo = subgrafo.custoRota(rotaConstrutiva);
            auto t1 = std::chrono::high_resolution_clock::now();
            Rota rotaFinal = buscaLocal2opt(subgrafo, rotaConstrutiva);
            double custoFinal = subgrafo.custoRota(rotaFinal);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> tempoConstrucao = t1 - t0;
            std::chrono::duration<double> tempoBusca = t2 - t1;
            std::chrono::duration<double> tempoTotal = t2 - t0;

            std::cout << "\nCenario: " << cenario.nome << " [" << medida << "]\n";
            std::cout << "  Custo construtivo: " << std::fixed << std::setprecision(2) << custoConstrutivo << "\n";
            std::cout << "  Custo apos 2-opt:  " << std::fixed << std::setprecision(2) << custoFinal << "\n";
            std::cout << "  Tempo construcao: " << std::fixed << std::setprecision(6) << tempoConstrucao.count() << " s\n";
            std::cout << "  Tempo 2-opt:      " << std::fixed << std::setprecision(6) << tempoBusca.count() << " s\n";
            std::cout << "  Tempo total:      " << std::fixed << std::setprecision(6) << tempoTotal.count() << " s\n";
            std::cout << "  Rota final: " << rotaParaString(subgrafo, rotaFinal) << "\n";
        }
    }
}

static void executarInsercaoMaisBarata(const Grafo &grafo_km, const Grafo &grafo_min)
{
    std::cout << "\n>> Executando Insercao Mais Barata + Busca Local Shift...\n";
    auto cenarios = criarCenarios();

    for (const auto &cenario : cenarios)
    {
        for (int tipo = 0; tipo < 2; ++tipo)
        {
            const Grafo &grafo = (tipo == 0) ? grafo_km : grafo_min;
            const std::string medida = (tipo == 0) ? "Distancia (Km)" : "Tempo (Min)";
            Grafo subgrafo = Grafo();
            subgrafo.n = static_cast<int>(cenario.indices.size());
            subgrafo.custo.assign(subgrafo.n, std::vector<double>(subgrafo.n, 0.0));
            subgrafo.nomes.assign(subgrafo.n, "");
            for (int i = 0; i < subgrafo.n; ++i)
            {
                subgrafo.nomes[i] = grafo.nomeDoVertice(cenario.indices[i]);
                for (int j = 0; j < subgrafo.n; ++j)
                {
                    subgrafo.custo[i][j] = grafo.custo[cenario.indices[i]][cenario.indices[j]];
                }
            }

            auto t0 = std::chrono::high_resolution_clock::now();
            Rota rotaConstrutiva = construirRotaInsercaoMaisBarata(subgrafo);
            double custoConstrutivo = subgrafo.custoRota(rotaConstrutiva);
            auto t1 = std::chrono::high_resolution_clock::now();
            Rota rotaFinal = buscaLocalShift(subgrafo, rotaConstrutiva);
            double custoFinal = subgrafo.custoRota(rotaFinal);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> tempoConstrucao = t1 - t0;
            std::chrono::duration<double> tempoShift = t2 - t1;
            std::chrono::duration<double> tempoTotal = t2 - t0;

            std::cout << "\nCenario: " << cenario.nome << " [" << medida << "]\n";
            std::cout << "  Custo construtivo: " << std::fixed << std::setprecision(2) << custoConstrutivo << "\n";
            std::cout << "  Custo apos shift:  " << std::fixed << std::setprecision(2) << custoFinal << "\n";
            std::cout << "  Tempo construcao: " << std::fixed << std::setprecision(6) << tempoConstrucao.count() << " s\n";
            std::cout << "  Tempo shift:       " << std::fixed << std::setprecision(6) << tempoShift.count() << " s\n";
            std::cout << "  Tempo total:       " << std::fixed << std::setprecision(6) << tempoTotal.count() << " s\n";
            std::cout << "  Rota final: " << rotaParaString(subgrafo, rotaFinal) << "\n";
        }
    }
}

static void executarAlgoritmoGenetico(const std::vector<std::vector<double>> &matriz_km, const std::vector<std::vector<double>> &matriz_min)
{
    std::cout << "\n>> Executando Algoritmo Genetico...\n";
    auto cenarios = criarCenarios();

    for (const auto &cenario : cenarios)
    {
        for (int tipo = 0; tipo < 2; ++tipo)
        {
            const auto &matriz = (tipo == 0) ? matriz_km : matriz_min;
            const std::string medida = (tipo == 0) ? "Distancia (Km)" : "Tempo (Min)";
            auto submatriz = extrairSubmatriz(matriz, cenario.indices);

            int tamanho_pop = (static_cast<int>(cenario.indices.size()) < 10) ? 50 : 100;
            int geracoes = (static_cast<int>(cenario.indices.size()) < 10) ? 200 : 500;
            double taxa_mutacao = 0.05;

            auto t0 = std::chrono::high_resolution_clock::now();
            double custoFinal = executar_ag(submatriz, tamanho_pop, geracoes, taxa_mutacao);
            auto t1 = std::chrono::high_resolution_clock::now();

            std::cout << "\nCenario: " << cenario.nome << " [" << medida << "]\n";
            std::cout << "  Custo final: " << std::fixed << std::setprecision(2) << custoFinal << "\n";
            std::cout << "  Tempo total: " << std::fixed << std::setprecision(6) << std::chrono::duration<double>(t1 - t0).count() << " s\n";
        }
    }
}

static void executarAlgoritmoMemetico(const std::vector<std::vector<double>> &matriz_km, const std::vector<std::vector<double>> &matriz_min)
{
    std::cout << "\n>> Executando Algoritmo Memetico...\n";
    auto cenarios = criarCenarios();

    for (const auto &cenario : cenarios)
    {
        for (int tipo = 0; tipo < 2; ++tipo)
        {
            const auto &matriz = (tipo == 0) ? matriz_km : matriz_min;
            const std::string medida = (tipo == 0) ? "Distancia (Km)" : "Tempo (Min)";
            auto submatriz = extrairSubmatriz(matriz, cenario.indices);

            int tamanho_pop = (static_cast<int>(cenario.indices.size()) < 12) ? 15 : 40;
            int geracoes = (static_cast<int>(cenario.indices.size()) < 12) ? 30 : 100;
            double taxa_mutacao = 0.05;
            double prob_busca_local = 0.25;

            auto t0 = std::chrono::high_resolution_clock::now();
            AlgoritmoMemetico am(submatriz, tamanho_pop, geracoes, taxa_mutacao, prob_busca_local);
            double custoFinal = am.executar();
            auto t1 = std::chrono::high_resolution_clock::now();

            std::cout << "\nCenario: " << cenario.nome << " [" << medida << "]\n";
            std::cout << "  Custo final: " << std::fixed << std::setprecision(2) << custoFinal << "\n";
            std::cout << "  Tempo total: " << std::fixed << std::setprecision(6) << std::chrono::duration<double>(t1 - t0).count() << " s\n";
        }
    }
}

int main()
{
    Grafo grafo_km;
    Grafo grafo_min;
    try
    {
        grafo_km.carregarDeArquivo("dados/grafo_km.txt");
        grafo_km.carregarNomesDeArquivo("dados/cidades.txt");
        grafo_min.carregarDeArquivo("dados/grafo_min.txt");
        grafo_min.carregarNomesDeArquivo("dados/cidades.txt");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro ao carregar dados: " << e.what() << "\n";
        return 1;
    }

    auto matriz_km = carregarMatriz("dados/grafo_km.txt");
    auto matriz_min = carregarMatriz("dados/grafo_min.txt");
    if (matriz_km.empty() || matriz_min.empty())
    {
        std::cerr << "Erro ao carregar a(s) matriz(es) de custo.\n";
        return 1;
    }

    while (true)
    {
        mostrarMenu();
        int escolha = lerEscolha();

        switch (escolha)
        {
            case 1:
                executarVizinhoMaisProximo(grafo_km, grafo_min);
                break;
            case 2:
                executarInsercaoMaisBarata(grafo_km, grafo_min);
                break;
            case 3:
                executarAlgoritmoGenetico(matriz_km, matriz_min);
                break;
            case 4:
                executarAlgoritmoMemetico(matriz_km, matriz_min);
                break;
            case 5:
                std::cout << "Encerrando programa.\n";
                return 0;
            default:
                std::cout << "Opcao invalida. Digite 1-5.\n";
                break;
        }

        std::cout << "\nPressione Enter para voltar ao menu...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    return 0;
}
