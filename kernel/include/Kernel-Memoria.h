#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

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
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "extern_globales.h"







 void atender_conexion_MEMORIA_KERNELL();
void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido);











#endif /*  KERNEL_MEMORIA_H_ */
