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

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "recursos.h"

#include "extern_globales.h"












void ingresar_en_lista(t_pcb* pcb, t_list* lista, char* modulo, pthread_mutex_t* semaforo_mutex, sem_t* semaforo, estado_code estado);
void cambiar_grado_multiprogramacion(int nuevo_valor);




/*
typedef  enum {
    FIFO,
    RR,
    //ERROR
} t_algoritmo_planificacion;

sem_t proceso_new;
sem_t proceso_ready;
t_pcb* proceso_actual;

sem_t controlador_pcp;

t_algoritmo_planificacion algoritmo_utilizado;
t_list* planificador_corto_plazo;

t_contexto_ejecucion contexto_actual;




void* pcp_planificar(void* args);
void planificar_fifo();
void planificar_rr();

void cambiar_estado(t_pcb*, t_estado);

t_contexto_ejecucion* obtener_contexto_ejecucion(t_pcb*);
t_algoritmo_planificacion determinar_algoritmo();
t_pcb* modificar_pcb(t_pcb* proceso);

void* cronometrar(void* args);
*/
#endif //PLANIFICACION_H
