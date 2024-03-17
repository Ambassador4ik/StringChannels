#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 5000
#define FIFO_READ "/tmp/fifo_read"
#define FIFO_WRITE "/tmp/fifo_write"

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
        if (valid && i + N <= length) {
            lastStart = i;
        }
    }
    if (lastStart != -1) {
        strncpy(output, input + lastStart, N);
        output[N] = '\0'; // Ensure the string is properly terminated
    } else {
        output[0] = '\0';
    }
}

void create_fifo(const char *fifo_path) {
    if (mkfifo(fifo_path, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <output file> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    create_fifo(FIFO_READ);
    create_fifo(FIFO_WRITE);

    int N = atoi(argv[3]);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child
        int fd_read = open(FIFO_READ, O_RDONLY);
        int fd_write = open(FIFO_WRITE, O_WRONLY);
        char buffer[BUFFER_SIZE + 1]; // +1 for the null terminator

        ssize_t bytes_read = read(fd_read, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null terminate the string
            char output[BUFFER_SIZE + 1];
            process_data(buffer, output, N);
            write(fd_write, output, strlen(output));
        }

        close(fd_read);
        close(fd_write);
    } else { // Parent
        int fd_write = open(FIFO_READ, O_WRONLY);
        int fd_read = open(FIFO_WRITE, O_RDONLY);

        int input_fd = open(argv[1], O_RDONLY);
        char buffer[BUFFER_SIZE + 1]; // +1 for the null terminator

        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null terminate the string
            write(fd_write, buffer, bytes_read);
        }

        wait(NULL); // Wait for child to finish processing

        ssize_t bytes_written = read(fd_read, buffer, BUFFER_SIZE);
        if (bytes_written > 0) {
            int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            write(output_fd, buffer, bytes_written);
            close(output_fd);
        }

        close(fd_write);
        close(fd_read);
        close(input_fd);

        unlink(FIFO_READ);
        unlink(FIFO_WRITE);
    }

    return EXIT_SUCCESS;
}
