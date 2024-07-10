#ifndef TP_CPU_MAIN_H_
#define TP_CPU_MAIN_H_

#include <pthread.h>
#include <stdbool.h>
#include "../../utils/include/utils.h"
#include <semaphore.h>
#include "CPU_var_globales.h"
#include "CPU_inicio.h"
#include "CPU-memoria.h"
#include "CPU-kernel.h"
#include "CPU-kernel.h"
#include "CPU_mmu.h"

void ejecutar_instruccion(uint32_t PID, t_contexto_ejecucion* contexto_interno, t_instruccion* ins_actual);
void* direccion_registro(t_contexto_ejecucion* contexto, char* registro);
bool registro_chico(char* registro);
void ejecutar_IO_STD_IN(char* nombre_interfaz, uint32_t direccion_logica, uint32_t tamanio_a_leer);
void ejecutar_IO_STD_OUT(char* nombre_interfaz, uint32_t direccion_logica, uint32_t tamanio_a_leer);
void ejecutar_IO_FS_TRUNCATE(char* nombre_interfaz, char* nombre_archivo, uint32_t tamanio);
void loggear_valores();
 
#endif //TP_CPU_MAIN_H_