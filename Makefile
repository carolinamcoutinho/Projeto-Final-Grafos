# Makefile para Projeto-Final-Grafos
# Compila incrementalmente apenas os arquivos alterados.

CXX := g++
CXXFLAGS := -Iinclude -Wall
LDFLAGS :=

SRC_DIR := src
SRCS := $(SRC_DIR)/main.cpp \
        $(SRC_DIR)/vizinho_mais_proximo.cpp \
        $(SRC_DIR)/insercao_mais_barata.cpp \
        $(SRC_DIR)/algoritmo_genetico.cpp \
        $(SRC_DIR)/algoritmo_memetico.cpp

OBJS := $(SRCS:$(SRC_DIR)/%.cpp=%.o)
TARGET := main.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJS) 2>nul || true
	del /Q $(TARGET) 2>nul || true

.PHONY: all clean
