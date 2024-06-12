#ifndef CPU_KERNEL_H_
#define CPU_KERNEL_H_

#include <pthread.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_config* config;

extern char* ip_memoria;
extern char* puerto_memoria;
extern char* puerto_escucha_dispatch;
extern char* puerto_escucha_interrupt;
extern int socket_cpu_kernel_dispatch;
extern int socket_cpu_kernel_interrupt;
extern int socket_cpu_memoria;
extern int socket_escucha;

extern int cant_entradas_TLB;
extern char* algoritmo_TLB;

extern uint32_t PID;
extern int_code interrupcion;
extern t_contexto_ejecucion contexto_interno;

void recibir_proceso(void/*int socket_cpu_kernel_dispatch, uint32_t* PID, t_contexto_ejecucion* contexto_interno*/);
void desalojar_proceso(void/*int socket_cpu_kernel_dispatch, uint32_t* PID, t_contexto_ejecucion* contexto_interno, int_code motivo_interrupcion*/);

#endif //CPU_KERNEL_H_