#ifndef ENTRADASALIDA_FS_H_
#define ENTRADASALIDA_FS_H_

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include <commons/bitarray.h>

extern t_log* logger;

extern uint32_t TIEMPO_UNIDAD_TRABAJO;
extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;

extern char* path_bloques;
extern FILE* archivo_bloques;
extern char* path_bitmap;
extern FILE* bitmap_bloques;
extern char* path_metadata;

void inicializar_FS();
void crear_bitmap();

#endif //ENTRADASALIDA_FS_H_