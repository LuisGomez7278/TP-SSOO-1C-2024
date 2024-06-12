#ifndef TP_MEMORIA_KERNEL_H_
#define TP_MEMORIA_KERNEL_H_

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

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"
#include "../include/memPaginacion.h"

void atender_conexion_KERNEL_MEMORIA();
void escuchando_KERNEL_memoria();
void crear_proceso();
void eliminar_proceso();
void contestar_a_kernel_carga_proceso(op_code codigo_operacion, uint32_t PID);

#endif //TP_MEMORIA_KERNEL_H_