SRC_DIR = .
BIN_DIR = bin

SOURCE = $(SRC_DIR)/vendas.c
TARGET = vendas.exe

N_THREADS = 10
DELAY = 5

run: compilar
	@echo "Executando teste com medição de tempo (Overhead)..."
	time ./$(BIN_DIR)/$(TARGET) -n $(N_THREADS) -t $(DELAY)

compilar: clean
	@mkdir -p $(BIN_DIR)
	gcc $(SOURCE) -o $(BIN_DIR)/$(TARGET) -pthread

teste10x: compilar
	@echo "iniciando bateria de 10 testes para validacao"
	@for i in {1..10}; do \
		echo -n "rodada $$i: "; \
		./$(BIN_DIR)/$(TARGET) -n $(N_THREADS) -t $(DELAY) | grep "Vendas"; \
	done

clean:
	@rm -rf $(BIN_DIR)