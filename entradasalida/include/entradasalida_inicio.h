#ifndef ENTRADASALIDA_INICIO_H_
#define ENTRADASALIDA_INICIO_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

extern t_log* logger;
extern t_config* config;

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;

extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;

extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;

void iniciar_entradasalida(char* nombre_interfaz, char* config_interfaz);
void iniciar_logs(char* nombre_interfaz);
void iniciar_config(char* config_interfaz);
cod_interfaz get_tipo_interfaz(char* TIPO_INTERFAZ);


#endif /*  ENTRADASALIDA_INICIO_H_ */