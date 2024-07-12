#ifndef ENTRADASALIDA_INICIO_H_
#define ENTRADASALIDA_INICIO_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;

extern char* TIPO_INTERFAZ;
extern uint32_t TIEMPO_UNIDAD_TRABAJO;

extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;

extern sem_t respuesta_memoria;

void iniciar_entradasalida(char* nombre_interfaz, char* config_interfaz);
void iniciar_logs(char* nombre_interfaz);
void iniciar_config(char* config_interfaz);
cod_interfaz get_tipo_interfaz(char* TIPO_INTERFAZ);


#endif /*  ENTRADASALIDA_INICIO_H_ */