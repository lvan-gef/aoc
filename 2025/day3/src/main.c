#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct filemap_s {
    char *filename;
    char *buf;
    size_t size;
    int fd;
} filemap_t;

static size_t calc(const char *line, size_t len, int size);
static void read_file(filemap_t *fm);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Use like: ./main <filename>\n");
        return 1;
    }

    filemap_t fm = {.filename = argv[1]};
    read_file(&fm);
    if (fm.fd < 0) {
        return 2;
    }

    size_t p1_sum = 0;
    size_t p2_sum = 0;
    char *buf = fm.buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm.size - (size_t)(buf - fm.buf));
        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        p1_sum += calc(line, len, 2);
        p2_sum += calc(line, len, 12);

        buf = c + 1;
    }

    printf("Part: 1: Total output joltage: %zu\n", p1_sum);
    printf("Part: 2: Total output joltage: %zu\n", p2_sum);
    close(fm.fd);
    munmap(fm.buf, fm.size);

    return 0;
}

static size_t calc(const char *line, size_t len, int size) {
    int strlen = (int)len;
    char stack[size + 1];
    int top = -1;

    for (int index = 0; index < strlen; ++index) {
        char cur = line[index];
        while (top >= 0 && stack[top] < cur &&
               (top + 1 + (strlen - index) > size)) {
            --top;
        }

        if (top + 1 < size) {
            stack[++top] = cur;
        }
    }

    stack[size] = '\0';
    char *endptr;
    return strtoul(stack, &endptr, 10);
}

static void read_file(filemap_t *fm) {
    const char *error = NULL;

    fm->fd = open(fm->filename, O_RDONLY);
    if (fm->fd < 0) {
        error = "Failed to open the file";
        goto failed;
    }

    struct stat st;
    if (fstat(fm->fd, &st) < 0) {
        error = "Failed to get fstat of the file";
        goto failed;
    }

    fm->buf = mmap(0, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fm->fd, 0);
    if (fm->buf == MAP_FAILED) {
        error = "Failed to mmap the file to mem";
        goto failed;
    }

    fm->size = (size_t)st.st_size;
    return;

failed:
    fprintf(stderr, "%s\n", error);
    if (fm->fd > 0) {
        close(fm->fd);
    }
    fm->fd = -1;
}
