#ifndef ENTRADASALIDA_VAR_GLOBALES_H_
#define ENTRADASALIDA_VAR_GLOBALES_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

t_log* logger;
t_config* config;

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
int socket_entradasalida_kernel;
int socket_entradasalida_memoria;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;

char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;


#endif /*  ENTRADASALIDA_INICIO_H_ */