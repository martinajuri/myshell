#include "../include/metrics.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 256
#define PROC_MEMINFO "/proc/meminfo"
#define PROC_STAT "/proc/stat"
#define PROC_DISKSTATS "/proc/diskstats"
#define PROC_NET_DEV "/proc/net/dev"
#define ERROR_VALUE -1.0

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen(PROC_MEMINFO, "r");
    if (fp == NULL)
    {
        perror("Error opening" PROC_MEMINFO);
        return ERROR_VALUE;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error reading memory information from " PROC_MEMINFO "\n");
        return ERROR_VALUE;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent;
}

void get_memory_stats(double* total_memory, double* used_memory, double* available_memory)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen(PROC_MEMINFO, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_MEMINFO);
        *total_memory = ERROR_VALUE;
        *used_memory = ERROR_VALUE;
        *available_memory = ERROR_VALUE;
        return;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error reading memory information from " PROC_MEMINFO "\n");
        *total_memory = ERROR_VALUE;
        *used_memory = ERROR_VALUE;
        *available_memory = ERROR_VALUE;
        return;
    }

    // Calcular los valores de memoria
    *total_memory = (double)total_mem;
    *used_memory = (double)(total_mem - free_mem);
    *available_memory = (double)free_mem;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen(PROC_STAT, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_STAT);
        return ERROR_VALUE;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error when reading " PROC_STAT);
        fclose(fp);
        return ERROR_VALUE;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Parsing error " PROC_STAT "\n");
        return ERROR_VALUE;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald is zero, cannot calculate CPU usage!\n");
        return ERROR_VALUE;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

double get_disk_io()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long read_sectors = 0, write_sectors = 0;

    // Abrir el archivo /proc/diskstats
    fp = fopen(PROC_DISKSTATS, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_DISKSTATS);
        return ERROR_VALUE;
    }

    // Leer las estadísticas de disco
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned int major, minor;
        char device[32];
        unsigned long long reads, writes;

        // Parsear la línea para obtener las estadísticas de disco
        if (sscanf(buffer, "%u %u %s %*u %*u %llu %*u %*u %*u %llu", &major, &minor, device, &reads, &writes) == 5)
        {
            read_sectors += reads;
            write_sectors += writes;
        }
    }

    fclose(fp);

    // Calcular el total de I/O de disco
    double total_io = (double)(read_sectors + write_sectors);

    return total_io;
}

double get_network_traffic()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long rx_bytes = 0, tx_bytes = 0;

    // Abrir el archivo /proc/net/dev
    fp = fopen(PROC_NET_DEV, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_NET_DEV);
        return ERROR_VALUE;
    }

    // Saltar las dos primeras líneas de encabezado
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    // Leer las estadísticas de red
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char iface[32];
        unsigned long long rx, tx;

        // Parsear la línea para obtener las estadísticas de red
        if (sscanf(buffer, "%s %llu %*u %*u %*u %*u %*u %*u %llu", iface, &rx, &tx) == 3)
        {
            rx_bytes += rx;
            tx_bytes += tx;
        }
    }

    fclose(fp);

    // Calcular el total de tráfico de red
    double total_traffic = (double)(rx_bytes + tx_bytes);

    return total_traffic;
}

double get_process_count()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long process_count = 0;

    // Abrir el archivo /proc/stat
    fp = fopen(PROC_STAT, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_STAT);
        return ERROR_VALUE;
    }

    // Leer las estadísticas de procesos
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "processes %llu", &process_count) == 1)
        {
            break; // Encontramos el conteo de procesos
        }
    }

    fclose(fp);

    return (double)process_count;
}

double get_context_switches()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long context_switches = 0;

    // Abrir el archivo /proc/stat
    fp = fopen(PROC_STAT, "r");
    if (fp == NULL)
    {
        perror("Error opening " PROC_STAT);
        return ERROR_VALUE;
    }

    // Leer las estadísticas de cambios de contexto
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "ctxt %llu", &context_switches) == 1)
        {
            break; // Encontramos los cambios de contexto
        }
    }

    fclose(fp);

    return (double)context_switches;
}
