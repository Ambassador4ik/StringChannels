# Определяем компилятор и флаги
CC = gcc
CFLAGS = -Wall -g

# Исходный и объектный файлы, а также исполняемый файл
SRC = main.c
OBJ = $(SRC:%.c=build/%.o)
EXEC = app

# Директория для сборки
BUILD_DIR = build

# Директория для входных файлов
IN_DIR = test/in

# Директория для выходных файлов
OUT_DIR = test/out

# Правило для создания папки build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Правило для сборки исполняемого файла
$(BUILD_DIR)/$(EXEC): $(OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Правило для объектных файлов, учитывает создание папки build
build/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для очистки проекта
clean:
	rm -rf $(BUILD_DIR)

# Правило для запуска программы
run: $(BUILD_DIR)/$(EXEC)
	cd $(BUILD_DIR) && ./$(EXEC) ../$(IN_DIR)/$(input) ../$(OUT_DIR)/$(output) $(count)

# Сделаем правила clean и run специальными, чтобы они не воспринимались как файлы
.PHONY: clean run
