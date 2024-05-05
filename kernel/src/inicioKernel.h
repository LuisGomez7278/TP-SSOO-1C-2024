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

#include <utils/hello.h>
#include "extern_globales.h"


void iniciar_Kernel(void);
void iniciar_logs(void);
void iniciar_configs(void);
void imprimir_configs(void);








#endif /* INICIO_KERNEL_H_ */
