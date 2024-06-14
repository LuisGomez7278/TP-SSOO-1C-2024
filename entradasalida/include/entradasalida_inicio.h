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

void iniciar_entradasalida(void);
void iniciar_logs(void);
void iniciar_config(void);

#endif /*  ENTRADASALIDA_INICIO_H_ */