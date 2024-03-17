#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 5000
#define FIFO_READ "/tmp/fifo_read"
#define FIFO_WRITE "/tmp/fifo_write"

void create_fifo(const char *fifo_path) {
    if (mkfifo(fifo_path, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    create_fifo(FIFO_READ);
    create_fifo(FIFO_WRITE);

    int fd_read = open(FIFO_WRITE, O_RDONLY);
    int fd_write = open(FIFO_READ, O_WRONLY);

    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE + 1];
    ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        write(fd_write, buffer, bytes_read);
    }

    close(input_fd);
    close(fd_write);

    ssize_t bytes_written = read(fd_read, buffer, BUFFER_SIZE);
    if (bytes_written > 0) {
        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
        write(output_fd, buffer, bytes_written);
        close(output_fd);
    }

    close(fd_read);

    unlink(FIFO_READ);
    unlink(FIFO_WRITE);

    return 0;
}
