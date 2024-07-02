#ifndef CPU_KERNEL_H_
#define CPU_KERNEL_H_

#include <pthread.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_log* logger_debug;
extern int socket_cpu_kernel_dispatch;
extern int socket_cpu_kernel_interrupt;

extern uint32_t PID;
extern t_contexto_ejecucion contexto_interno;

extern int_code interrupcion;
extern op_code tipo_interrupcion;

void recibir_proceso(void);
void desalojar_proceso(op_code motivo_desalojo);
void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1);
void enviar_CE_con_2_arg(op_code motivo_desalojo, char* arg1, char* arg2);
void enviar_CE_con_5_arg(op_code motivo_desalojo, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5);
bool esperar_respuesta_recurso();

void gestionar_conexion_interrupt();


#endif //CPU_KERNEL_H_