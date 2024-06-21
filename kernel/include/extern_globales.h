#ifndef VARIABLES_GLOBALES_H_
#define  VARIABLES_GLOBALES_H_

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





extern char* ip_memoria;
extern char* puerto_memoria;
extern char* ip_cpu;
extern char* puerto_cpu_dispatch;
extern char* puerto_cpu_interrupt;
extern char* puerto_escucha;
extern char* algoritmo_planificacion;
extern int64_t quantum;
extern char** recursos;
extern char** instancias_recursos;
extern int32_t grado_multiprogramacion;
extern uint32_t* instancias_recursos_int;
extern t_recurso* lista_de_recursos;

extern uint32_t socket_kernel_cpu_dispatch;
extern uint32_t socket_kernel_cpu_interrupt; //queda libre por ahora
extern uint32_t socket_memoria_kernel;
extern uint32_t socket_entradasalida_kernel;
extern uint32_t socket_escucha;

extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

extern uint32_t conexion_CPU_DISPATCH;
extern t_log* logger;
extern t_config *config;


extern t_list *lista_new;
extern t_list *lista_ready;
extern t_list *lista_exit;
extern t_list *lista_bloqueado;
extern t_list *lista_bloqueado_prioritario;
extern t_list *lista_ready_prioridad;


extern sem_t control_multiprogramacion;
extern sem_t cantidad_procesos_new;
extern sem_t cantidad_procesos_ready;
extern sem_t cantidad_procesos_ready_prioritario;
extern sem_t cantidad_procesos_en_algun_ready;
extern sem_t semaforo_plp;
extern sem_t semaforo_pcp;
extern bool detener_planificacion;

extern pthread_mutex_t semaforo_new;
extern pthread_mutex_t semaforo_ready;
extern pthread_mutex_t semaforo_ready_prioridad;
extern pthread_mutex_t semaforo_bloqueado_prioridad;
extern pthread_mutex_t semaforo_bloqueado;
extern pthread_mutex_t mutex_recursos;

extern uint32_t identificador_PID;
extern pthread_mutex_t mutex_pid;
extern uint32_t pcb_actual_en_cpu;


//atender_operacion.h:
    void atender_instruccion_validada(char* leido);
    void iniciar_proceso(char*leido);
    void finalizar_proceso_con_pid(uint32_t pid_a_finalizar);

//planificacion.h
    void loggeo_de_cambio_estado(uint32_t pid, t_estado viejo, t_estado nuevo);
    void ingresar_en_lista(t_pcb* pcb, t_list* lista,  pthread_mutex_t* semaforo_mutex, sem_t* semaforo_contador, t_estado estado);
    void cambiar_grado_multiprogramacion(int32_t nuevo_valor);
    void gestionar_dispatch ();

//kernel-cpu-dispatch
    void atender_conexion_CPU_DISPATCH_KERNEL ();

//inicio kernel
    void iniciar_Kernel(void);
    void iniciar_logs(void);
    void iniciar_configs(void);
    void iniciar_estructuras_planificacion();

//consola.h
    bool validacion_de_ingreso_por_consola (char* leido);
    void iniciar_consola_interactiva();

//Kernel-EntradaSalida.h
    void atender_conexion_ENTRADASALIDA_KERNEL();
    void escuchar_a_ENTRADASALIDA();

//kernel-Memoria.h
    void atender_conexion_MEMORIA_KERNEL();
    void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido);
    void carga_exitosa_en_memoria();
    t_pcb* buscar_pcb_por_PID_en_lista(t_list* lista, uint32_t pid_buscado);

//servicios_kernel
    uint32_t asignar_pid();

//recursos.h
    uint32_t* convertir_a_enteros_la_lista_de_instancias(char** array_de_cadenas);
    void construir_lista_de_recursos();
    void imprimir_recursos();
    uint32_t wait_recursos(char* recurso_solicitado,t_pcb* pcb_solicitante);
    uint32_t signal_recursos ( char*recurso_solicitado,uint32_t PID);
    void eliminar_proceso_de_lista_recursos (uint32_t PID);

#endif /*  VARIABLES_GLOBALES_H_ */
