#ifndef TP_MEMORIA_MAIN_H_
#define TP_MEMORIA_MAIN_H_

#include <readline/readline.h>
#include <commons/bitarray.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../include/memPaginacion.h"
#include "../include/memIniciar.h"
#include "../include/memCpu.h"
#include "../include/extGlobales.h"
#include "../include/memES.h"
#include "../include/memKernel.h"

t_log* logger_debug;
char* path_base;
int tam_pagina;
int tam_memoria;
int retardo;
t_config* config;
char* puerto_escucha;
int cant_frames;
void* memoria_usuario; 

int socket_cpu_memoria_dispatch;
int socket_cpu_memoria_interrupt;
int socket_kernel_memoria;
int socket_entradasalida_memoria;
int socket_escucha;

// memoria de instrucciones
t_instruccion* parsear_instruccion(char* linea);
t_list* leer_pseudocodigo(char* path);
cod_ins hash_ins(char* ins);
char* path_completo(char* path_base, char* path_parcial);
t_instruccion* get_ins(t_list* lista_instrucciones, uint32_t PC);

#endif //TP_MEMORIA_MAIN_H_