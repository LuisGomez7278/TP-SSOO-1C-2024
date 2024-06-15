#ifndef TP_ENTRADASALIDA_MAIN_H_
#define TP_ENTRADASALIDA_MAIN_H_

#include <pthread.h>
#include <stdbool.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "entradasalida_var_globales.h"
#include "entradasalida_inicio.h"
#include "entradasalida-memoria.h"
#include "entradasalida-kernel.h"

extern t_log* logger;
extern t_config* config;
 
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern int socket_entradasalida_kernel;
extern int socket_entradasalida_memoria;
 
extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
 
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;

void validar_argumentos(char* nombre_interfaz, char* config_interfaz);
t_instruccion* recibir_instruccion_IO(int socket_cliente, t_log* logger);
void ejecutar_instruccion_IO(t_instruccion* instruccion);


#endif //TP_ENTRADASALIDA_MAIN_H_