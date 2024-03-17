#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 5000

void process_data(const char *input, char *output, int N) {
    int length = strlen(input);
    int lastStart = -1;

    for (int i = 0; i <= length - N; ++i) {
        int valid = 1;
        for (int j = i; j < i + N - 1; ++j) {
            if (input[j] >= input[j + 1]) {
                valid = 0;
                break;
            }
        }
        if (valid) {
            lastStart = i;
        }
    }
    if (lastStart != -1) {
        strncpy(output, input + lastStart, N);
        output[N] = '\0';
    } else {
        output[0] = '\0';
    }
}

void create_fifo(const char *fifo_path) {
    if (mkfifo(fifo_path, 0666) && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <output file> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *fifo1 = "/tmp/fifo1";
    const char *fifo2 = "/tmp/fifo2";

    create_fifo(fifo1);
    create_fifo(fifo2);

    int N = atoi(argv[3]);
    pid_t cpid1, cpid2, cpid3;

    cpid1 = fork();
    if (cpid1 == 0) {
        int input_fd = open(argv[1], O_RDONLY);
        int fifo1_fd = open(fifo1, O_WRONLY);
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
            write(fifo1_fd, buffer, bytes_read);
        }

        close(input_fd);
        close(fifo1_fd);
        exit(EXIT_SUCCESS);
    }

    cpid2 = fork();
    if (cpid2 == 0) {
        int fifo1_fd = open(fifo1, O_RDONLY);
        int fifo2_fd = open(fifo2, O_WRONLY);
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_read = read(fifo1_fd, buffer, BUFFER_SIZE);

        buffer[bytes_read] = '\0';
        char output[BUFFER_SIZE] = {0};
        process_data(buffer, output, N);

        write(fifo2_fd, output, strlen(output));
        close(fifo1_fd);
        close(fifo2_fd);
        exit(EXIT_SUCCESS);
    }

    cpid3 = fork();
    if (cpid3 == 0) {
        int fifo2_fd = open(fifo2, O_RDONLY);
        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(fifo2_fd, buffer, BUFFER_SIZE)) > 0) {
            write(output_fd, buffer, bytes_read);
        }

        close(fifo2_fd);
        close(output_fd);
        exit(EXIT_SUCCESS);
    }

    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);
    waitpid(cpid3, NULL, 0);

    unlink(fifo1);
    unlink(fifo2);

    return EXIT_SUCCESS;
}
