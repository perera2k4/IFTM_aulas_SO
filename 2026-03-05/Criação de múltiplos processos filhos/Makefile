
SRC_DIR = src
BIN_DIR = bin

# Código-fonte
SOURCE = $(SRC_DIR)/fork-n-filhos.c

# Executável final
TARGET = fork_n_filhos.exe

run: compilar
	./$(BIN_DIR)/$(TARGET) 5

compilar: clean
	@mkdir -p $(BIN_DIR)
	gcc $(SOURCE) -o $(BIN_DIR)/$(TARGET)

clean:
	@rm -rf $(BIN_DIR)/*l