#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commands.h"
#include "utils.h"

#ifndef HOST_NAME_MAX
/**
 * @brief Maximum size of the buffer for the hostname.
 */
#define HOST_NAME_MAX 256
#endif

/**
 * @brief Size of the input buffer.
 */
#define INPUT_SIZE 1024

/**
 * @brief Maximum number of lines to read in batch mode.
 */
#define MAX_LINES 100

/**
 * @brief Maximum length of a username.
 */
#define USERNAME_MAX 256

/**
 * @brief Main function of the program.
 *
 * This function initializes the program, retrieves the hostname and current working directory,
 * and processes input in batch mode.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Returns 0 on success, or 1 on failure.
 */
int main(int argc, char* argv[])
{
    char hostname[HOST_NAME_MAX];
    char cwd[PATH_MAX];
    char* input[MAX_LINES];
    char* username = getenv("USER");

    if (username == NULL)
    {
        perror("getenv");
        return 1;
    }

    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        perror("gethostname");
        return 1;
    }

    // Setup signal handlers
    setup_signal_handlers();

    if (argc == 2)
    {
        // Mode batch: read commands from a file
        FILE* file = fopen(argv[1], "r");
        if (file == NULL)
        {
            perror("fopen");
            return 1;
        }

        int line_count = 0;
        while (line_count < MAX_LINES && (input[line_count] = malloc(INPUT_SIZE)) != NULL &&
               fgets(input[line_count], INPUT_SIZE, file) != NULL)
        {
            input[line_count][strcspn(input[line_count], "\n")] = 0;
            line_count++;
        }
        fclose(file);

        for (int i = 0; i < line_count; i++)
        {
            execute_command(input[i]);
            free(input[i]);
        }
    }
    else
    {
        char single_input[INPUT_SIZE];
        while (1)
        {
            // Get the current working directory
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                continue;
            }

            print_colored_prompt(username, hostname, cwd);

            // Read user input
            if (fgets(single_input, sizeof(single_input), stdin) == NULL)
            {
                perror("fgets");
                continue;
            }
            execute_command(single_input);
        }
    }
    return 0;
}
