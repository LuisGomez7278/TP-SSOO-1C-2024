#ifndef KERNEL_CPU_DISPATCH_H_
#define KERNEL_CPU_DISPATCH_H_


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "extern_globales.h"

void atender_conexion_CPU_DISPATCH_KERNEL ();

#endif /*  KERNEL_CPU_DISPATCH_H_ */
