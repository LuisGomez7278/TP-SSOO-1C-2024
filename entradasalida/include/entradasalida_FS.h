#ifndef ENTRADASALIDA_FS_H_
#define ENTRADASALIDA_FS_H_

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include <commons/bitarray.h>
#include <commons/memory.h>

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

void inicializar_FS();
void inicializar_bloques();
void inicializar_bitmap();

void crear_archivo(char* nombre_archivo);
int32_t buscar_bloque_libre();
void crear_metadata(int32_t bloque, char* nombre_archivo);

#endif //ENTRADASALIDA_FS_H_