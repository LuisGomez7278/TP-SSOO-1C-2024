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

void iniciar_entradasalida(void);
void iniciar_logs(void);
void iniciar_config(void);

#endif /*  ENTRADASALIDA_INICIO_H_ */