#ifndef INICIO_KERNEL_H_
#define INICIO_KERNEL_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

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

extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;
 
extern t_queue *cola_new;
extern t_queue *cola_ready;
extern t_queue *cola_exit;
extern t_queue *cola_bloqueado;


void iniciar_Kernel(void);
void iniciar_logs(void);
void iniciar_configs(void);
void iniciar_colas_de_estado();

#endif /* INICIO_KERNEL_H_ */


