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

typedef struct map_s {
    pos_t pos;
    char **grid;
} map_t;

static size_t part1(const map_t *map);
static size_t rectangel(const map_t *map, pos_t *pos);
static void get_size(const filemap_t *fm, map_t *map);
static bool init(const filemap_t *fm, map_t *map);
static void free_prog(filemap_t *fm, map_t *map);
static void free_grid(char **grid);
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

    if (!init(&fm, &map)) {
        free_prog(&fm, &map);
        return 3;
    }

    size_t result1 = part1(&map);

    printf("Part: 1: Beam splitting: %zu\n", result1);
    free_prog(&fm, &map);

    return 0;
}

static size_t part1(const map_t *map) {

    size_t row = 0;
    size_t result = 0;
    while (row < map->pos.row) {
        size_t col = 0;
        while (col < map->pos.col) {
            if (map->grid[row][col] == '#') {
                pos_t cur = {.row = row, .col = col};
                size_t size = rectangel(map, &cur);
                printf("%zu\n", size);
                if (size > result) {
                    result = size;
                }
            }
            ++col;
        }
        ++row;
    }

    return result;
}

static size_t rectangel(const map_t *map, pos_t *pos) {
    // walk left
    pos_t cur_pos = {.row = pos->row, .col = pos->col};
    pos_t left_pos = {0};
    while (cur_pos.row < map->pos.row && cur_pos.col < map->pos.col) {
        if (map->grid[cur_pos.row][cur_pos.col] == '#') {
            printf("cur: %zu, %zu\npos: %zu, %zu\n\n", cur_pos.row, cur_pos.col, pos->row, pos->col);
            if (cur_pos.row > left_pos.row && cur_pos.col > left_pos.col) {
                left_pos.row = cur_pos.row;
                left_pos.col = cur_pos.col;
            }
        }

        ++cur_pos.row;
        ++cur_pos.col;
    }
    size_t left = (left_pos.col - pos->col) * (left_pos.row - pos->row);

    // walk rigth
    cur_pos.row = pos->row;
    cur_pos.col = pos->col;
    pos_t rigth_pos = {0};
    while (cur_pos.row < map->pos.row && cur_pos.col < map->pos.col) {
        if (map->grid[cur_pos.row][cur_pos.col] == '#') {
            printf("cur: %zu, %zu\npos: %zu, %zu\n\n", cur_pos.row, cur_pos.col, pos->row, pos->col);
            if (cur_pos.row > rigth_pos.row && cur_pos.col > rigth_pos.col) {
                rigth_pos.row = cur_pos.row;
                rigth_pos.col = cur_pos.col;
            }
        }

        --cur_pos.row;
        --cur_pos.col;
    }
    size_t rigth = (rigth_pos.col - pos->col) * (rigth_pos.row - pos->row);

    if (left > rigth) {
        return left;
    }

    return rigth;
}

static void get_size(const filemap_t *fm, map_t *map) {
    const char *buf = fm->buf;

    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return;
        }

        size_t col = (size_t)(c - buf);
        if (col >= map->pos.col) {
            map->pos.col = col;
        }
        ++map->pos.row;

        buf = c + 1;
    }
}

static bool init(const filemap_t *fm, map_t *map) {
    map->grid = calloc(map->pos.row + 1, sizeof(*map->grid));
    if (!map->grid) {
        return false;
    }

    size_t index = 0;
    const char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return true;
        }

        map->grid[index] = calloc(map->pos.col + 1, sizeof(**map->grid));
        if (!map->grid[index]) {
            goto failed;
        }
        strncpy(map->grid[index], buf, map->pos.col);

        buf = c + 1;
        ++index;
    }

    return true;
failed:
    if (map->grid) {
        free_grid(map->grid);
        map->grid = NULL;
    }

    return false;
}

static void free_prog(filemap_t *fm, map_t *map) {
    if (fm) {
        close(fm->fd);
        munmap(fm->buf, fm->size);
    }

    free_grid(map->grid);
}

static void free_grid(char **grid) {
    if (grid) {
        size_t index = 0;
        while (grid[index]) {
            free(grid[index]);
            ++index;
        }

        free(grid);
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
