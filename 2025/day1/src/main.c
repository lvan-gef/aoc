#include <fcntl.h>
#include <stdbool.h>
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

    int dail = 50;
    int p1_counter = 0;
    int p2_counter = 0;
    char *buf = fm.buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm.size - (size_t)(buf - fm.buf));
        size_t len = (size_t)(c - buf);
        char line[6] = {0};
        memcpy(line, buf, len);

        int value = atoi(line + 1);
        for (int index = 0; index < value; ++index) {
            if (line[0] == 'L') {
                dail = (dail - 1 + 100) % 100;
            } else {
                dail = (dail + 1) % 100;
            }

            if (dail == 0) {
                ++p2_counter;
            }
        }

        if (dail == 0) {
            ++p1_counter;
        }

        buf = c + 1;
    }

    printf("Part: 1: The password is: %d\n", p1_counter);
    printf("Part: 2: The password is: %d\n", p2_counter);
    close(fm.fd);
    munmap(fm.buf, fm.size);

    return 0;
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
