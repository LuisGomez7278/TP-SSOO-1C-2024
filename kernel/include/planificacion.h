#ifndef PLANIFICACION_H
#define PLANIFICACION_H

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
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <sys/time.h>
#include <commons/temporal.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"




#include "extern_globales.h"

t_temporal * temporizador=NULL;
uint32_t tiempo_ya_ejecutado;
pthread_t hilo_de_desalojo_por_quantum;
uint32_t pcb_actual_en_cpu=0;
uint32_t backup_de_quantum_ejecutado;

void interruptor_de_QUANTUM(void* quantum_de_pcb);

void enviar_siguiente_proceso_a_ejecucion ();

void enviar_nuevamente_proceso_a_ejecucion(t_pcb* pcb_a_reenviar);
void respuesta_CPU_recurso(op_code respuesta);







#endif //PLANIFICACION_H
