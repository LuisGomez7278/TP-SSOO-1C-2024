#ifndef ENTRADASALIDA_KERNEL_H_
#define ENTRADASALIDA_KERNEL_H_

#include <pthread.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_config* config;

extern int socket_entradasalida_kernel;

void atender_conexion_entradasalida_KERNEL();


#endif //ENTRADASALIDA_KERNEL_H_