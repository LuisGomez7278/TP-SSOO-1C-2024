#ifndef KERNEL_CPU_DISPATCH_H_
#define KERNEL_CPU_DISPATCH_H_

#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern int socket_kernel_cpu_dispatch;
extern int socket_kernel_cpu_interrupt;
extern int socket_memoria_kernel;
extern int socket_entradasalida_kernel;
 
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

void atender_conexion_CPU_DISPATCH_KERNEL ();

#endif /*  KERNEL_CPU_DISPATCH_H_ */
