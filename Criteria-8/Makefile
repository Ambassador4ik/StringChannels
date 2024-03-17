# Определяем компилятор и флаги
CC = gcc
CFLAGS = -Wall -g

# Исходные файлы для двух программ
SRC_SENDER = sender.c
SRC_RECEIVER = receiver.c

# Исполняемые файлы для двух программ
EXEC_SENDER = sender
EXEC_RECEIVER = receiver

# Директория для сборки
BUILD_DIR = build

# Создание директории build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Правила для компиляции sender и receiver
$(BUILD_DIR)/$(EXEC_SENDER): $(SRC_SENDER) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(EXEC_RECEIVER): $(SRC_RECEIVER) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Сборка обоих исполняемых файлов
build: $(BUILD_DIR)/$(EXEC_SENDER) $(BUILD_DIR)/$(EXEC_RECEIVER)

# Правило для очистки проекта
clean:
	rm -rf $(BUILD_DIR)

# Запуск программ
run: build
	$(BUILD_DIR)/$(EXEC_RECEIVER) $(count) &
	$(BUILD_DIR)/$(EXEC_SENDER) $(input) $(output)
	wait

# Сделаем правила clean и run специальными
.PHONY: clean run build
