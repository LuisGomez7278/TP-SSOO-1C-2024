#ifndef TP_KERNEL_MAIN_H_
#define TP_KERNEL_MAIN_H_

#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "planificacion.h"
#include "Kernel-CPU-dispatch.h"
#include "Kernel-CPU-Interrupt.h"
#include "Kernel-EntradaSalida.h"
#include "Kernel-Memoria.h"
#include "inicioKernel.h"


#include "extern_globales.h"

pthread_t hilo_sts;

    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;

    int socket_kernel_cpu_dispatch;
    int socket_kernel_cpu_interrupt; //queda libre por ahora
    int socket_memoria_kernel;
    int socket_entradasalida_kernel;
    int socket_escucha;

    t_log* logger;
    t_log* logger_debug;
    t_config* config;


#endif //TP_KERNEL_MAIN_H_