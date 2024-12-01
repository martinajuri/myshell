/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer utilizado para leer datos.
 */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtiene las estadísticas de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y los almacena
 * en las variables de salida.
 *
 * @param total_memory Puntero a la variable de salida para la memoria total.
 * @param used_memory Puntero a la variable de salida para la memoria usada.
 * @param available_memory Puntero a la variable de salida para la memoria disponible.
 */
void get_memory_stats(double* total_memory, double* used_memory, double* available_memory);

/**
 * @brief Obtiene el tráfico total de red desde /proc/net/dev.
 *
 * Lee las estadísticas de red desde /proc/net/dev y calcula el tráfico total
 * de red en bytes.
 *
 * @return Tráfico total de red en bytes, o -1.0 en caso de error.
 */
double get_network_traffic();

/**
 * @brief Obtiene el número de procesos en ejecución desde /proc/stat.
 *
 * Lee las estadísticas de procesos desde /proc/stat y cuenta el número total
 * de procesos en ejecución.
 *
 * @return Número de procesos en ejecución, o -1.0 en caso de error.
 */
double get_process_count();

/**
 * @brief Obtiene el número total de operaciones de I/O de disco desde /proc/diskstats.
 *
 * Lee las estadísticas de I/O de disco desde /proc/diskstats y calcula el número
 * total de operaciones de I/O de disco.
 *
 * @return Número total de operaciones de I/O de disco, o -1.0 en caso de error.
 */
double get_disk_io();

/**
 * @brief Obtiene el número total de cambios de contexto desde /proc/stat.
 *
 * Lee las estadísticas de cambios de contexto desde /proc/stat y calcula el número
 * total de cambios de contexto.
 *
 * @return Número total de cambios de contexto, o -1.0 en caso de error.
 */
double get_context_switches();