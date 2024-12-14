#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include "config_search.h"

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"

const char *config_extensions[] = {".config", ".conf", ".json", ".xml", ".ini"};
const char *config_keywords[] = {"config", "settings", "configuration", "setup", "file"};
const char *important_keywords[] = {"key", "password", "token", "secret", "user", "host", "port", "metrics"};

void read_config_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    printf("\n");
    printf(COLOR_GREEN "Contents of %s (important configurations only):\n" COLOR_RESET, file_path);
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        for (size_t i = 0; i < sizeof(important_keywords) / sizeof(important_keywords[0]); i++) {
            if (strstr(line, important_keywords[i]) != NULL) {
                printf("%s", line);
                break;
            }
        }
    }
    printf("\n");
    fclose(file);
}

int is_config_file(const char *filename) {
    // Check for common configuration file extensions
    for (size_t i = 0; i < sizeof(config_extensions) / sizeof(config_extensions[0]); i++) {
        if (strstr(filename, config_extensions[i]) != NULL) {
            return 1;
        }
    }
    // Check for common configuration file keywords
    for (size_t i = 0; i < sizeof(config_keywords) / sizeof(config_keywords[0]); i++) {
        if (strstr(filename, config_keywords[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

void list_config_files(const char *directory) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(directory)) == NULL) {
        perror("opendir");
        return;
    }

    printf(COLOR_CYAN "Listing configuration files in directory: %s\n" COLOR_RESET, directory);

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
            if (is_config_file(entry->d_name)) {
                printf(COLOR_MAGENTA "Configuration file found: %s/%s\n" COLOR_RESET, directory, entry->d_name);
            }
        }
    }
    printf("\n");
    closedir(dir);
}

void search_config_files(const char *directory, const char *extension) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(directory)) == NULL) {
        perror("opendir");
        return;
    }

    printf( "Exploring directory: %s for files with extension '%s'\n" , directory, extension);

    struct timespec ts = {0, 100000000}; 
    nanosleep(&ts, NULL);

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            search_config_files(path, extension);
        } else {
            if (strstr(entry->d_name, extension) != NULL) {
                printf(COLOR_YELLOW "Configuration file found: %s\n" COLOR_RESET, path);
                read_config_file(path);
            }
        }
    }
    closedir(dir);
}