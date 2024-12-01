#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pipe.h"

/**
 * @brief Size of the input buffer.
 */
#define INPUT_SIZE 1024

/**
 * @brief Maximum number of commands in a pipeline.
 */
#define MAX_COMMANDS 100

/**
 * @brief Maximum number of arguments for a command.
 */
#define MAX_ARGS (INPUT_SIZE / 2 + 1)

/**
 * @brief Number of file descriptors per pipe.
 */
#define PIPE_FDS_PER_PIPE 2

void execute_piped_commands(char* input)
{
    char* commands[MAX_ARGS];
    int num_commands = 0;
    char* token = strtok(input, "|");
    while (token != NULL)
    {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }
    commands[num_commands] = NULL;

    if (num_commands > MAX_COMMANDS)
    {
        fprintf(stderr, "Error: Too many commands\n");
        return;
    }

    size_t pipe_fds_size = PIPE_FDS_PER_PIPE * (num_commands - 1) * sizeof(int);
    if (pipe_fds_size / sizeof(int) != PIPE_FDS_PER_PIPE * (num_commands - 1))
    {
        fprintf(stderr, "Error: Size calculation overflow\n");
        return;
    }

    int* pipe_fds = malloc(pipe_fds_size);
    if (pipe_fds == NULL)
    {
        perror("malloc");
        return;
    }

    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipe_fds + i * PIPE_FDS_PER_PIPE) == -1)
        {
            perror("pipe");
            free(pipe_fds);
            return;
        }
    }

    for (int i = 0; i < num_commands; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            free(pipe_fds);
            return;
        }
        else if (pid == 0)
        {
            if (i > 0)
            {
                // Redirect the input to the pipe from the previous command
                if (dup2(pipe_fds[(i - 1) * PIPE_FDS_PER_PIPE], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < num_commands - 1)
            {
                // Redirect the output to the pipe to the next command
                if (dup2(pipe_fds[i * PIPE_FDS_PER_PIPE + 1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            // Close all the pipes in the child process
            for (int j = 0; j < PIPE_FDS_PER_PIPE * (num_commands - 1); j++)
            {
                close(pipe_fds[j]);
            }
            // Split the command into arguments
            char* args[MAX_ARGS];
            int k = 0;
            char* arg_token = strtok(commands[i], " ");
            while (arg_token != NULL)
            {
                args[k++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[k] = NULL;
            if (execvp(args[0], args) == -1)
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }
    // Close all the pipes in the parent process
    for (int i = 0; i < PIPE_FDS_PER_PIPE * (num_commands - 1); i++)
    {
        close(pipe_fds[i]);
    }
    // Wait for all the child processes to finish
    for (int i = 0; i < num_commands; i++)
    {
        int status;
        if (waitpid(-1, &status, 0) == -1)
        {
            perror("waitpid");
        }
    }
    free(pipe_fds);
}
