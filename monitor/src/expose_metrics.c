#include "../include/expose_metrics.h"

/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t* cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t* memory_usage_metric;

/** Métricas adicionales */
static prom_gauge_t* total_memory_metric;
static prom_gauge_t* used_memory_metric;
static prom_gauge_t* available_memory_metric;
static prom_gauge_t* disk_io_metric;
static prom_gauge_t* network_traffic_metric;
static prom_gauge_t* process_count_metric;
static prom_gauge_t* context_switches_metric;

void update_cpu_gauge()
{
    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting CPU usage\n");
    }
}

void update_memory_gauge()
{
    double usage = get_memory_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting memory usage\n");
    }
}

void update_memory_metrics()
{
    double total_memory, used_memory, available_memory;
    get_memory_stats(&total_memory, &used_memory, &available_memory);
    if (total_memory >= 0 && used_memory >= 0 && available_memory >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(total_memory_metric, total_memory, NULL);
        prom_gauge_set(used_memory_metric, used_memory, NULL);
        prom_gauge_set(available_memory_metric, available_memory, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting memory statistics\n");
    }
}

void update_disk_io_metrics()
{
    double disk_io = get_disk_io();
    if (disk_io >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_io_metric, disk_io, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting disk I/O statistics\n");
    }
}

void update_network_traffic_metrics()
{
    double network_traffic = get_network_traffic();
    if (network_traffic >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(network_traffic_metric, network_traffic, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting network traffic statistics\n");
    }
}

void update_process_count_metrics()
{
    double process_count = get_process_count();
    if (process_count >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(process_count_metric, process_count, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting process count\n");
    }
}

void update_context_switches_metrics()
{
    double context_switches = get_context_switches();
    if (context_switches >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(context_switches_metric, context_switches, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error getting context switches\n");
    }
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error starting HTTP server\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

int init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error initializing mutex\n");
        return EXIT_FAILURE;
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error initializing Prometheus registry\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "CPU usage percentage", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error creating CPU usage metric\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Memory usage percentage", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error creating memory usage metric\n");
        return EXIT_FAILURE;
    }

    // Creamos las métricas adicionales
    total_memory_metric = prom_gauge_new("total_memory", "Total memory in kB", 0, NULL);
    used_memory_metric = prom_gauge_new("used_memory", "Used memory in kB", 0, NULL);
    available_memory_metric = prom_gauge_new("available_memory", "Available memory in kB", 0, NULL);
    disk_io_metric = prom_gauge_new("disk_io", "Disk I/O operations", 0, NULL);
    network_traffic_metric = prom_gauge_new("network_traffic", "Network traffic in bytes", 0, NULL);
    process_count_metric = prom_gauge_new("process_count", "Number of running processes", 0, NULL);
    context_switches_metric = prom_gauge_new("context_switches", "Number of context switches", 0, NULL);

    // Verificamos que todas las métricas se hayan creado correctamente
    if (total_memory_metric == NULL || used_memory_metric == NULL || available_memory_metric == NULL ||
        disk_io_metric == NULL || network_traffic_metric == NULL || process_count_metric == NULL ||
        context_switches_metric == NULL)
    {
        fprintf(stderr, "Error creating one or more additional metrics\n");
        return EXIT_FAILURE;
    }

    // Registramos todas las métricas en el registro por defecto
    prom_collector_registry_must_register_metric(cpu_usage_metric);
    prom_collector_registry_must_register_metric(memory_usage_metric);
    prom_collector_registry_must_register_metric(total_memory_metric);
    prom_collector_registry_must_register_metric(used_memory_metric);
    prom_collector_registry_must_register_metric(available_memory_metric);
    prom_collector_registry_must_register_metric(disk_io_metric);
    prom_collector_registry_must_register_metric(network_traffic_metric);
    prom_collector_registry_must_register_metric(process_count_metric);
    prom_collector_registry_must_register_metric(context_switches_metric);

    return EXIT_SUCCESS;
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}
