#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main() {
    // Open a file
    int fd = open("test.txt", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Write to the file
    const char *str = "Hello, world!\n";
    ssize_t bytes_written = write(fd, str, strlen(str));
    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    // Close the file
    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
