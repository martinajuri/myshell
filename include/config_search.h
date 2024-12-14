#ifndef CONFIG_SEARCH_H
#define CONFIG_SEARCH_H

/**
 * @brief Busca archivos de configuración en un directorio.
 * 
 * @param directory Directorio a buscar.
 * @param extension Extensión de los archivos a buscar.
 */
void search_config_files(const char *directory, const char *extension);

/**
 * @brief Lee un archivo de configuración.
 * 
 * @param file_path Ruta del archivo a leer.
 */
void read_config_file(const char *file_path);

/**
 * @brief Verifica si un archivo es de configuración.
 * 
 * @param file_name Nombre del archivo a verificar.
 * @return int 1 si es un archivo de configuración, 0 en caso contrario.
 */
void list_config_files(const char *directory);

/**
 * @brief Verifica si un archivo es de configuración.
 * 
 * @param file_name Nombre del archivo a verificar.
 * @return int 1 si es un archivo de configuración, 0 en caso contrario.
 */
int is_config_file(const char *file_name);

#endif // CONFIG_SEARCH_H
