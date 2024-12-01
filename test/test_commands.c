#include "../include/commands.h"
#include "unity.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void setUp(void)
{
    // No setup needed for these tests
}

void tearDown(void)
{
    // No teardown needed for these tests
}

void test_cd(void)
{
    char original_path[PATH_MAX];
    if (getcwd(original_path, sizeof(original_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get current working directory");
    }

    // Change to a valid directory
    int result = cd("/tmp");
    TEST_ASSERT_EQUAL_INT(0, result);

    char new_path[PATH_MAX];
    if (getcwd(new_path, sizeof(new_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get new working directory");
    }
    TEST_ASSERT_EQUAL_STRING("/tmp", new_path);

    // Try to change to an invalid directory
    result = cd("/path/invalido");
    TEST_ASSERT_EQUAL_INT(-1, result);

    // Set PROJECT_ROOT to the current directory
    setenv("PROJECT_ROOT", original_path, 1);

    // Try to change to the parent directory
    result = cd("..");
    TEST_ASSERT_EQUAL_INT(0, result);

    if (getcwd(new_path, sizeof(new_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get new working directory");
    }
    TEST_ASSERT_FALSE(strcmp(original_path, new_path) == 0);

    // Restore the original path
    result = cd(original_path);
    TEST_ASSERT_EQUAL_INT(0, result);

    if (getcwd(new_path, sizeof(new_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get new working directory");
    }
    TEST_ASSERT_EQUAL_STRING(original_path, new_path);
}

void test_echo(void)
{
    // Save the original stdout file descriptor
    fflush(stdout);
    int saved_stdout = dup(fileno(stdout));
    if (saved_stdout == -1)
    {
        perror("dup");
        TEST_FAIL_MESSAGE("Failed to duplicate stdout");
    }

    // Redirect stdout to a file
    FILE* fp = freopen("test_output.txt", "w+", stdout);
    if (fp == NULL)
    {
        perror("freopen");
        TEST_FAIL_MESSAGE("Failed to redirect stdout");
    }

    // Call the echo function
    echo("Hola Mundo");

    // Flush and restore stdout
    fflush(stdout);
    if (dup2(saved_stdout, fileno(stdout)) == -1)
    {
        perror("dup2");
        TEST_FAIL_MESSAGE("Failed to restore stdout");
    }
    close(saved_stdout);

    // Read the output file
    fp = fopen("test_output.txt", "r");
    if (fp == NULL)
    {
        perror("fopen");
        TEST_FAIL_MESSAGE("Failed to open output file");
    }
    char output[50];
    if (fgets(output, sizeof(output), fp) == NULL)
    {
        perror("fgets");
        TEST_FAIL_MESSAGE("Failed to read output file");
    }
    TEST_ASSERT_EQUAL_STRING("Hola Mundo\n", output);
    fclose(fp);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cd);
    RUN_TEST(test_echo);
    return UNITY_END();
}
