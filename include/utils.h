#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Reset color to default.
 */
#define COLOR_RESET "\033[0m"

/**
 * @brief Color for the username (backround pink).
 */
#define COLOR_USER "\033[1;95;45m"

/**
 * @brief Color for the hostname (pink).
 */
#define COLOR_HOST "\033[1;95m"

/**
 * @brief Color for the directory (light blue).
 */
#define COLOR_DIR "\033[1;36m"

/**
 * @brief Global variable to control the path view mode.
 *
 * 1 for full path, 0 for only the current directory.
 */
extern int show_full_path;

/**
 * @brief Toggle the path view mode between full path and current directory.
 */
void toggle_path_view();

/**
 * @brief Print the command prompt with colors.
 *
 * @param username The username to display.
 * @param hostname The hostname to display.
 * @param cwd The current working directory to display.
 */
void print_colored_prompt(const char* username, const char* hostname, const char* cwd);

#endif // UTILS_H
