#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
        output[N] = '\0';
    } else {
        output[0] = '\0';
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);

    int fd_read = open(FIFO_READ, O_RDONLY);
    int fd_write = open(FIFO_WRITE, O_WRONLY);

    char buffer[BUFFER_SIZE + 1];
    ssize_t bytes_read = read(fd_read, buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        char output[BUFFER_SIZE + 1];
        process_data(buffer, output, N);
        write(fd_write, output, strlen(output));
    }

    close(fd_read);
    close(fd_write);

    return 0;
}
