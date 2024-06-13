
#ifndef INICIO_KERNEL_H_
#define INICIO_KERNEL_H_


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



sem_t control_multiprogramacion;
sem_t cantidad_procesos_new;
sem_t cantidad_procesos_ready;

pthread_mutex_t semaforo_new;
pthread_mutex_t semaforo_ready;
pthread_mutex_t semaforo_bloqueado;










#endif /* INICIO_KERNEL_H_ */


