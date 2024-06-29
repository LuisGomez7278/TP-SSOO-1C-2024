#ifndef TP_ENTRADASALIDA_MAIN_H_
#define TP_ENTRADASALIDA_MAIN_H_

#include <pthread.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "entradasalida_var_globales.h"
#include "entradasalida_inicio.h"
#include "entradasalida-memoria.h"
#include "entradasalida-kernel.h"

char* nombre_interfaz;
char* config_interfaz;

void validar_argumentos(char* nombre_interfaz, char* config_interfaz);
void notificar_kernel(uint32_t PID);
char* leer_de_teclado(uint32_t tamanio_a_leer);

#endif //TP_ENTRADASALIDA_MAIN_H_