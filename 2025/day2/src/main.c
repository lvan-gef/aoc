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

static size_t parse_str(const char *str, size_t size, size_t part);
static size_t check_seq(size_t start, size_t end, size_t size, size_t part);
static bool is_repeated(const char *s, size_t len, size_t wanted);
static size_t count_repeats(const char *s);
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

    size_t id1 = 0;
    size_t id2 = 0;
    char *buf = fm.buf;
    while (*buf) {
        char *c = memchr(buf, ',', fm.size - (size_t)(buf - fm.buf));
        if (!c) {
            id1 += parse_str(buf, fm.size - (size_t)(buf - fm.buf), 1);
            id2 += parse_str(buf, fm.size - (size_t)(buf - fm.buf), 2);
            break;
        }

        size_t len = (size_t)(c - buf);
        char line[len + 1];
        memcpy(line, buf, len);
        line[len] = '\0';

        id1 += parse_str(line, len, 1);
        id2 += parse_str(line, len, 2);

        buf = c + 1;
    }

    printf("Part: 1: The ivalid IDs: %zu\n", id1);
    printf("Part: 2: The ivalid IDs: %zu\n", id2);
    close(fm.fd);
    munmap(fm.buf, fm.size);

    return 0;
}

static size_t parse_str(const char *str, size_t size, size_t part) {
    size_t sum = 0;
    const char *c = memchr(str, '-', size);
    size_t len = (size_t)(c - str);
    char line[len + 1];
    snprintf(line, len + 1, "%s", str);

    str += (len + 1);
    if (*line && *str) {
        char *endptr = NULL;
        size_t start = (size_t)strtol(line, &endptr, 10);
        size_t end = (size_t)strtol(str, &endptr, 10);
        sum += check_seq(start, end, strlen(str), part);
    }

    return sum;
}

static size_t check_seq(size_t start, size_t end, size_t size, size_t part) {
    size_t seq = 0;

    while (start <= end) {
        char tmp[size + 1];
        int len = snprintf(tmp, size + 1, "%zu", start);

        if (part == 1) {
            bool repeat = is_repeated(tmp, (size_t)len, 2);
            if (repeat) {
                seq += start;
            }
        } else {
            size_t counter = count_repeats(tmp);
            if (counter >= 2) {
                seq += start;
            }
        }

        ++start;
    }

    return seq;
}

static bool is_repeated(const char *s, size_t len, size_t wanted) {
    if (len % wanted != 0) {
        return false;
    }

    size_t block_len = len / wanted;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] != s[i % block_len]) {
            return false;
        }
    }

    return true;
}

static size_t count_repeats(const char *s) {
    size_t n = strlen(s);
    size_t block_len = 1;

    for (size_t i = 1; i <= n; ++i) {
        if (n % i == 0) {
            bool ok = true;
            for (size_t j = 0; j < n; ++j) {
                if (s[j] != s[j % i]) {
                    ok = false;
                    break;
                }
            }

            if (ok) {
                block_len = i;
                break;
            }
        }
    }
    return (n / block_len);
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
