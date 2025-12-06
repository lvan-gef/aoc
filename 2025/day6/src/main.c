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

typedef struct map_s {
    size_t row;
    size_t col;
} map_t;

static void get_size(const filemap_t *fm, map_t *map);
static char *set_nbrs(filemap_t *fm, size_t **matrix);
static size_t calc(size_t **matrix, size_t col, char operator, size_t size);
static void free_prog(filemap_t *fm, size_t **matrix);
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

    map_t map = {0};
    get_size(&fm, &map);

    size_t **matrix = (size_t **)calloc(map.row + 1, sizeof(*matrix));
    if (!matrix) {
        free_prog(&fm, NULL);
        return 3;
    }

    size_t index = 0;
    while (index < map.row) {
        matrix[index] = calloc(map.col + 1, sizeof(size_t));
        if (!matrix[index]) {
            free_prog(&fm, matrix);
            return 4;
        }
        ++index;
    }

    const char *operators = set_nbrs(&fm, matrix);
    size_t result = 0;
    index = 0;
    while (*operators) {
        while (*operators == ' ') {
            ++operators;
        }

        result += calc(matrix, index, *operators, map.row);

        ++index;
        ++operators;
    }

    printf("Part: 1: The sum is: %zu\n", result);
    free_prog(&fm, matrix);

    return 0;
}

static void get_size(const filemap_t *fm, map_t *map) {
    const char *buf = fm->buf;

    while (*buf) {
        size_t col = 0;
        while (*buf != '\n') {
            while (*buf == ' ') {
                ++buf;
            }

            ++col;
            while (*buf != ' ' && *buf != '\n') {
                ++buf;
            }
        }

        if (col > map->col) {
            map->col = col;
        }

        ++map->row;
        ++buf;
    }
}

static char *set_nbrs(filemap_t *fm, size_t **matrix) {
    char *buf = fm->buf;
    size_t row = 0;

    while (*buf) {
        size_t col = 0;
        while (*buf != '\n') {
            while (*buf == ' ') {
                ++buf;
            }

            if (*buf == '+' || *buf == '-' || *buf == '*' || *buf == '\\') {
                return buf;
            }

            char *endptr;
            matrix[row][col] = strtoul(buf, &endptr, 10);
            while (*buf != ' ' && *buf != '\n') {
                ++buf;
            }
            ++col;
        }

        ++row;
        ++buf;
    }

    return NULL;
}

static size_t calc(size_t **matrix, size_t col, char operator, size_t size) {
    size_t sum = 0;
    size_t sub_index = 0;

    while (sub_index < size - 1) {
        if (operator == '+') {
            sum += matrix[sub_index][col];
        } else if (operator == '*') {
            if (sum == 0) {
                sum = matrix[sub_index][col];
            } else {
                sum *= matrix[sub_index][col];
            }
        } else {
            sum += 0;
        }

        ++sub_index;
    }

    return sum;
}

static void free_prog(filemap_t *fm, size_t **matrix) {
    if (fm) {
        close(fm->fd);
        munmap(fm->buf, fm->size);
    }

    if (matrix) {
        size_t index = 0;
        while (matrix[index]) {
            free(matrix[index]);
            ++index;
        }

        free(matrix);
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
