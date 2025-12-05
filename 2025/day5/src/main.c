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

static void part1(filemap_t *fm, const range_t *nbrs, size_t max);
static void part2(range_t *nbrs1, const size_t range, range_t *nbrs2);
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
    range_t *nbrs1 = (range_t *)calloc(max + 1, sizeof(*nbrs1));
    if (!nbrs1) {
        fprintf(stderr, "Failed calloc\n");
        close(fm.fd);
        munmap(fm.buf, fm.size);
    }

    range_t *nbrs2 = (range_t *)calloc(max + 1, sizeof(*nbrs1));
    if (!nbrs2) {
        fprintf(stderr, "Failed calloc\n");
        close(fm.fd);
        munmap(fm.buf, fm.size);
        free(nbrs1);
        return 3;
    }

    set_nbrs(&fm, nbrs1);
    part1(&fm, nbrs1, max);
    part2(nbrs1, max, nbrs2);

    close(fm.fd);
    munmap(fm.buf, fm.size);
    free(nbrs1);
    free(nbrs2);

    return 0;
}

static void part1(filemap_t *fm, const range_t *nbrs, size_t max) {
    size_t counter = 0;
    char *buf = fm->buf;
    while (*buf) {
        char *c = memchr(buf, '\n', fm->size - (size_t)(buf - fm->buf));
        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        const char *dash = memchr(buf, '-', len);
        if (dash) {
            buf = c + 1;
            continue;
        }

        char *endptr;
        unsigned long nbr = strtoul(line, &endptr, 10);
        for (size_t index = 0; index < max; ++index) {
            if (nbr >= nbrs[index].min && nbr <= nbrs[index].max) {
                ++counter;
                break;
            }
        }

        buf = c + 1;
    }

    printf("Part: 1: Available fresh ingredient IDs: %zu\n", counter);
}

static void part2(const range_t *nbrs1, size_t range, range_t *nbrs2) {
    size_t merged_count = 1;
    nbrs2[0] = nbrs1[0];

    for (size_t index = 1; index < range; ++index) {
        range_t new_range = nbrs1[index];
        int merged = 0;

        for (size_t sub_index = 0; sub_index < merged_count; ++sub_index) {
            if (new_range.min <= nbrs2[sub_index].max + 1 &&
                new_range.max + 1 >= nbrs2[sub_index].min) {
                if (new_range.min < nbrs2[sub_index].min) {
                    nbrs2[sub_index].min = new_range.min;
                }

                if (new_range.max > nbrs2[sub_index].max) {
                    nbrs2[sub_index].max = new_range.max;
                }
                merged = 1;

                for (size_t next_index = sub_index + 1;
                     next_index < merged_count; ++next_index) {
                    if (nbrs2[sub_index].min <= nbrs2[next_index].max + 1 &&
                        nbrs2[sub_index].max + 1 >= nbrs2[next_index].min) {
                        if (nbrs2[next_index].min < nbrs2[sub_index].min) {
                            nbrs2[sub_index].min = nbrs2[next_index].min;
                        }

                        if (nbrs2[next_index].max > nbrs2[sub_index].max) {
                            nbrs2[sub_index].max = nbrs2[next_index].max;
                        }

                        for (size_t m = next_index; m < merged_count - 1; m++) {
                            nbrs2[m] = nbrs2[m + 1];
                        }
                        merged_count--;
                        next_index--;
                    }
                }
                break;
            }
        }

        if (!merged) {
            nbrs2[merged_count] = new_range;
            merged_count++;
        }
    }

    size_t counter = 0;
    for (size_t i = 0; i < merged_count; ++i) {
        counter += (nbrs2[i].max - nbrs2[i].min + 1);
    }

    printf("Part: 2: Fresh ingredient IDs          : %zu\n", counter);
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

        const char *dash = memchr(buf, '-', len);
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

        const char *dash = memchr(buf, '-', len);
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
