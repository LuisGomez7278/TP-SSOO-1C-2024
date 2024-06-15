#ifndef CPU_MEMORIA_H_
#define CPU_MEMORIA_H_

#include <pthread.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_config* config;

extern char* ip_memoria;
extern char* puerto_memoria;
extern int socket_cpu_memoria;

extern int cant_entradas_TLB;
extern char* algoritmo_TLB;
extern uint32_t tamanio_de_pagina;

extern uint32_t PID;
extern t_contexto_ejecucion contexto_interno;

t_instruccion* fetch(uint32_t PID, uint32_t PC, int socket_cpu_memoria);
t_instruccion* recibir_instruccion(int socket_cpu_memoria);
void recibir_tamanio_de_pagina();

#endif //CPU_MEMORIA_H_