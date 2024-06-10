#ifndef TP_MEMORIA_EXT_H_
#define TP_MEMORIA_EXT_H_

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
#include <commons/bitarray.h>
#include <pthread.h>

#include <semaphore.h>
#include <commons/collections/queue.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"

extern t_bitarray* bitmap;
extern void* memoria_usuario; 
extern t_list* tablaDePaginas;

//LOGGER
extern t_log* logger;

//CONFIG
extern char* puerto_escucha;
extern char* path_base;
extern int tam_pagina;
extern int tam_memoria;
extern int retardo;
extern int cant_frames;
extern t_log* logger_debug;

extern t_config* config;

extern int socket_cpu_memoria_dispatch;
extern int socket_cpu_memoria_interrupt;
extern int socket_kernel_memoria;
extern int socket_entradasalida_memoria;
extern int socket_escucha;

#endif //TP_MEMORIA_EXT_H_