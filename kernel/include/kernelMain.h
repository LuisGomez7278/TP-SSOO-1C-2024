#ifndef TP_KERNEL_MAIN_H_
#define TP_KERNEL_MAIN_H_

#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

//#include "planificacion.h"
//#include "Kernel-CPU-dispatch.h"
//#include "Kernel-CPU-Interrupt.h"
//#include "Kernel-EntradaSalida.h"
//#include "Kernel-Memoria.h"
//#include "inicioKernel.h"
//#include "consola.h"


#include "extern_globales.h"







    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
	int quantum;
	char** recursos;
	char** instancias_recursos;
	int grado_multiprogramacion;





    int socket_kernel_cpu_dispatch;
    int socket_kernel_cpu_interrupt; //queda libre por ahora
    int socket_memoria_kernel;
    int socket_entradasalida_kernel;
    int socket_escucha;



    t_log* logger;
    t_log* logger_debug;
    t_config* config;

    t_list *lista_new;
    t_list *lista_ready;
    t_list *lista_exit;
    t_list *lista_bloqueado;
    t_list *lista_ready_prioridad;

    




#endif //TP_KERNEL_MAIN_H_