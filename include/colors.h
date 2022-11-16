#ifndef __COLORS_H_
#define __COLORS_H_

#define Color_BLINK        "\x1b[5m"
#define Color_INVERT       "\x1b[7m"
#define Color_INVERT_RESET "\x1b[27m"
     /* See 'man 4 console_codes' for details:
      * "ESC c"        -- Reset
      * "ESC ( K"      -- Select user mapping
      * "ESC [ 0 m"    -- Reset all display attributes
      * "ESC [ J"      -- Erase to the end of screen
      * "ESC [ ? 25 h" -- Make cursor visible
      */
#define Color_RESET_TERMINAL  "\x1b" "c\x1b(K\x1b[0m\x1b[J\x1b[?25h"
#define Color_RESET      "\x1b[0m" /* reset all */
#define Color_RESET_NOBG "\x1b[27;22;24;25;28;39m"  /* Reset everything except background (order is important) */
#define Color_RESET_BG   "\x1b[49m"
#define Color_RESET_ALL  "\x1b[0m\x1b[49m"
#define Color_BLACK      "\x1b[30m"
#define Color_BGBLACK    "\x1b[40m"
#define Color_RED        "\x1b[31m"
#define Color_BGRED      "\x1b[41m"
#define Color_WHITE      "\x1b[37m"
#define Color_BGWHITE    "\x1b[47m"
#define Color_GREEN      "\x1b[32m"
#define Color_BGGREEN    "\x1b[42m"
#define Color_MAGENTA    "\x1b[35m"
#define Color_BGMAGENTA  "\x1b[45m"
#define Color_YELLOW     "\x1b[33m"
#define Color_BGYELLOW   "\x1b[43m"
#define Color_CYAN       "\x1b[36m"
#define Color_BGCYAN     "\x1b[46m"
#define Color_BLUE       "\x1b[34m"
#define Color_BGBLUE     "\x1b[44m"
#define Color_GRAY       "\x1b[90m"
#define Color_BGGRAY     "\x1b[100m"
/* bright colors */
#define Color_BBLACK     Color_GRAY
#define Color_BBGBLACK   Color_BGGRAY
#define Color_BRED       "\x1b[91m"
#define Color_BBGRED     "\x1b[101m"
#define Color_BWHITE     "\x1b[97m"
#define Color_BBGWHITE   "\x1b[107m"
#define Color_BGREEN     "\x1b[92m"
#define Color_BBGGREEN   "\x1b[102m"
#define Color_BMAGENTA   "\x1b[95m"
#define Color_BBGMAGENTA "\x1b[105m"
#define Color_BYELLOW    "\x1b[93m"
#define Color_BBGYELLOW  "\x1b[103m"
#define Color_BCYAN      "\x1b[96m"
#define Color_BBGCYAN    "\x1b[106m"
#define Color_BBLUE      "\x1b[94m"
#define Color_BBGBLUE    "\x1b[104m"
#endif /* __COLORS_H_ */