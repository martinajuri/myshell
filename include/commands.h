#ifndef COMMANDS_H
#define COMMANDS_H

/**
 * @brief Changes the current directory to `directory`.
 *
 * If `directory` is not present, it shows the current directory.
 * If the directory does not exist, an appropriate error should be printed.
 * This command should update the `PWD` and `OLDPWD` environment variables,
 * and support `cd -` to return to the last working directory.
 *
 * @param directory The directory to change to.
 * @return int 0 if the directory change was successful, -1 on error.
 */
int cd(const char* directory);

/**
 * @brief Clears the screen, providing a clear view for new tasks.
 */
void clr(void);

/**
 * @brief Displays `comment` on the screen followed by a new line.
 *
 * Supports environment variables using `$`.
 *
 * @param comment The comment or environment variable to display.
 * @return int 0 if the command was successful, -1 on error.
 */
int echo(const char* comment);

/**
 * @brief Closes the shell, allowing the user to exit gracefully.
 *
 * Resources should be properly freed.
 */
void quit(void);

/**
 * @brief Executes a command entered by the user.
 *
 * @param input The command entered by the user.
 */
void execute_command(char* input);

/**
 * @brief Signal handler for the shell.
 *
 * @param signo The signal number.
 */
void signal_handler(int signo);

/**
 * @brief Signal handler for the shell.
 *
 * @param signo The signal number.
 */
void sigchld_handler(int signo);

/**
 * @brief Sets up the signal handlers for the shell.
 */
void setup_signal_handlers(void);

#endif // COMMANDS_H
