#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

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

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <output file> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[3]);
    int pipefd1[2], pipefd2[2];
    pid_t cpid;

    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) { // Child process
        close(pipefd1[1]); // Close unused write end of pipefd1
        close(pipefd2[0]); // Close unused read end of pipefd2

        char input[BUFFER_SIZE] = {0};
        ssize_t bytes_read = read(pipefd1[0], input, BUFFER_SIZE);
        if (bytes_read > 0) {
            input[bytes_read] = '\0'; // Ensure null-termination
            char output[BUFFER_SIZE] = {0};
            process_data(input, output, N);
            write(pipefd2[1], output, strlen(output)); // Send processed data back
        }

        close(pipefd1[0]);
        close(pipefd2[1]);
        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipefd1[0]); // Close unused read end of pipefd1
        close(pipefd2[1]); // Close unused write end of pipefd2

        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd < 0) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            write(pipefd1[1], buffer, bytes_read); // Send data to child
        }
        close(input_fd);
        close(pipefd1[1]);

        wait(NULL); // Wait for child to process data

        ssize_t bytes_written = read(pipefd2[0], buffer, BUFFER_SIZE);
        if (bytes_written > 0) {
            int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            write(output_fd, buffer, bytes_written); // Write processed data to output file
            close(output_fd);
        }
        close(pipefd2[0]);
    }

    return EXIT_SUCCESS;
}
