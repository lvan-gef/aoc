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
    char **grid;
} map_t;

typedef struct node_s {
    ssize_t row;
    ssize_t col;
} node_t;

typedef struct queue_s {
    size_t head;
    size_t tail;
    node_t *nodes;
    bool **visited;
} queue_t;

static size_t part1(const map_t *map, queue_t *queue);
static void get_size(const filemap_t *fm, map_t *map);
static bool init(const filemap_t *fm, map_t *map, queue_t *queue);
static void get_start_pos(const map_t *map, node_t *node);
static void free_prog(filemap_t *fm, map_t *map, queue_t *queue);
static void free_grid(char **grid);
static void free_queue(queue_t *queue);
static void free_visited(bool **visited);
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

    queue_t queue = {0};
    if (!init(&fm, &map, &queue)) {
        free_prog(&fm, &map, &queue);
        return 3;
    }

    size_t result1 = part1(&map, &queue);

    printf("Part: 1: Beam splitting: %zu\n", result1);
    free_prog(&fm, &map, &queue);

    return 0;
}

static size_t part1(const map_t *map, queue_t *queue) {
    node_t start = {0};

    get_start_pos(map, &start);
    queue->visited[start.row][start.col] = true;
    queue->nodes[queue->tail] = start;
    ++queue->tail;

    size_t result = 0;
    while (queue->head != queue->tail) {
        node_t cur = queue->nodes[queue->head];
        ++queue->head;

        ssize_t r = cur.row;
        ssize_t c = cur.col;

        while ((size_t)r < map->row) {
            if (map->grid[r][c] == '^') {
                if (!queue->visited[r][c]) {
                    queue->visited[r][c] = true;
                    ++result;
                }

                if (c - 1 >= 0 && !queue->visited[r][c - 1]) {
                    node_t left = {.row = r, .col = c - 1};
                    queue->nodes[queue->tail] = left;
                    ++queue->tail;
                    queue->visited[r][c - 1] = true;
                }

                if ((size_t)(c + 1) < map->col && !queue->visited[r][c + 1]) {
                    node_t right = {.row = r, .col = c + 1};
                    queue->nodes[queue->tail] = right;
                    ++queue->tail;
                    queue->visited[r][c + 1] = true;
                }
                break;
            }
            ++r;
        }
    }

    return result;
}

static void get_size(const filemap_t *fm, map_t *map) {
    const char *buf = fm->buf;

    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return;
        }

        size_t col = (size_t)(c - buf);
        if (col >= map->col) {
            map->col = col;
        }
        ++map->row;

        buf = c + 1;
    }
}

static bool init(const filemap_t *fm, map_t *map, queue_t *queue) {
    queue->nodes = calloc(map->row * map->col * 2 + 1, sizeof(*queue->nodes));
    if (!queue->nodes) {
        return false;
    }

    map->grid = calloc(map->row + 1, sizeof(*map->grid));
    if (!map->grid) {
        goto failed;
    }

    queue->visited = calloc(map->row + 1, sizeof(*queue->visited));
    if (!queue->visited) {
        goto failed;
    }

    size_t index = 0;
    const char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        if (!c) {
            return true;
        }

        map->grid[index] = calloc(map->col + 1, sizeof(**map->grid));
        if (!map->grid[index]) {
            goto failed;
        }
        strncpy(map->grid[index], buf, map->col);

        queue->visited[index] = calloc(map->col + 1, sizeof(**queue->visited));
        if (!queue->visited[index]) {
            goto failed;
        }

        buf = c + 1;
        ++index;
    }

    return true;
failed:
    free(queue->nodes);
    queue->nodes = NULL;

    if (map->grid) {
        free_grid(map->grid);
        map->grid = NULL;
    }

    if (queue->visited) {
        free_visited(queue->visited);
        queue->visited = NULL;
    }

    return false;
}

static void get_start_pos(const map_t *map, node_t *node) {
    size_t row = 0;
    while (row < map->row) {
        size_t col = 0;
        while (col < map->col) {
            if (map->grid[row][col] == 'S') {
                node->row = (ssize_t)row + 1;
                node->col = (ssize_t)col;
                return;
            }
            ++col;
        }

        ++row;
    }
}

static void free_prog(filemap_t *fm, map_t *map, queue_t *queue) {
    if (fm) {
        close(fm->fd);
        munmap(fm->buf, fm->size);
    }

    free_grid(map->grid);
    free_queue(queue);
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

static void free_queue(queue_t *queue) {
    if (queue) {
        if (queue->visited) {
            free_visited(queue->visited);
        }

        if (queue->nodes) {
            free(queue->nodes);
        }
    }
}

static void free_visited(bool **visited) {
    size_t index = 0;
    while (visited[index]) {
        free(visited[index]);
        ++index;
    }

    free(visited);
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
