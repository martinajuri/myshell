#include "../include/monitor.h"
#include "unity.h"
#include <linux/limits.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

void setUp(void)
{
    // Buscar la ruta del archivo CMakeLists.txt
    char current_path[PATH_MAX];
    if (getcwd(current_path, sizeof(current_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get current working directory");
    }

    char cmake_path[PATH_MAX + 16]; // Aumentar el tamaño del buffer para evitar truncación
    snprintf(cmake_path, sizeof(cmake_path), "%s/CMakeLists.txt", current_path);

    while (access(cmake_path, F_OK) != 0)
    {
        // Subir un nivel en el directorio
        char *last_slash = strrchr(current_path, '/');
        if (last_slash == NULL)
        {
            TEST_FAIL_MESSAGE("CMakeLists.txt not found in any parent directory");
        }
        *last_slash = '\0';
        snprintf(cmake_path, sizeof(cmake_path), "%s/CMakeLists.txt", current_path);
    }

    setenv("PROJECT_ROOT", current_path, 1);
}

void tearDown(void)
{
    // Clean up any created files
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root != NULL)
    {
        char pid_file_path[PATH_MAX];
        snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);
        remove(pid_file_path);
    }
}

int is_process_running(pid_t pid)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ps -p %d > /dev/null 2>&1", pid);
    int result = system(cmd);
    return result == 0;
}

void test_start_monitor(void)
{
    const char* project_root = getenv("PROJECT_ROOT");
    TEST_ASSERT_NOT_NULL_MESSAGE(project_root, "PROJECT_ROOT environment variable is not set");

    printf("Starting monitor...\n");
    fflush(stdout);

    start_monitor();

    char pid_file_path[PATH_MAX];
    snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

    FILE* pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL)
    {
        perror("fopen");
        TEST_FAIL_MESSAGE("Failed to open pid file");
    }

    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1)
    {
        perror("fscanf");
        TEST_FAIL_MESSAGE("Failed to read pid from pid file");
    }
    fclose(pid_file);

    printf("Monitor started with PID: %d\n", pid);
    fflush(stdout);

    // Check if the process is running
    if (!is_process_running(pid))
    {
        perror("ps");
        TEST_FAIL_MESSAGE("Monitor process is not running");
    }

    // Stop the monitor to clean up
    printf("Stopping monitor...\n");
    fflush(stdout);
    stop_monitor();
}

void test_status_monitor(void)
{
    const char* project_root = getenv("PROJECT_ROOT");
    TEST_ASSERT_NOT_NULL_MESSAGE(project_root, "PROJECT_ROOT environment variable is not set");

    printf("Starting monitor...\n");
    fflush(stdout);

    start_monitor();

    char pid_file_path[PATH_MAX];
    snprintf(pid_file_path, sizeof(pid_file_path), "%s/monitor.pid", project_root);

    FILE* pid_file = fopen(pid_file_path, "r");
    if (pid_file == NULL)
    {
        perror("fopen");
        TEST_FAIL_MESSAGE("Failed to open pid file");
    }

    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1)
    {
        perror("fscanf");
        TEST_FAIL_MESSAGE("Failed to read pid from pid file");
    }
    fclose(pid_file);

    printf("Monitor started with PID: %d\n", pid);
    fflush(stdout);

    // Check if the process is running
    if (!is_process_running(pid))
    {
        perror("ps");
        TEST_FAIL_MESSAGE("Monitor process is not running");
    }

    // Capture the output of status_monitor
    fflush(stdout);
    int saved_stdout = dup(fileno(stdout));
    FILE* output_file = freopen("test_output.txt", "w+", stdout);
    if (output_file == NULL)
    {
        perror("freopen");
        TEST_FAIL_MESSAGE("Failed to redirect stdout");
    }

    status_monitor();

    fflush(stdout);
    dup2(saved_stdout, fileno(stdout));
    close(saved_stdout);

    // Read the output file
    output_file = fopen("test_output.txt", "r");
    if (output_file == NULL)
    {
        perror("fopen");
        TEST_FAIL_MESSAGE("Failed to open output file");
    }
    char output[256];
    if (fgets(output, sizeof(output), output_file) == NULL)
    {
        perror("fgets");
        TEST_FAIL_MESSAGE("Failed to read output file");
    }
    fclose(output_file);

    // Remove ANSI color codes from the output
    char* ansi_removed_output = output;
    char* output_ptr = output;
    while (*ansi_removed_output)
    {
        if (*ansi_removed_output == '\x1B')
        {
            while (*ansi_removed_output && *ansi_removed_output != 'm')
            {
                ansi_removed_output++;
            }
            if (*ansi_removed_output)
            {
                ansi_removed_output++;
            }
        }
        else
        {
            *output_ptr++ = *ansi_removed_output++;
        }
    }
    *output_ptr = '\0';

    char expected_output[256];
    snprintf(expected_output, sizeof(expected_output), "Monitor is running with PID %d\n", pid);
    TEST_ASSERT_EQUAL_STRING(expected_output, output);

    // Stop the monitor
    printf("Stopping monitor...\n");
    fflush(stdout);
    stop_monitor();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_start_monitor);
    RUN_TEST(test_status_monitor);
    return UNITY_END();
}
