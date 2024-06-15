#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_


#include <stdlib.h>
#include <stdio.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern int socket_entradasalida_kernel;
extern int socket_escucha;
 
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

void atender_conexion_ENTRADASALIDA_KERNEL();
void escuchar_a_ENTRADASALIDA();
void enviar_a_ENTRADASALIDA();

#endif /*  KERNEL_ENTRADASALIDA_H_ */
