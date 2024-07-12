
#ifndef ENTRADASALIDA_MEMORIA_H_
#define ENTRADASALIDA_MEMORIA_H_

#include <stdbool.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern int32_t socket_entradasalida_memoria;

extern t_log* logger;
extern t_log* logger_debug;

extern sem_t respuesta_memoria;
extern char* string_leida_memoria;


void gestionar_conexion_memoria();

#endif //ENTRADASALIDA_MEMORIA_H_