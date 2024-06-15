#ifndef KERNEL_VARIABLES_GLOBALES_H_
#define  KERNEL_VARIABLES_GLOBALES_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

char* ip_memoria;
char* puerto_memoria;
char* ip_cpu;
char* puerto_cpu_dispatch;
char* puerto_cpu_interrupt;
char* puerto_escucha;

int socket_kernel_cpu_dispatch;
int socket_kernel_cpu_interrupt;
int socket_memoria_kernel;
int socket_entradasalida_kernel;
int socket_escucha;

char* algoritmo_planificacion;
int quantum;
char** recursos;
char** instancias_recursos;
int grado_multiprogramacion;

t_log* logger;
t_log* logger_debug;
t_config* config;

t_queue *cola_new;
t_queue *cola_ready;
t_queue *cola_exit;
t_queue *cola_bloqueado;

#endif /*  KERNEL_VARIABLES_GLOBALES_H_ */
