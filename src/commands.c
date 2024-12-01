#include "commands.h"
#include "monitor.h"
#include "pipe.h"
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Size of the input buffer.
 */
#define INPUT_SIZE 1024

/**
 * @brief Actual job ID.
 */
static int job_id = 1;

/**
 * @brief PID of the foreground process.
 */
static pid_t foreground_pid = -1;

/**
 * @brief File permissions for output redirection.
 */
#define FILE_PERMISSIONS 0644

/**
 * @brief Offset to get the next character after the last slash.
 */
#define NEXT_CHAR 1

void signal_handler(int sig)
{
    if (foreground_pid > 0)
    {
        kill(foreground_pid, sig);
    }
}

void sigchld_handler(int sig)
{
    int saved_errno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        if (WIFSTOPPED(status))
        {
            printf("Proceso con PID %d detenido\n", pid);
            foreground_pid = -1;
        }
    }

    errno = saved_errno;
}

void setup_signal_handlers()
{
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    if (sigaction(SIGQUIT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
}

int cd(const char* directory)
{
    if (directory == NULL || strcmp(directory, "") == 0)
    {
        fprintf(stderr, "cd: missing argument\n");
        return -1;
    }

    char absolute_path[PATH_MAX];
    if (directory[0] != '/')
    {
        if (getcwd(absolute_path, sizeof(absolute_path)) == NULL)
        {
            perror("getcwd");
            return -1;
        }
        strncat(absolute_path, "/", sizeof(absolute_path) - strlen(absolute_path) - 1);
        strncat(absolute_path, directory, sizeof(absolute_path) - strlen(absolute_path) - 1);
        directory = absolute_path;
    }

    char* oldpwd = getenv("PWD");
    char cwd[PATH_MAX];

    // cd -
    if (strcmp(directory, "-") == 0)
    {
        directory = getenv("OLDPWD");
        if (directory == NULL)
        {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return -1;
        }
    }

    // change directory
    if (chdir(directory) != 0)
    {
        perror("cd");
        return -1;
    }

    // update OLDPWD
    if (oldpwd != NULL)
    {
        setenv("OLDPWD", oldpwd, 1);
    }

    // update PWD
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        setenv("PWD", cwd, 1);
    }
    else
    {
        perror("getcwd");
        return -1;
    }

    return 0;
}

void clr(void)
{
    printf("\033[H\033[J");
}

int echo(const char* comment)
{
    char* env;
    char* ptr = (char*)comment;
    char* start = NULL;
    char* end = NULL;
    char* buffer = NULL;
    size_t len = 0;

    while (*ptr != '\0')
    {
        if (*ptr == '$')
        {
            start = ptr + 1;
            end = start;

            while (*end != '\0' && *end != ' ')
            {
                end++;
            }

            len = end - start;
            buffer = (char*)malloc(len + 1);
            if (buffer == NULL)
            {
                perror("malloc");
                return -1;
            }

            strncpy(buffer, start, len);
            buffer[len] = '\0';

            env = getenv(buffer);
            if (env != NULL)
            {
                printf("%s", env);
            }
            else
            {
                fprintf(stderr, "echo: %s: variable not set\n", buffer);
            }

            free(buffer);
            ptr = end;
        }
        else
        {
            printf("%c", *ptr);
            ptr++;
        }
    }

    printf("\n");

    return 0;
}

void quit(void)
{
    exit(0);
}

void execute_command(char* input)
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, "Error: PROJECT_ROOT environment variable is not set.\n");
        return;
    }

    // Remove the newline character from the input
    input[strcspn(input, "\n")] = 0;

    // Handle the 'togglepath' command
    if (strcmp(input, "togglepath") == 0)
    {
        toggle_path_view();
        return;
    }

    // Handle the 'cd' command
    if (strncmp(input, "cd", 2) == 0)
    {
        char* directory = input + 3;
        cd(directory);
        return;
    }

    // Handle the 'clr' command
    if (strcmp(input, "clr") == 0)
    {
        clr();
        return;
    }

    // Handle the 'quit' command
    if (strcmp(input, "quit") == 0)
    {
        quit();
        return;
    }

    // Handle the 'start_monitor' command
    if (strcmp(input, "start_monitor") == 0)
    {
        start_monitor();
        return;
    }

    // Handle the 'stop_monitor' command
    if (strcmp(input, "stop_monitor") == 0)
    {
        stop_monitor();
        return;
    }

    // Handle the 'update_monitor' command
    if (strcmp(input, "update_monitor") == 0)
    {
        update_monitor();
        return;
    }

    // Handle the 'status_monitor' command
    if (strcmp(input, "status_monitor") == 0)
    {
        status_monitor();
        return;
    }

    // Handle the 'config_monitor' command
    if (strcmp(input, "config_monitor") == 0)
    {
        config_monitor();
        return;
    }

    // Check for pipes
    if (strchr(input, '|') != NULL)
    {
        execute_piped_commands(input);
        return;
    }

    // Check for redirections
    char* input_file = NULL;
    char* output_file = NULL;
    char* command_part = input;
    char* token;

    // Identify redirections and separate the command part
    while ((token = strpbrk(command_part, "<>")) != NULL)
    {
        if (*token == '<')
        {
            *token = '\0';
            input_file = strtok(token + NEXT_CHAR, " ");
        }
        else if (*token == '>')
        {
            *token = '\0';
            output_file = strtok(token + NEXT_CHAR, " ");
        }
    }

    // Split the command part into arguments
    char* args[INPUT_SIZE / 2 + 1];
    int i = 0;
    token = strtok(command_part, " ");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Check if the command should be executed in the background
    int background = 0;
    if (i > 0 && strcmp(args[i - 1], "&") == 0)
    {
        background = 1;
        args[i - 1] = NULL;
    }

    // Handle redirection
    int input_fd = -1;
    int output_fd = -1;
    if (input_file != NULL)
    {
        char absolute_input_file[PATH_MAX];
        snprintf(absolute_input_file, sizeof(absolute_input_file), "%s/%s", project_root, input_file);
        input_fd = open(absolute_input_file, O_RDONLY);
        if (input_fd == -1)
        {
            perror("open input file");
            return;
        }
    }
    if (output_file != NULL)
    {
        char absolute_output_file[PATH_MAX];
        snprintf(absolute_output_file, sizeof(absolute_output_file), "%s/%s", project_root, output_file);
        output_fd = open(absolute_output_file, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMISSIONS);
        if (output_fd == -1)
        {
            perror("open output file");
            return;
        }
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        // Redirect input and output if necessary
        if (input_fd != -1)
        {
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                perror("dup2 input");
                exit(EXIT_FAILURE);
            }
            close(input_fd);
        }
        if (output_fd != -1)
        {
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output");
                exit(EXIT_FAILURE);
            }
            close(output_fd);
        }

        // Handle the 'echo' command
        if (strncmp(args[0], "echo", 4) == 0)
        {
            // Concatenate the remaining arguments to form the comment
            char comment[INPUT_SIZE] = "";
            for (int j = 1; args[j] != NULL; j++)
            {
                strcat(comment, args[j]);
                if (args[j + 1] != NULL)
                {
                    strcat(comment, " ");
                }
            }
            echo(comment);
            exit(0);
        }

        // Handle external commands
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (background)
        {
            printf("[%d] %d\n", job_id++, pid);
        }
        else
        {
            foreground_pid = pid; // Update the PID of the foreground process
            int status;
            if (waitpid(pid, &status, WUNTRACED) == -1)
            {
                perror("waitpid");
            }
            if (WIFSTOPPED(status))
            {
                printf("Proceso con PID %d detenido\n", pid);
            }
            foreground_pid = -1; // Restart the PID of the foreground process
        }
    }
}
