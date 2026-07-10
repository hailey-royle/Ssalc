#ifndef TUI
#define TUI

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "assert.h"

#define backspace_key 8
#define tab_key 9
#define newline_key 10
#define linefeed_key 13
#define escape_key 27
#define delete_key 127

#define ansi_cursor_home "\x1b[H"
#define ansi_erase_line "\x1b[2K"

#define ansi_cursor_show "\x1b[?25h"
#define ansi_cursor_hidden "\x1b[?25l"
#define ansi_start_alt_screen "\x1b[?1049h"
#define ansi_end_alt_screen "\x1b[?1049l"

#define ansi_reset_graphics "\x1b[0m"
#define ansi_bold_start "\x1b[1m"
#define ansi_bold_end "\x1b[22m"
#define ansi_inverse_start "\x1b[7m"
#define ansi_inverse_end "\x1b[27m"

#define ansi_backrgound_black "\x1b[40m"
#define ansi_foreground_black "\x1b[30m"
#define ansi_foreground_red "\x1b[31m"
#define ansi_backrgound_red "\x1b[41m"
#define ansi_foreground_green "\x1b[32m"
#define ansi_backrgound_green "\x1b[42m"
#define ansi_foreground_yellow "\x1b[33m"
#define ansi_backrgound_yellow "\x1b[43m"
#define ansi_foreground_blue "\x1b[34m"
#define ansi_backrgound_blue "\x1b[44m"
#define ansi_foreground_magenta "\x1b[35m"
#define ansi_backrgound_magenta "\x1b[45m"
#define ansi_foreground_cyan "\x1b[36m"
#define ansi_backrgound_cyan "\x1b[46m"
#define ansi_foreground_white "\x1b[37m"
#define ansi_backrgound_white "\x1b[47m"
#define ansi_foreground_default "\x1b[39m"
#define ansi_backrgound_default "\x1b[49m"

#endif
