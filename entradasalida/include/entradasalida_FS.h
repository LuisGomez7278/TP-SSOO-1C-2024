#ifndef ENTRADASALIDA_FS_H_
#define ENTRADASALIDA_FS_H_

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include <commons/bitarray.h>
#include <commons/memory.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>

extern t_log* logger;
extern t_log* logger_debug;

extern uint32_t TIEMPO_UNIDAD_TRABAJO;
extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;

extern char* path_bloques;
// extern FILE* archivo_bloques;
extern char* path_bitmap;
extern t_bitarray* bitmap_bloques;
extern char* path_metadata;
extern t_list* archivos_existentes;

extern sem_t respuesta_memoria;


void inicializar_FS();
void inicializar_bloques();
void inicializar_bitmap();

void crear_archivo(char* nombre_archivo);
int32_t buscar_bloque_libre();
void crear_metadata(int32_t bloque, char* nombre_archivo);

void eliminar_archivo(char* nombre_archivo);
bool existe_archivo(char* nombre_archivo, int32_t* indice);
void liberar_bloques(char* path_archivo_metadata);

void truncar_archivo(char* nombre_archivo, uint32_t nuevo_tamanio);
int32_t cantidad_de_bloques(int32_t tamanio_archivo);
void liberar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_liberar);
bool asignar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_asignar);
bool reasignar_bloques(t_config* metadata, int32_t cant_bloques, int32_t nueva_cant_bloques);

void FS_WRITE(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, char* datos_a_escribir);
void FS_READ(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, void* datos_leidos);

#endif //ENTRADASALIDA_FS_H_