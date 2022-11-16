#define _LARGEFILE64_SOURCE
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "colors.h"


#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define OFFSET_STR "OFFSET"

void print_data_hex(FILE* output_file, char *data, size_t size,
                    uintptr_t address, int color_mode) {
    const size_t offset_size = sizeof(OFFSET_STR) - 1;
    const size_t ptr_size = sizeof(void*) * 2;
    const size_t offset_margin = ptr_size - offset_size + 2;

    char ascii[17];
    unsigned char cur;
    size_t i, j, padding_left, padding_right;
    
    ascii[16] = '\0';

    /* header */
    padding_left = offset_margin / 2;
    padding_right = offset_margin - padding_left;

    if (color_mode) {
        fprintf(output_file, Color_BLUE);
    }
    fprintf(output_file, "%*s%s%*s",
            (int)padding_left, "",
            OFFSET_STR,
            (int)padding_right, "");

    /* margin from relative offset column */
    fprintf(output_file, "%*s", (int)(ptr_size + 2 + 4), "");

    for (i = 0; i < 16; i++) {
        fprintf(output_file, " %02" PRIxPTR, i);

        if ((i+1) % 8 ==0) {
            fprintf(output_file, " ");
        }
    }
    if (color_mode) {
        fprintf(output_file, Color_RESET);
    }

    fprintf(output_file, "\n");

    for (i = 0; i < size; ++i) {
        cur = ((unsigned char *)data)[i];
        if (i % 16 == 0) {
            /* print offset column */

            if (color_mode) {
                fprintf(output_file, Color_CYAN);
            }
            fprintf (output_file, "0x%.*" PRIxPTR " ", (int)ptr_size, i + address);
            fprintf (output_file, "(+0x%.*" PRIxPTR ") ", (int)ptr_size, i);

            if (color_mode) {
                fprintf(output_file, Color_RESET);
            }
        }

        if (color_mode && 0 == cur) {
            fprintf(output_file, Color_GRAY);
        }

        fprintf(output_file, "%02X ", cur);

        if (color_mode && 0 == cur) {
            fprintf(output_file, Color_RESET);
        }
        
        /* printable */
        if (cur >= ' ' && cur <= '~') {
            ascii[i % 16] = cur;
        }
        else {
            ascii[i % 16] = '.';
        }

        if ((i + 1) % 8 == 0 || i + 1 == size) {
            fprintf(output_file, " ");
            if ((i + 1) % 16 == 0) {
                fprintf(output_file, "|  %s \n", ascii);
            }
            else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    fprintf(output_file, " ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    fprintf(output_file, "   ");
                }
                fprintf(output_file, "|  %s \n", ascii);
            }
        }
    }
}

uint64_t get_data_as_address(char *data, size_t size) {
    union 
    {
        char data[9];
        uint64_t value;
    } val;
    
    if (size <= 0 || size > 8) return 0;

    memcpy(val.data, data, size);
    return val.value;
}

void print_data_as_address(FILE* output_file, char *data, size_t size) {
    uint64_t value;

    value = get_data_as_address(data, size);

    if (size <= 4) {
        fprintf(output_file, "0x%x\n", (uint32_t)value);
    } else {
        fprintf(output_file, "0x%lx\n", value);
    }

}

void write_data_hex(int fd, char *data, size_t length) {
    size_t i, max_len, data_ok;
    char c[3];
    char *buf;
    unsigned long v;

    if (!length) return;

    /* two hex values = 1 byte */
    max_len = length / 2;

    buf = malloc(max_len);

    data_ok = 1;
    c[2] = 0;

    for (i = 0; i < max_len; i++) {
        memcpy(c, data + (i *2), 2);
        v = strtoul(c, NULL, 16);

        /* error */
        if (ULONG_MAX == v) {
            data_ok = 0;
            break;
        }

        buf[i] = v;
    }

    if (data_ok) {
        write(fd, buf, max_len);
    }

    free(buf);
}

size_t split_str(const char *str, const char *delim, char ***splitted) {
    size_t count, i;
    char *c;
    char *copy;

    if (!splitted) return 0;
    
    if (!str) return 0;

    copy = strdup(str);
    if (!copy) return 0;

    count = 0;
    
    c = copy;
    while (1) {
        c = strstr(c, delim);
        if (!c) break;

        count++;
        c++;
    }

    /* every list will have NULL pointer at the end */
    *splitted = (char**)calloc(sizeof(char*), count +2);
    if (!*splitted) return 0;

    if (count <= 0) {
        **splitted = copy;
        return 2;    
    }

    i = 0;

    /* there is only one thread */
    c = strtok(copy, delim);
    while(c) {
        (*splitted)[i++] = strdup(c);
        c = strtok(NULL, delim);
    }

    free(copy);

    return count + 2;
}

int str_starts_with(const char *str, const char *needle) {
	if (!str || !needle) {
		return 0;
	}
	if (str == needle) {
		return 1;
	}
	return !strncmp (str, needle, strlen(needle));
}

char* expand_path(char const *filepath) {
    char *str, *tmp;
    char path[PATH_MAX] = {0};
    size_t str_length, l, remain_space, i;

    if (!filepath) return NULL;

    /* remove prefix spaces */
    for (str = (char*)filepath; *str && isspace(*str); str++);

    str_length = strlen(str);
    if (!str_length) return NULL;

    remain_space = sizeof(path) -1;

    if (str_starts_with(str, "~/")) {
        /* expand home */        
        tmp = getenv("HOME");
        if (!tmp) return NULL;

        /* one more for '/' at the end */
        l = strlen(tmp) + 1;

        if (l > remain_space) {
            /* not enough space to copy */
            return NULL;
        }

        strcat(path, tmp);
        strcat(path, "/");
        remain_space -= l;

        str += 2;
        str_length -= 2;
    }

    if (str_length > remain_space) {
        /* not enough space to copy */
        return NULL;
    }

    strcat(path, str);

    /* trim spaces at the end */
    l = strlen(path);
    for (i = l - 1; i >=0; i--) {
        if (isspace(path[i])) {
            path[i] = 0;
        } else {
            break;
        }
    }

    return strdup(path);
}

char* get_filename(int fd) {
    char path[PATH_MAX], buf[PATH_MAX];

    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

    if (-1 == readlink(path, buf, sizeof(buf))) {
        return NULL;
    }

    return strdup(buf);
}

