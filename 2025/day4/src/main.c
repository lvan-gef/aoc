#include <assert.h>
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

typedef struct map_size_s {
    unsigned int row;
    unsigned int column;
} map_size_t;

static size_t check_map(char **map, char **visit, map_size_t *ms, map_size_t *pos);
static void mapsize(filemap_t *fm, map_size_t *ms);
static char **create_map(map_size_t *ms);
static char **setmap(filemap_t *fm, map_size_t *ms);
static void free_map(char **map);
static void read_file(filemap_t *fm);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Use like: ./main <filename>\n");
        return 1;
    }

    char **map = NULL;
    char **visit = NULL;
    filemap_t fm = {.filename = argv[1]};
    read_file(&fm);
    if (fm.fd < 0) {
        goto failed;
    }

    map_size_t ms = {0};
    mapsize(&fm, &ms);
    map = setmap(&fm, &ms);
    if (!map) {
        goto failed;
    }

    visit = create_map(&ms);
    if (!visit) {
        goto failed;
    }

    int p1_counter = 0;
    size_t row = 0;
    while (row < ms.row) {
        size_t col = 0;
        while (col < ms.column) {
            map_size_t cur_pos = {.row = (unsigned int)row, .column = (unsigned int)col};
            p1_counter += check_map(map, visit, &ms, &cur_pos);
            ++col;
        }
        ++row;
    }

    printf("Part: 1: Rolls of paper: %d\n", p1_counter);
    close(fm.fd);
    munmap(fm.buf, fm.size);
    free(map);

    return 0;

failed:
    if (map) {
        free_map(map);
    }

    if (visit) {
        free_map(visit);
    }

    if (fm.fd) {
        close(fm.fd);
        munmap(fm.buf, fm.size);
    }

    return 2;
}

static size_t check_map(char **map, char **visit, map_size_t *ms, map_size_t *pos) {
    if (map[pos->row][pos->column] != '@' || visit[pos->row][pos->column]) {
        visit[pos->row][pos->column] = '1';
        return 0;
    }

    visit[pos->row][pos->column] = '1';
    size_t counter = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }

            int new_row = (int)pos->row + dr;
            int new_col = (int)pos->column + dc;
            if (new_row >= 0 && new_row < (int)ms->row &&
                new_col >= 0 && new_col < (int)ms->column) {
                if (map[new_row][new_col] == '@') {
                    counter++;
                }
            }
        }
    }

    return counter < 4 ? 1 : 0;
}

static void mapsize(filemap_t *fm, map_size_t *ms) {
    char *buf = fm->buf;

    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        size_t len = (size_t)(c - buf);

        ms->row += 1;
        if (len > ms->column) {
            ms->column = (unsigned int)len;
        }

        buf = c + 1;
    }
}

static char **create_map(map_size_t *ms) {
    char **map = calloc(ms->row + 1, sizeof(*map));
    if (!map) {
        return NULL;
    }

    size_t index = 0;
    while (index < ms->row) {
        map[index] = calloc(ms->column + 1, sizeof(char));
        if (!map[index]) {
            free_map(map);
            return NULL;
        }
        ++index;
    }

    return map;
}

static char **setmap(filemap_t *fm, map_size_t *ms) {
    char **map = create_map(ms);
    if (!map) {
        return NULL;
    }

    size_t index = 0;
    char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        memmove(map[index], buf, ms->column);
        buf = c + 1;
        ++index;
    }

    return map;
}

static void free_map(char **map) {
    size_t index = 0;

    while (map[index]) {
        free(map[index]);
        ++index;
    }

    free(map);
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
