//
// Created by 于承业 on 2023/10/23.
//
#include "chunk.h"
#include "unistd.h"
#include "fcntl.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
            printf("failed to open %s : %s\n", argv[1], strerror(errno));
            exit(-1);
        }
        struct stat st;
        if (fstat(fd, &st) == -1) {
            printf("failed to get file stat\n");
            exit(-1);
        }
        char *buf = new char[st.st_size];
        size_t n = read(fd, buf, st.st_size);
        if (n != st.st_size) {
            printf("failed to read (%zu vs %lld)\n", n, st.st_size);
            exit(-1);
        }
        close(fd);
        Chunk chunk(buf, st.st_size);
    }
}