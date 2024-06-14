#ifndef TP_KERNEL_MAIN_H_
#define TP_KERNEL_MAIN_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "Kernel_var_globales.h"
#include "Kernel-CPU-dispatch.h"
#include "Kernel-EntradaSalida.h"
#include "inicioKernel.h"
// #include "consola.h"

extern char* ip_memoria;
extern char* puerto_memoria;
extern char* ip_cpu;
extern char* puerto_cpu_dispatch;
extern char* puerto_cpu_interrupt;
extern char* puerto_escucha;
extern char* algoritmo_planificacion;
extern int quantum;
extern char** recursos;
extern char** instancias_recursos;
extern int grado_multiprogramacion;

extern int socket_kernel_cpu_dispatch;
extern int socket_kernel_cpu_interrupt;
extern int socket_memoria_kernel;
extern int socket_entradasalida_kernel;
extern int socket_escucha;
 
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;
 
extern t_queue *cola_new;
extern t_queue *cola_ready;
extern t_queue *cola_exit;
extern t_queue *cola_bloqueado;

#endif //TP_KERNEL_MAIN_H_