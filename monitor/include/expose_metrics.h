/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU y memoria y exponerlos como métricas de Prometheus.
 */

#include "metrics.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep

/**
 * Tamaño del búfer para leer los datos.
 */
#define BUFFER_SIZE 256

/**
 * @brief Actualiza la métrica de uso de CPU.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 */
void update_memory_gauge();

/**
 * @brief Actualiza las métricas de memoria.
 */
void update_memory_metrics();

/**
 * @brief Actualiza las métricas de E/S de disco.
 */
void update_disk_io_metrics();

/**
 * @brief Actualiza las métricas de tráfico de red.
 */

void update_network_traffic_metrics();

/**
 * @brief Actualiza las métricas de conteo de procesos.
 */
void update_process_count_metrics();

/**
 * @brief Actualiza las métricas de cambios de contexto.
 */
void update_context_switches_metrics();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializar mutex y métricas.
 */
int init_metrics();

/**
 * @brief Destructor de mutex
 */
void destroy_mutex();
