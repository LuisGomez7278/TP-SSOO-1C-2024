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
#include <sys/types.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "atender_operacion.h"

extern t_log* logger_debug;


#ifndef TPANUAL_CONSOLA_H
#define TPANUAL_CONSOLA_H


bool validacion_de_ingreso_por_consola (char* leido);
void iniciar_consola_interactiva();





#endif //TPANUAL_CONSOLA_H
