#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_


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

void crear_nodo_interfaz (IO_type* nueva_interfaz);
void escuchar_a_Nueva_Interfaz();
void gestionar_cola_nueva_interfaz(void* interfaz);








#endif /*  KERNEL_ENTRADASALIDA_H_ */
