#include "monitor.h"
#include <cjson/cJSON.h>
#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief ANSI escape code for red color.
 */
#define ANSI_COLOR_RED "\x1b[31m"

/**
 * @brief ANSI escape code for green color.
 */
#define ANSI_COLOR_GREEN "\x1b[32m"

/**
 * @brief ANSI escape code to reset color.
 */
#define ANSI_COLOR_RESET "\x1b[0m"

/**
 * @brief ANSI escape code for blue color.
 */
#define ANSI_COLOR_BLUE "\x1b[34m"

/**
 * @brief Size of the input buffer.
 */
#define INPUT_SIZE 10

/**
 * @brief Default sleep time in seconds.
 */
#define DEFAULT_SLEEP_TIME 1

void start_monitor()
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: PROJECT_ROOT environment variable is not set.\n" ANSI_COLOR_RESET);
        return;
    }

    char metrics_path[PATH_MAX];
    snprintf(metrics_path, sizeof(metrics_path), "%s/monitor/metrics", project_root);

    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror(ANSI_COLOR_RED "fork" ANSI_COLOR_RESET);
        return;
    }
    else if (pid == 0)
    {
        execl(metrics_path, metrics_path, config_path, (char*)NULL);
        perror(ANSI_COLOR_RED "execl" ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    else
    {
        char pid_file_path[PATH_MAX];
        snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

        FILE* pid_file = fopen(pid_file_path, "w");
        if (pid_file == NULL)
        {
            perror(ANSI_COLOR_RED "fopen" ANSI_COLOR_RESET);
            return;
        }
        fprintf(pid_file, "%d\n", pid);
        fclose(pid_file);
        printf(ANSI_COLOR_GREEN "Monitor started with PID %d\n" ANSI_COLOR_RESET, pid);
    }
}

void stop_monitor()
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: PROJECT_ROOT environment variable is not set.\n" ANSI_COLOR_RESET);
        return;
    }

    char pid_file_path[PATH_MAX];
    snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

    FILE* pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL)
    {
        perror(ANSI_COLOR_RED "fopen" ANSI_COLOR_RESET);
        return;
    }
    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1)
    {
        perror(ANSI_COLOR_RED "fscanf" ANSI_COLOR_RESET);
        fclose(pid_file);
        return;
    }
    fclose(pid_file);
    if (kill(pid, SIGINT) == -1)
    {
        perror(ANSI_COLOR_RED "kill" ANSI_COLOR_RESET);
    }
    else
    {
        printf(ANSI_COLOR_GREEN "Monitor stopped\n" ANSI_COLOR_RESET);
    }
}

void update_monitor()
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: PROJECT_ROOT environment variable is not set.\n" ANSI_COLOR_RESET);
        return;
    }

    char pid_file_path[PATH_MAX];
    snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

    FILE* pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL)
    {
        perror(ANSI_COLOR_RED "fopen" ANSI_COLOR_RESET);
        return;
    }
    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1)
    {
        perror(ANSI_COLOR_RED "fscanf" ANSI_COLOR_RESET);
        fclose(pid_file);
        return;
    }
    fclose(pid_file);
    if (kill(pid, SIGHUP) == -1)
    {
        perror(ANSI_COLOR_RED "kill" ANSI_COLOR_RESET);
    }
    else
    {
        printf(ANSI_COLOR_GREEN "Monitor updated\n" ANSI_COLOR_RESET);
    }
}

void status_monitor()
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: PROJECT_ROOT environment variable is not set.\n" ANSI_COLOR_RESET);
        return;
    }

    char pid_file_path[PATH_MAX];
    snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

    FILE* pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL)
    {
        printf(ANSI_COLOR_RED "Monitor is not running\n" ANSI_COLOR_RESET);
        return;
    }
    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1)
    {
        perror(ANSI_COLOR_RED "fscanf" ANSI_COLOR_RESET);
        fclose(pid_file);
        return;
    }
    fclose(pid_file);
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            printf(ANSI_COLOR_RED "Monitor is not running\n" ANSI_COLOR_RESET);
        }
        else
        {
            perror(ANSI_COLOR_RED "kill" ANSI_COLOR_RESET);
        }
    }
    else
    {
        printf(ANSI_COLOR_GREEN "Monitor is running with PID %d\n" ANSI_COLOR_RESET, pid);
    }
}

void config_monitor()
{
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: PROJECT_ROOT environment variable is not set.\n" ANSI_COLOR_RESET);
        return;
    }

    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

    char input[INPUT_SIZE];
    int sleep_time;
    cJSON* config = cJSON_CreateObject();
    cJSON* metrics = cJSON_CreateObject();

    printf("Enter 't' or 'f' for the following metrics:\n");

    printf("CPU: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting CPU to true by default.\n" ANSI_COLOR_RESET);
        cJSON_AddBoolToObject(metrics, "cpu", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "cpu", input[0] == 't');
    }

    printf("Memory: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting Memory to true by default.\n" ANSI_COLOR_RESET);
        cJSON_AddBoolToObject(metrics, "memory", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "memory", input[0] == 't');
    }

    printf("Disk IO: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting Disk IO to true by default.\n" ANSI_COLOR_RESET);
        cJSON_AddBoolToObject(metrics, "disk_io", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "disk_io", input[0] == 't');
    }

    printf("Process Count: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting Process Count to true by default.\n" ANSI_COLOR_RESET);
        cJSON_AddBoolToObject(metrics, "process_count", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "process_count", input[0] == 't');
    }

    printf("Context Switches: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting Context Switches to true by default.\n" ANSI_COLOR_RESET);
        cJSON_AddBoolToObject(metrics, "context_switches", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "context_switches", input[0] == 't');
    }

    cJSON_AddItemToObject(config, "metrics", metrics);

    printf("Enter sleep time (in seconds): ");
    if (fgets(input, sizeof(input), stdin) == NULL || (sleep_time = atoi(input)) <= 0)
    {
        printf(ANSI_COLOR_RED "Invalid input. Setting sleep time to %d second by default.\n" ANSI_COLOR_RESET,
               DEFAULT_SLEEP_TIME);
        sleep_time = DEFAULT_SLEEP_TIME;
    }
    cJSON_AddNumberToObject(config, "sleep_time", sleep_time);

    char* config_string = cJSON_Print(config);
    FILE* file = fopen(config_path, "w");
    if (file == NULL)
    {
        perror(ANSI_COLOR_RED "fopen" ANSI_COLOR_RESET);
        cJSON_Delete(config);
        free(config_string);
        return;
    }
    fprintf(file, "%s", config_string);
    fclose(file);

    cJSON_Delete(config);
    free(config_string);

    printf(ANSI_COLOR_GREEN "Configuration updated successfully.\n" ANSI_COLOR_RESET);
    printf("Please update the monitor process to apply the changes with the update_monitor command.\n");
}
