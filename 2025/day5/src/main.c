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

typedef struct range_s {
    unsigned long min;
    unsigned long max;
} range_t;

static size_t count_elements(filemap_t *fm);
static void set_nbrs(filemap_t *fm, range_t *nbrs);
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

    size_t max = count_elements(&fm);
    range_t *nbrs = (range_t *)calloc(max + 1, sizeof(*nbrs));
    if (!nbrs) {
        fprintf(stderr, "Failed calloc\n");
        goto failed;
    }
    set_nbrs(&fm, nbrs);

    size_t p1_counter = 0;
    char *buf = fm.buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm.size - (size_t)(buf - fm.buf));
        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        char *dash = memchr(buf, '-', len);
        if (dash) {
            buf = c + 1;
            continue;
        }

        char *endptr;
        unsigned long nbr = strtoul(line, &endptr, 10);
        for (size_t index = 0; index < max; ++index) {
            if (nbr >= nbrs[index].min && nbr <= nbrs[index].max) {
                ++p1_counter;
                break;
            }
        }

        buf = c + 1;
    }

    printf("Part: 1: Available fresh ingredient IDs: %zu\n", p1_counter);
    close(fm.fd);
    free(nbrs);
    munmap(fm.buf, fm.size);

    return 0;

failed:

    close(fm.fd);
    munmap(fm.buf, fm.size);
    if (nbrs) {
        free(nbrs);
    }

    return 1;
}

static size_t count_elements(filemap_t *fm) {
    size_t counter = 0;

    char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        char *dash = memchr(buf, '-', len);
        if (!dash) {
            break;
        }

        ++counter;
        buf = c + 1;
    }

    return counter;
}

static void set_nbrs(filemap_t *fm, range_t *nbrs) {
    size_t index = 0;
    char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        char *dash = memchr(buf, '-', len);
        if (!dash) {
            break;
        }

        len = (size_t)(dash - buf);
        char *endptr;
        unsigned long min = strtoul(buf, &endptr, 10);
        unsigned long max = strtoul(dash + 1, &endptr, 10);
        range_t range = {.min = min, .max = max};

        nbrs[index] = range;
        ++index;
        buf = c + 1;
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
