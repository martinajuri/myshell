/**
 * @file main.c
 * @brief Entry point of the system
 */

#include "../include/expose_metrics.h"
#include "../include/metrics.h"
#include <cjson/cJSON.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tiempo de espera en segundos entre actualizaciones de métricas.
 */
#define DEFAULT_SLEEP_TIME 1

/**
 * @brief Variables globales para controlar qué métricas se actualizan y con qué frecuencia.
 */

/**
 * @brief Indica si se debe actualizar la métrica de uso de CPU.
 */
bool update_cpu = true;

/**
 * @brief Indica si se debe actualizar la métrica de uso de memoria.
 */
bool update_memory = true;

/**
 * @brief Indica si se debe actualizar la métrica de I/O de disco.
 */
bool update_disk_io_flag = true;

/**
 * @brief Indica si se debe actualizar la métrica de tráfico de red.
 */
bool update_network = true;

/**
 * @brief Indica si se debe actualizar la métrica de conteo de procesos.
 */
bool update_process_count = true;

/**
 * @brief Indica si se debe actualizar la métrica de cambios de contexto.
 */
bool update_context_switches = true;

/**
 * @brief Tiempo de espera en segundos entre actualizaciones de métricas.
 */
int sleep_time = DEFAULT_SLEEP_TIME;

/**
 * @brief Variable para controlar la ejecución del bucle principal.
 */
volatile sig_atomic_t keep_running = 1;

/**
 * @brief Nombre del archivo de configuración.
 */
const char* config_filename = NULL;

/**
 * @brief Lee la configuración desde un archivo JSON.
 *
 * @param filename Nombre del archivo JSON.
 * @return EXIT_SUCCESS si la configuración se lee correctamente, EXIT_FAILURE en caso de error.
 */
int read_config(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening configuration file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1);
    if (data == NULL)
    {
        perror("Error allocating memory for configuration file");
        fclose(file);
        return EXIT_FAILURE;
    }

    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON* json = cJSON_Parse(data);
    if (json == NULL)
    {
        fprintf(stderr, "Error parsing JSON file\n");
        free(data);
        return EXIT_FAILURE;
    }

    cJSON* metrics = cJSON_GetObjectItem(json, "metrics");
    if (metrics != NULL)
    {
        update_cpu = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "cpu"));
        update_memory = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "memory"));
        update_disk_io_flag = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "disk_io"));
        update_network = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "network"));
        update_process_count = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "process_count"));
        update_context_switches = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "context_switches"));
    }

    cJSON* sleep_time_item = cJSON_GetObjectItem(json, "sleep_time");
    if (sleep_time_item != NULL && cJSON_IsNumber(sleep_time_item))
    {
        sleep_time = sleep_time_item->valueint;
    }

    cJSON_Delete(json);
    free(data);

    return EXIT_SUCCESS;
}

/**
 * @brief Manejador de señales para SIGHUP.
 *
 * @param signo Número de la señal.
 */
void signal_handler(int signo)
{
    if (signo == SIGHUP)
    {
        printf("Updating configuration...\n");
        if (config_filename != NULL)
        {
            if (read_config(config_filename) == EXIT_SUCCESS)
            {
                printf("\033[0;36mConfiguration updated successfully.\033[0m\n");
            }
            else
            {
                printf("Error updating configuration.\n");
            }
        }
    }
}

/**
 * @brief Actualiza las métricas de CPU, memoria, I/O de disco, tráfico de red, conteo de procesos y cambios de
 * contexto.
 *
 * @return NULL
 */
void* update_metrics()
{
    while (keep_running)
    {
        if (update_cpu)
            update_cpu_gauge();
        if (update_memory)
            update_memory_gauge();
        if (update_memory)
            update_memory_metrics();
        if (update_disk_io_flag)
            update_disk_io_metrics();
        if (update_network)
            update_network_traffic_metrics();
        if (update_process_count)
            update_process_count_metrics();
        if (update_context_switches)
            update_context_switches_metrics();
        sleep(sleep_time);
    }
    return NULL;
}

/**
 * @brief Punto de entrada del sistema.
 *
 * Este es el punto de entrada principal del sistema. Crea un hilo para exponer
 * las métricas vía HTTP y entra en un bucle principal para actualizar las métricas
 * cada segundo.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Array de cadenas de caracteres que contienen los argumentos de la línea de comandos.
 * @return EXIT_SUCCESS si el programa termina correctamente, EXIT_FAILURE en caso de error.
 */
int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        config_filename = argv[1];
        if (read_config(config_filename) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }

    // Configurar manejador de señal para SIGHUP
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("Error configuring signal handler for SIGHUP");
        return EXIT_FAILURE;
    }

    // Creamos un hilo para exponer las métricas vía HTTP
    pthread_t tid_http;
    if (pthread_create(&tid_http, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error creating HTTP server thread\n");
        return EXIT_FAILURE;
    }

    if (init_metrics() != EXIT_SUCCESS)
    {
        fprintf(stderr, "Error initializing metrics\n");
        return EXIT_FAILURE;
    }

    // Creamos un hilo para actualizar las métricas
    pthread_t tid_metrics;
    if (pthread_create(&tid_metrics, NULL, update_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error creating metrics update thread\n");
        return EXIT_FAILURE;
    }

    // Esperar a que el hilo de actualización de métricas termine
    pthread_join(tid_metrics, NULL);

    // Esperar a que el hilo del servidor HTTP termine
    pthread_join(tid_http, NULL);

    return EXIT_SUCCESS;
}
