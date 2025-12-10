#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
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

typedef struct pos_s {
    size_t row;
    size_t col;
} pos_t;

static size_t part1(const pos_t *pos, size_t size);
static size_t get_size(const filemap_t *fm);
static pos_t *init(const filemap_t *fm, size_t size);
static void free_prog(filemap_t *fm, pos_t *pos);
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

    size_t size = get_size(&fm);
    pos_t *pos = init(&fm, size);
    if (!pos) {
        free_prog(&fm, pos);
        return 3;
    }

    size_t result1 = part1(pos, size);

    printf("Part: 1: Largest rectangle: %zu\n", result1);
    free_prog(&fm, pos);

    return 0;
}

static size_t part1(const pos_t *pos, size_t size) {
    (void)pos;
    long long int max = 0;

    size_t index = 0;
    while (index < size) {
        size_t sub_index = index + 1;
        while (sub_index < size) {
            long int row_dif =
                ((long int)pos[index].row - (long int)pos[sub_index].row) + 1;
            if (row_dif < 0) {
                row_dif = -row_dif;
            }

            long int col_dif =
                ((long int)pos[index].col - (long int)pos[sub_index].col) + 1;
            if (col_dif < 0) {
                col_dif = -col_dif;
            }

            long int result = row_dif * col_dif;
            if (result > max) {
                max = result;
            }
            ++sub_index;
        }

        ++index;
    }

    return (size_t)max;
}

static size_t get_size(const filemap_t *fm) {
    const char *buf = fm->buf;
    size_t result = 0;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return result;
        }

        ++result;
        buf = c + 1;
    }

    return result;
}

static pos_t *init(const filemap_t *fm, size_t size) {
    pos_t *pos = calloc(size + 1, sizeof(*pos));
    if (!pos) {
        return NULL;
    }

    size_t index = 0;
    const char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return pos;
        }

        size_t len = (size_t)(c - buf);
        char *endptr;
        size_t col = strtoul(buf, &endptr, 10);

        const char *comma = memchr(buf, ',', len);
        if (!comma) {
            free(pos);
            return NULL;
        }
        size_t row = strtoul(comma + 1, &endptr, 10);

        pos_t n_pos = {.row = row, .col = col};
        pos[index] = n_pos;

        buf = c + 1;
        ++index;
    }

    return pos;
}

static void free_prog(filemap_t *fm, pos_t *pos) {
    if (fm) {
        close(fm->fd);
        munmap(fm->buf, fm->size);
    }

    if (pos) {
        free(pos);
    }
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
