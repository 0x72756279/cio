#ifndef __CMD_HELP_H_
#define __CMD_HELP_H_

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define COMMAND_P_DSC "print response"
#define PRINT_DSC "print N bytes"
#define WRITE_DSC "write DATA into current offset"
#define SEEK_DSC "seek to offset"
#define PRINT_HEX_DSC "hexdump N bytes"


const char *help_msg_c[] = {
	"Usage:", "c[=ax] [cmd]", "",
	"ca  ", "[cmd]", COMMAND_P_DSC " as address",
	"cx  ", "[cmd]", COMMAND_P_DSC " as hex",
	NULL
};

const char *help_msg_px[] = {
	"Usage:", "px[=C] [len]", "",
	"pxC ", "[len]", PRINT_HEX_DSC " with colors",
	NULL
};

const char *help_msg_p[] = {
	"Usage:", "p[=aprx] [len]", "",
	"p   ", "[?] [len]", PRINT_DSC,
	"pa  ", "[len]", PRINT_DSC " as address",
	"pp  ", "", "print last output",
	"pr  ", "[reg]", "print register",
	"px  ", "[?] [len]", PRINT_HEX_DSC,
	NULL
};

const char *help_msg_w[] = {
	"Usage:", "w [dat]", "",
	"w   ", "[dat]", WRITE_DSC,
	NULL
};

const char *help_msg_s[] = {
	"Usage:", "s[=+-as] [off]", "",
	"s   ", "[?] [off]", SEEK_DSC,
	"s+  ", "[off]", "relative forward " SEEK_DSC,
	"s-  ", "[off]", "relative backward " SEEK_DSC,
	"sa  ", "[len]", "use N bytes as address to seek to",
	"sr  ", "[reg]", "seek to register",
	"ss  ", "", "seek to saved address",
	NULL
};

const char *help_msg_y[] = {
	"Usage:", "y[=r] [reg]", "",
	"y   ", "[?] [reg]", "yank output to register",
	"yr  ", "[reg] [reg]", "yank register into another register",
	NULL
};

const char *help_msg_global[] = {
	"Usage:", "[cmd]", "",
	"c   ", "[?] [cmd]", "ioctl command",
	"i   ", "[?] [cmd]", "show info",
	"l   ", "[?] [cmd]", "show logs",
	"p   ", "[?] [len]", PRINT_DSC,
	"s   ", "[?] [adr]", SEEK_DSC,
	"w   ", "[?] [dat]", WRITE_DSC,
	"y   ", "[?] [reg]", "yank to register",
	NULL
};


/**
 * @brief Parse and show help messages for commands.
 *
 * @param output_file print help to this file.
 * @param help command structure to show.
 */
void show_cmd_help(FILE* output_file, const char *help[]);

#endif /*__CMD_HELP_H_ */

