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
#include "CPU_test.h"

void ejecutar_instruccion(uint32_t PID, t_contexto_ejecucion* contexto_interno, t_instruccion* ins_actual);
void* direccion_registro(t_contexto_ejecucion* contexto, char* registro);
int tam_registro(char* registro);

#endif //TP_CPU_MAIN_H_