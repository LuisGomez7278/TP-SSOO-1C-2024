#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

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

extern int socket_memoria_kernel;
 
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

void atender_conexion_MEMORIA_KERNEL();
void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido);
void carga_exitosa_en_memoria();

#endif /*  KERNEL_MEMORIA_H_ */
