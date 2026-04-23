SRC_DIR = .
BIN_DIR = bin

SOURCE = $(SRC_DIR)/nos.c
TARGET = nos

N_THREADS = 8
DELAY = 5

compilar: clean
	@mkdir -p $(BIN_DIR)
	gcc $(SOURCE) -o $(BIN_DIR)/$(TARGET) -pthread

run: compilar
	@echo "Executando com medicao de tempo (overhead)..."
	time ./$(BIN_DIR)/$(TARGET) -n $(N_THREADS) -t $(DELAY)

teste10x: compilar
	@echo "Iniciando bateria de 10 testes (-n $(N_THREADS) -t $(DELAY))"
	@for i in {1..10}; do \
		echo "----- Rodada $$i -----"; \
		./$(BIN_DIR)/$(TARGET) -n $(N_THREADS) -t $(DELAY) | grep -E "Nos na lista|Validacao de integridade"; \
	done

clean:
	@rm -rf $(BIN_DIR)
