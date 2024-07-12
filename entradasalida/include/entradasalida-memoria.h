
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

void ejecutar_IO_STDIN(uint32_t marco, uint32_t offset, char* string_leida);
void solicitar_lectura__memoria(uint32_t PID, void* resto_del_buffer);

#endif //ENTRADASALIDA_MEMORIA_H_