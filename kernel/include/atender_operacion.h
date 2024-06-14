#ifndef  ATENDER_OPERACION_H
#define ATENDER_OPERACION_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "Kernel-Memoria.h"
#include "servicios_kernel.h"

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

void atender_instruccion_validada(char* leido);
void iniciar_proceso(char*leido);

#endif //ATENDER_OPERACION_H
