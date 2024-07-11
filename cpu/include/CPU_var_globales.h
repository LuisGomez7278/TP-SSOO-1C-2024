#ifndef CPU_VAR_GLOBALES_H_
#define CPU_VAR_GLOBALES_H_

#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

t_log* logger;
t_log* logger_debug;
t_log* logger_valores;
t_config* config;

char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;

int32_t socket_cpu_kernel_dispatch;
int32_t socket_cpu_kernel_interrupt;
int32_t socket_cpu_memoria;
int32_t socket_escucha_dispatch;
int32_t socket_escucha_interrupt;

uint32_t cant_entradas_TLB;
char* algoritmo_TLB;
uint32_t tamanio_de_pagina;
bool usa_TLB;
t_list* tabla_TLB;
bool detener_ejecucion=true;

uint32_t PID;
t_contexto_ejecucion contexto_interno;
sem_t hay_proceso_ejecutando;
sem_t espera_iterador;

t_instruccion* ins_actual;
sem_t prox_instruccion;
char* string_leida_de_memoria;

op_code interrupcion;
op_code motivo_desalojo;

pthread_t hilo_conexion_dispatch;
pthread_t hilo_conexion_interrupt;
pthread_t hilo_conexion_memoria;

#endif /*  CPU_VAR_GLOBALES_H */
