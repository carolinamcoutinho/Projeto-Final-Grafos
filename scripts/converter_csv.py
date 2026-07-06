#!/usr/bin/env python3
"""
Conversor (via CSV): arquivos km.csv, min.csv e cidades.csv -> arquivos de
texto (.txt) no formato lido pela classe Grafo em C++.

SAÍDA (gerada em dados/):
  - cidades.txt   -> lista "indice;nome" (uma cidade por linha, 0..N-1)
  - grafo_km.txt  -> grafo com pesos em quilômetros
  - grafo_min.txt -> grafo com pesos em minutos

Formato dos arquivos grafo_*.txt (lido pela classe Grafo em C++):
  linha 1........: N (quantidade de vértices/cidades)
  linhas 2..N+1...: matriz de custos N x N, uma linha por vértice,
                     valores separados por espaço, diagonal = 0.0
"""
import csv
import os

PASTA_DADOS = "../dados"

CAMINHO_KM = os.path.join(PASTA_DADOS, "km.csv")
CAMINHO_MIN = os.path.join(PASTA_DADOS, "min.csv")
CAMINHO_CIDADES = os.path.join(PASTA_DADOS, "cidades.csv")


def detectar_delimitador(caminho):
    """Lê a primeira linha do arquivo e decide se o separador é ',' ou ';'.
    """
    with open(caminho, encoding="utf-8-sig") as f:
        primeira_linha = f.readline()
    if primeira_linha.count(";") > primeira_linha.count(","):
        return ";"
    return ","


def para_float(texto):
    """Converte uma célula do CSV para float, tratando célula vazia como
    0.0 (diagonal da matriz) e aceitando tanto '.' quanto ',' como
    separador decimal
    """
    texto = texto.strip()
    if texto == "":
        return 0.0
    if "," in texto and "." not in texto:
        texto = texto.replace(",", ".")
    else:
        texto = texto.replace(",", "")  # remove eventual separador de milhar
    return float(texto)


def ler_matriz_csv(caminho):
    """Lê uma matriz de custos a partir do arquivo CSV exportado

    Entrada : caminho do arquivo CSV (ex.: dados/km.csv)
    Saída   : lista de listas (matriz NxN) com os custos em float.
    """
    delimitador = detectar_delimitador(caminho)
    with open(caminho, encoding="utf-8-sig", newline="") as f:
        leitor = csv.reader(f, delimiter=delimitador)
        linhas = [linha for linha in leitor if any(c.strip() for c in linha)]

    n = len(linhas) - 1  # descontando a linha de cabeçalho
    matriz = []
    for i in range(1, n + 1):
        linha_valores = []
        for j in range(1, n + 1):
            celula = linhas[i][j] if j < len(linhas[i]) else ""
            linha_valores.append(para_float(celula))
        matriz.append(linha_valores)
    return matriz


def ler_cidades_csv(caminho):
    """Lê a lista de cidades a partir do CSV exportado da aba 'Cidades'.

    Entrada : caminho do arquivo CSV (ex.: dados/cidades.csv)
    Saída   : lista de nomes de cidades, ordenada pelo id (1..N).
    """
    delimitador = detectar_delimitador(caminho)
    with open(caminho, encoding="utf-8-sig", newline="") as f:
        leitor = csv.reader(f, delimiter=delimitador)
        linhas = [linha for linha in leitor if any(c.strip() for c in linha)]

    cidades = {}
    for linha in linhas[1:]:  # pula o cabeçalho
        for k in (0, 2):
            if k + 1 >= len(linha):
                continue
            id_texto, nome = linha[k].strip(), linha[k + 1].strip()
            if id_texto != "" and nome != "":
                cidades[int(float(id_texto))] = nome

    ids_ordenados = sorted(cidades.keys())
    return [cidades[i] for i in ids_ordenados]


def escrever_grafo(caminho, n, matriz):
    """Grava a matriz de custos no formato lido pela classe Grafo (C++)."""
    with open(caminho, "w", encoding="utf-8") as f:
        f.write(f"{n}\n")
        for linha in matriz:
            f.write(" ".join(f"{v:.4f}" for v in linha) + "\n")


def escrever_cidades(caminho, nomes):
    """Grava a lista de cidades no formato 'indice;nome', uma por linha."""
    with open(caminho, "w", encoding="utf-8") as f:
        for idx, nome in enumerate(nomes):
            f.write(f"{idx};{nome}\n")


def main():
    print("Lendo cidades.csv...")
    nomes = ler_cidades_csv(CAMINHO_CIDADES)
    n = len(nomes)
    print(f"Cidades encontradas: {n}")

    print("Lendo km.csv...")
    matriz_km = ler_matriz_csv(CAMINHO_KM)
    print("Lendo min.csv...")
    matriz_min = ler_matriz_csv(CAMINHO_MIN)

    # validação simples: dimensões devem bater com a quantidade de cidades
    if len(matriz_km) != n or len(matriz_min) != n:
        print("AVISO: a quantidade de linhas das matrizes não bate com a "
              f"quantidade de cidades (n={n}, km={len(matriz_km)}, "
              f"min={len(matriz_min)}). Confira os arquivos CSV exportados.")

    escrever_cidades(os.path.join(PASTA_DADOS, "cidades.txt"), nomes)
    escrever_grafo(os.path.join(PASTA_DADOS, "grafo_km.txt"), n, matriz_km)
    escrever_grafo(os.path.join(PASTA_DADOS, "grafo_min.txt"), n, matriz_min)

    print("Arquivos gerados em", PASTA_DADOS)


if __name__ == "__main__":
    main()
