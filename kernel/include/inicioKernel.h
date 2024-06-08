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



#include "atender_operacion.h"
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "extern_globales.h"


void iniciar_Kernel(void);
void iniciar_logs(void);
void iniciar_configs(void);
void iniciar_colas_de_estado();


t_queue *cola_new;
t_queue *cola_ready;
t_queue *cola_exit;
t_queue *cola_bloqueado;




#endif /* INICIO_KERNEL_H_ */


