
SRC_DIR = src
BIN_DIR = bin

SRC_NAME_ASSINC = banco
SRC_NAME_MUTEX = banco-mutex
SRC_NAME_SEM_BIN = banco-semaforo-binario
SRC_NAME_SEM_CON = banco-semaforo-contador

# Código-fonte
SOURCE_ASSINC = $(SRC_DIR)/$(SRC_NAME_ASSINC).c
SOURCE_MUTEX  = $(SRC_DIR)/$(SRC_NAME_MUTEX).c
SOURCE_SEMBIN = $(SRC_DIR)/$(SRC_NAME_SEM_BIN).c
SOURCE_SEMCON = $(SRC_DIR)/$(SRC_NAME_SEM_CON).c

# Executável final
TARGET_1 = $(SRC_NAME_ASSINC).exe
TARGET_2 = $(SRC_NAME_MUTEX).exe
TARGET_3 = $(SRC_NAME_SEM_BIN).exe
TARGET_4 = $(SRC_NAME_SEM_CON).exe

exec:
	./$(BIN_DIR)/$(TARGET_1) -n 50 -t 5
	./$(BIN_DIR)/$(TARGET_2) -n 50 -t 5
	./$(BIN_DIR)/$(TARGET_3) -n 50 -t 5
	./$(BIN_DIR)/$(TARGET_4) -n 50 -t 5

run: compilar exec

compilar: clean
	@mkdir -p $(BIN_DIR)
	gcc $(SOURCE_ASSINC) -o $(BIN_DIR)/$(TARGET_1)
	gcc $(SOURCE_MUTEX) -o $(BIN_DIR)/$(TARGET_2)
	gcc $(SOURCE_SEMBIN) -o $(BIN_DIR)/$(TARGET_3)
	gcc $(SOURCE_SEMCON) -o $(BIN_DIR)/$(TARGET_4)

clean:
	@rm -rf $(BIN_DIR)/*
