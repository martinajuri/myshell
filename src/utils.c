#include "utils.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Flag to determine whether to show full path or not.
 */
int show_full_path = 1;

/**
 * @brief Offset to get the next character after the last slash.
 */
#define NEXT_CHAR 1

void toggle_path_view()
{
    show_full_path = !show_full_path;
}

void print_colored_prompt(const char* username, const char* hostname, const char* cwd)
{
    char* display_dir = (char*)cwd;
    if (!show_full_path)
    {
        char* last_slash = strrchr(cwd, '/');
        if (last_slash != NULL && *(last_slash + NEXT_CHAR) != '\0')
        {
            display_dir = last_slash + NEXT_CHAR;
        }
    }

    printf(COLOR_USER "%s" COLOR_RESET "@" COLOR_HOST "%s" COLOR_RESET ":" COLOR_DIR "%s" COLOR_RESET "$ ", username,
           hostname, display_dir);
}
