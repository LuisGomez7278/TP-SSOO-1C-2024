
#ifndef ENTRADASALIDA_MEMORIA_H_
#define ENTRADASALIDA_MEMORIA_H_

#include <stdbool.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern uint32_t socket_entradasalida_memoria;

extern t_log* logger;
extern t_config* config;

void ejecutar_IO_STDIN(uint32_t marco, uint32_t offset, char* string_leida);

#endif //ENTRADASALIDA_MEMORIA_H_