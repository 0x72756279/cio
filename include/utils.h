#ifndef __STR_UTILS_H_
#define __STR_UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>


#define ERR(file, format, args...) \
    if (file) {fprintf(file, format ":%s\n", ## args, strerror(errno));}

#define LOG(file, format, args...) \
    if (file) {fprintf(file, format, ## args);}

#define MALLOC(x) \
    calloc(1, x)

#define FREE(x) \
    if (x) { \
        free(x); \
        x = 0; \
    }

/**
 * @brief Get the data as address value
 *
 * @param data the data to parse.
 * @param size the pointer size to use.
 * @return returns the address value.
 */
uint64_t get_data_as_address(char *data, size_t size);

/**
 * @brief Print data blob as hexdump.
 *
 * @param output_file print to this file.
 * @param data the data blob to print.
 * @param size the size of the data blob.
 * @param address a reference offset to start the count from in the hexdump table.
 * @param color_mode use colored output.
 */
void print_data_hex(FILE* output_file, char *data, size_t size,
                    uintptr_t address, uint8_t color_mode,
                    uint8_t relative_mode);

/**
 * @brief Print data as address.
 *
 * @param output_file print to this file.
 * @param data the data to translate.
 * @param size the size of pointer.
 */
void print_data_as_address(FILE* output_file, char *data, size_t size);

/**
 * @brief Write data string as hex values.
 *
 * @param fd target file to write to.
 * @param data string data to use as hex string.
 * @param length the length of the string data.
 */
void write_data_hex(int fd, char *data, size_t length);

/**
 * @brief Split string by delimiter.
 * This function allocates memory and it is the user responsibility to clean
 * it up.
 *
 * @param str the string to split.
 * @param delim the delimiter to use.
 * @param splitted pointer to return a new allocated array of strings. ends with a \a NULL.
 * @return the number of returned strings. (that needs to be freed)
 */
size_t split_str(const char *str, const char *delim, char ***splitted);

/**
 * @brief Check if a string has prefix.
 *
 * @param str the string to check.
 * @param needle the prefix to check.
 * @return Non zero value if \a str has the \a needle as prefix.
 */
int str_starts_with(const char *str, const char *needle);


/**
 * @brief Expand '~' to a real path.
 * returns a new allocated string that needs to be freed by the user.
 *
 * @param filepath the original received path.
 * @return a new expended string.
 */
char* expand_path(char const *filepath);

/**
 * @brief Get the filename from file descriptor.
 * This function may allocates memory for the file name that needs to
 * be freed by the user.
 *
 * @param fd the file descriptor to use.
 * @return a new allocated string with the file name (or NULL on error).
 */
char* get_filename(int fd);

#endif /* __STR_UTILS_H_ */

