#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

#include "../deps/isocline/include/isocline.h"

#include "colors.h"
#include "cmd_help.h"
#include "utils.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define REG_SIZE 0x100

#define MAX(x,y) (((x)>(y))?(x):(y))


typedef struct {
    char *output;
    size_t size;
} output_t;


static char *progname;
static char *target_filepath;
static uintptr_t saved_address = 0;
static output_t last_output = {0};
static output_t saved_registers[REG_SIZE] = {{0}};

/**
 * @brief the file descriptor that is managed by this program.
 *
 */
static int target_fd;

/**
 * @brief the file to write data output to.
 *
 */
static FILE *output_file = NULL;

/**
 * @brief the file to write prompts and other stuff to.
 *
 */
static FILE *error_file = NULL;

/**
 * @brief the file to write logs to.
 *
 */
static FILE *log_file = NULL;

static uintptr_t cur_address = 0;

static const struct option long_options[] = {
	{"logfile", required_argument, 0, 'l'},
	{"help", no_argument, 0, 'h'},
	{NULL, 0, 0, 0}
};

static const char* options_descriptions[] = {
    "a file to write logs to",
    "show the help menu",
    NULL
};


void* alloc(size_t size) {
    void* p;
    p = malloc(size);
    if (!p) {
        ERR(log_file, "malloc (%lu)", size);
    }

    return p;
}

void flush_output(FILE *fp, output_t *output) {
    fwrite(output->output, output->size, 1, fp);
    fflush(fp);
}

void clear_output(output_t *output) {
    if (!output) {
        LOG(log_file, "clear_output got NULL\n");
        return;
    }

    /* clean last output */
    if (output->output) {
        free(output->output);
        output->output = NULL;
    }
    output->size = 0;
}

output_t dup_output(output_t *output) {
    output_t res = {0};
    if (!output) {
        LOG(log_file, "dup_output got NULL\n");
        return res;
    }

    res.output = malloc(output->size);
    if (!res.output) {
        ERR(log_file, "dup_output malloc(%lu)\n", output->size);
        return res;
    }

    memcpy(res.output, output->output, output->size);
    res.size = output->size;
    return res;
}

off64_t str_to_addr(char *str) {
    off64_t addr = 0;

    if (str) {
        addr = strtoull(str, NULL, 0);
    }

    return addr;
}


void do_seek(off64_t offset) {
    off64_t ret;

    ret = lseek64(target_fd, offset, SEEK_SET);
    LOG(log_file, "lseek64(fd=%d, offset=%ld, mode=%d)\n",
        target_fd, offset, SEEK_SET);

    if (ret != -1) {
        cur_address = (uintptr_t)offset;
    }
}

ssize_t do_read(char *out_buf, size_t count) {
    ssize_t n_bytes;

    n_bytes = read(target_fd, out_buf, count);
    LOG(log_file, "read(fd=%d, count=%lu)\n", target_fd, count);

    do_seek((off64_t)cur_address);

    return n_bytes;
}

void seek_cmd(char *cmd) {
    char *cmd_args, *buf, reg;
    unsigned long long l_address = 0;
    off64_t offset = 0;
    ssize_t nbytes;
    size_t command_length;

    command_length = strlen(cmd);
    if (command_length <= 1) {
        show_cmd_help(output_file, help_msg_s);
        return;
    }

    cmd_args = strchr(cmd, ' ');

    /* skip spaces */
    while (cmd_args && *cmd_args == ' ') {
        cmd_args++;
    }

    offset = str_to_addr(cmd_args);

    switch (cmd[1]) {
        case '+':
            offset = cur_address + offset;
            break;

        case '-':
            offset = cur_address - offset;
            break;

        case 'a':
            buf = alloc(l_address);
            if (!buf) return;
            nbytes = do_read(buf, l_address);
            if (nbytes) {
                offset = get_data_as_address(buf, nbytes);
            }
            free(buf);

        case 'r':
            reg = cmd_args[0];
            cmd_args = saved_registers[reg % REG_SIZE].output;
            offset = str_to_addr(cmd_args);
            break;

        case 's':
            offset = saved_address;
            break;

        case ' ':
            break;

        case '?':
        default:
            show_cmd_help(output_file, help_msg_s);
            return;
    }

    do_seek(offset);
}


void print_cmd(char *cmd) {
    char *cmd_args, *buf = NULL, *redirection, *path, c;
    char filepath[PATH_MAX]= {0};
    int append_mode;
    size_t count = 0;
    ssize_t n_bytes;
    FILE* fp = NULL, *stream;

    cmd_args = strchr(cmd, ' ');
    redirection = strstr(cmd, ">>");
    append_mode = redirection ? 1 : 0;

    if (!append_mode) {
        redirection = strchr(cmd, '>');
    }

    if (redirection) {
        strncpy(filepath, redirection + (append_mode ? 2 : 1), sizeof(filepath));
        path = expand_path(filepath);
        if (path) {
            fp = fopen(path, append_mode ? "a+" : "w");
            FREE(path);
        } else {
            ERR(log_file, "fopen (%s)", path);
        }
    }

    if (!fp) {
        fp = output_file;
    }

    /* print last command */
    if (cmd[1] && cmd[1] == 'p') {
        flush_output(fp, &last_output);
        goto print_cmd_cleanup;
    }

    /* print last command */
    if (cmd[1] && cmd[1] == 'r') {
        if (!cmd_args) {
            show_cmd_help(output_file, help_msg_p);
            goto print_cmd_cleanup;
        }

        c = cmd_args[1];

        flush_output(fp, &saved_registers[c % REG_SIZE]);
        goto print_cmd_cleanup;
    }

    /* first level help */
    if (cmd[1] && cmd[1] == '?') {
        show_cmd_help(output_file, help_msg_p);
        goto print_cmd_cleanup;
    }

    if (cmd_args) {
        cmd_args++;
        count = strtoul(cmd_args, NULL, 0);
    }

    buf = (char*)alloc(count);
    if (!buf) {
        goto print_cmd_cleanup;
    }

    n_bytes = do_read(buf, count);
    if (!n_bytes) {
        FREE(buf);
        goto print_cmd_cleanup;
    }

    clear_output(&last_output);

    stream = open_memstream(&last_output.output, &last_output.size);

    switch (cmd[1]) {
        case '?':
            /* print command help */
            show_cmd_help(output_file, help_msg_p);
            goto print_cmd_cleanup;

        case 'a':
            print_data_as_address(stream, buf, n_bytes);
            saved_address = get_data_as_address(buf, n_bytes);
            break;

        case 'x':
            if (cmd[2] && cmd[2] == '?') {
                show_cmd_help(output_file, help_msg_px);
                goto print_cmd_cleanup;
            }
            print_data_hex(stream, buf, n_bytes, cur_address,cmd[2] && cmd[2] == 'C');
            break;

        case ' ':
            fprintf(stream, buf, n_bytes);
            break;

    }
    fflush(stream);
    fclose(stream);

    flush_output(fp, &last_output);

    fprintf(error_file, "\n");
    fflush(error_file);

print_cmd_cleanup:
    /* if we're using another file - close it */
    if (fp != output_file) {
        fclose(fp);
    }

    FREE(buf);
}


void write_cmd(char *cmd) {
    char *cmd_data;

    cmd_data = strchr(cmd, ' ');
    if (!cmd_data) {
        show_cmd_help(output_file, help_msg_w);
        return;
    }
    cmd_data++;

    switch (cmd[1]) {
        case '?':
            /* print command help */
            show_cmd_help(output_file, help_msg_w);
            return;

        case 'x':
            write_data_hex(target_fd, cmd_data, strlen(cmd_data));
            break;

        case ' ':
            write(target_fd, cmd_data, strlen(cmd_data));
            break;
    }

    /* return file pointer */
    do_seek((off64_t)cur_address);
}

void yank_cmd(char *cmd) {
    char *args;
    char c;

    args = strchr(cmd, ' ');
    if (!args) {
        show_cmd_help(output_file, help_msg_y);
        return;
    }

    args++;

    switch(cmd[1]) {
        case 'r':
            break;

        case ' ':
            c = args[0];
            clear_output(&saved_registers[c % REG_SIZE]);
            saved_registers[c % REG_SIZE] = dup_output(&last_output);

            break;

        default:
            show_cmd_help(output_file, help_msg_y);
    }
}

void ioctrl_cmd(char *cmd) {
    char *arg;
    unsigned long request = 0;
    int response;
    FILE *stream;

    arg = strchr(cmd, ' ');
    if (!arg) {
        show_cmd_help(output_file, help_msg_c);
        return;
    }

    arg++;
    request = strtoul(arg, NULL, 0);

    response = ioctl(target_fd, request);
    LOG(log_file, "ioctl(fd=%d, request=%lu)\n", target_fd, request);

    if (-1 == response) {
        ERR(log_file, "ioctl (%lu)", request);
        return;
    }

    clear_output(&last_output);

    stream = open_memstream(&last_output.output, &last_output.size);
    if (!stream) {
        ERR(log_file, "open_memstream\n");
        return;
    }

    switch(cmd[1]) {
        case 'a':
            fprintf(stream, "0x%.*X\n", (int)(sizeof(void *) * 2), response);
            saved_address = response;
            break;

        case 'x':
            fprintf(stream, "%x\n", response);
            break;

        case ' ':
        default:
            fprintf(stream, "%d\n", response);
            break;
    }

    fflush(stream);
    fclose(stream);

    flush_output(output_file, &last_output);

    fprintf(error_file, "\n");
    fflush(error_file);

}

void show_info_cmd(char *cmd) {
    struct stat target_stat;
    char *log_path = NULL;
    int log_fd;

    fstat(target_fd, &target_stat);

    if (log_file) {
        log_fd = fileno(log_file);
        log_path = get_filename(log_fd);
    }

    fprintf(output_file, "target path:      %s\n"
                         "target size:      %ld\n"
                         "logfile:          %s\n",
            target_filepath,
            target_stat.st_size,
            log_path ? log_path : "None"

            );

    if (log_path) {
        free(log_path);
    }
}

void show_logs_cmd(char *cmd) {
    char c;

    if (!log_file) return;

    log_file = freopen(NULL, "r", log_file);
    while (1) {
        c = fgetc(log_file);
        if (feof(log_file)) break;

        fprintf(output_file, "%c", c);
    }
    fprintf(output_file, "\n");

    log_file = freopen(NULL, "a", log_file);
}

void usage() {
    size_t i, name_max_len = 0, l, padding;
    const size_t size = (sizeof(long_options) / sizeof(struct option)) - 1;

    fprintf(error_file, "usage: %s <filepath>\n", progname);

    fprintf(error_file, "OPTIONS:\n");

    /* first calculate lengths */
    for (i = 0; i < size; i++) {
        if (!long_options[i].name) continue;

        l = strlen(long_options[i].name);
        if (l > name_max_len) {
            name_max_len = l;
        }
    }

    name_max_len += 2;

    for (i = 0; i < size; i++) {
        l = strlen(long_options[i].name);
        padding = MAX((name_max_len - l), 0);

        fprintf(error_file, "  -%c, --%s%*s %s\n",
                long_options[i].val,
                long_options[i].name,
                (int)padding, "",
                options_descriptions[i]);
    }

}

size_t print_prompt(char **buf) {
    const size_t ptr_size = sizeof(void*) * 2;
    char prompt[200];
    int l;

    if (!buf) return 0;

    l = sprintf(prompt, Color_YELLOW "\\[0x%.*" PRIxPTR "] " Color_RESET,
                (int)ptr_size, cur_address);
    prompt[l] = 0;

    *buf = ic_readline(prompt);

    if (!*buf) {
        ERR(log_file, "readline");

        /* create valid pointer that can be freed */
        *buf = alloc(0);

        return 0;
    }

    l = strlen(*buf);

    if (!l) return 0;

    ic_history_add(*buf);

    return l;
}


int execute_command(char *cmd) {
    if (!cmd) return 0;

    switch (*cmd)
    {
    case 'i':
        show_info_cmd(cmd);
        break;

    case 'l':
        show_logs_cmd(cmd);
        break;

    case 'c':
        ioctrl_cmd(cmd);
        break;

    case 'h':
    case '?':
        show_cmd_help(output_file, help_msg_global);
        break;

    case 's':
        seek_cmd(cmd);
        break;

    case 'q':
        return 1;

    case 'r':
    case 'p':
        print_cmd(cmd);
        break;

    case 'w':
        write_cmd(cmd);
        break;

    case 'y':
        yank_cmd(cmd);
        break;

    default:
        show_cmd_help(output_file, help_msg_global);
        break;
    }

    return 0;
}

void main_loop() {
    char *cmd;
    size_t length;
    int done;
    while (1) {
        /* use custom prompt */
        length = print_prompt(&cmd);

        if (!length) continue;

        done = execute_command(cmd);
        free(cmd);

        if (done){
            break;
        }
    }

}

int main(int argc, char **argv) {
    int c, index = 0;
    char *logfile_path = NULL;

    /* save the program name */
    progname = *argv;


    output_file = stdout;
    error_file = stderr;

    if (argc <= 1) {
        usage();
        exit(1);
    }

    target_filepath = argv[1];

    while ((c = getopt_long(argc, argv, ":hl:", long_options, &index)) != -1) {
        switch(c) {
            case 'l':
            logfile_path = optarg;
            break;
            case 'h':
            default:
                usage();
                return -1;
            break;
        }
    }

    target_fd = open(target_filepath, O_RDWR);

    /* set logfile */
    if (logfile_path) {
        log_file = fopen(logfile_path, "w");
    }

    if (target_fd <= 2) {
        ERR(log_file, "open (%s)", target_filepath);
        exit(1);
    }

    // enable history; use a NULL filename to not persist history to disk
    ic_set_history("history.txt", -1 /* default entries (= 200) */);

    main_loop();

    if (log_file) {
        fclose(log_file);
    }

    close(target_fd);
}

