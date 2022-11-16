#include <stdio.h>
#include "utils.h"

#define MAX(x,y) (((x)>(y))?(x):(y))


void show_cmd_help(FILE* output_file, const char *help[])
{
    size_t i, max_length = 0, len_cmd, len_args, str_length, padding;
    const char *usage_str = "Usage:";
    const char *help_cmd = NULL, *help_args = NULL, *help_desc = NULL;

    /* calculate padding for description text in advance */
    for (i = 0; help[i]; i += 3)
    {
        help_cmd = help[i + 0];
        help_args = help[i + 1];

        len_cmd = strlen(help_cmd);
        len_args = strlen(help_args);
        if (i)
        {
            max_length = MAX(max_length, len_cmd + len_args);
        }
    }

    for (i = 0; help[i]; i += 3)
    {
        help_cmd = help[i + 0];
        help_args = help[i + 1];
        help_desc = help[i + 2];

        if (str_starts_with(help_cmd, usage_str)) {
            /* Usage header */
            fprintf(output_file, "%s", help_cmd);
            if (help_args[0]) {
                fprintf(output_file, "   %s", help_args);
            }
            if (help_desc[0]) {
                fprintf(output_file, "  %s", help_desc);
            }
            fprintf(output_file, "\n");
        }
        else if (!help_args[0] && !help_desc[0])
        {
            /* Section header, no need to indent it */
            fprintf(output_file, "%s\n", help_cmd);
        }
        else
        {
            /* Body of help text, indented */
            str_length = strlen(help_cmd) + strlen(help_args);
            padding = MAX((max_length - str_length), 0);
            fprintf(output_file, "| %s%s%*s  %s\n",
                    help_cmd,
                    help_args,
                    (int)padding, "",
                    help_desc);
        }
    }
}