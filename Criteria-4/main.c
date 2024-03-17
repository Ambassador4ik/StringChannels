#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5000

// Функция для процесса 2: обработка данных
void process_data(const char *input, char *output, int N) {
    int length = strlen(input);
    int lastStart = -1; // Индекс начала последней подходящей подстроки

    // Перебираем все возможные начальные позиции для подстрок длины N
    for (int i = 0; i <= length - N; ++i) {
        int valid = 1; // Флаг, указывающий, что текущая подстрока удовлетворяет условиям

        // Проверяем условие "каждый следующий символ больше предыдущего"
        for (int j = i; j < i + N - 1; ++j) {
            if (input[j] >= input[j + 1]) {
                valid = 0;
                break;
            }
        }

        // Если подстрока подходит, обновляем индекс начала последней подходящей подстроки
        if (valid) {
            lastStart = i;
        }
    }

    // Если подходящая подстрока найдена, копируем её в output
    if (lastStart != -1) {
        strncpy(output, input + lastStart, N);
        output[N] = '\0'; // Добавляем символ окончания строки
    } else {
        // Если подстрока не найдена, возвращаем пустую строку
        output[0] = '\0';
    }
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <output file> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int pipefd1[2], pipefd2[2];
    pid_t cpid1, cpid2;
    char buffer[BUFFER_SIZE] = {0};
    int N = atoi(argv[3]);

    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid1 = fork();
    if (cpid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid1 == 0) { // Child 1: Читает данные из файла и передает их через pipe
        close(pipefd1[0]); // Закрываем неиспользуемый конец чтения
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        write(pipefd1[1], buffer, bytes_read);
        close(pipefd1[1]);
        close(input_fd);
        exit(EXIT_SUCCESS);
    } else {
        cpid2 = fork();
        if (cpid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (cpid2 == 0) { // Child 2: Принимает данные, обрабатывает их и передает дальше
            close(pipefd1[1]); // Закрываем неиспользуемый конец записи
            close(pipefd2[0]); // Закрываем неиспользуемый конец чтения

            ssize_t bytes_read = read(pipefd1[0], buffer, BUFFER_SIZE);
            if (bytes_read == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            buffer[bytes_read] = '\0';

            char output[BUFFER_SIZE] = {0};
            process_data(buffer, output, N);

            write(pipefd2[1], output, strlen(output));
            close(pipefd1[0]);
            close(pipefd2[1]);
            exit(EXIT_SUCCESS);
        } else {
            // Parent
            close(pipefd1[0]); // Закрываем неиспользуемые концы
            close(pipefd1[1]);
            close(pipefd2[1]);

            wait(NULL); // Ждем завершения первого дочернего процесса
            wait(NULL); // Ждем завершения второго дочернего процесса

            // Читаем обработанные данные от второго дочернего процесса и записываем их в файл
            ssize_t bytes_read = read(pipefd2[0], buffer, BUFFER_SIZE);
            if (bytes_read == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }

            write(output_fd, buffer, bytes_read);
            close(pipefd2[0]);
            close(output_fd);
        }
    }

    return EXIT_SUCCESS;
}
