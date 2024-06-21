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

t_log* logger;
t_log* logger_debug;
char* path_base;
int32_t tam_pagina;
int32_t tam_memoria;
int32_t retardo;
t_config* config;
char* puerto_escucha;
int32_t cant_frames;
void* memoria_usuario; 

int32_t socket_cpu_memoria;
int32_t socket_kernel_memoria;
int32_t socket_entradasalida_memoria;
int32_t socket_escucha;


#endif //TP_MEMORIA_MAIN_H_