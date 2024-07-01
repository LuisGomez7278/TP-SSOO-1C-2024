#include "../include/Kernel-CPU-dispatch.h"
    
void atender_conexion_CPU_DISPATCH_KERNEL(){
    log_info(logger, "Inicio conexion entre CPU y Kernel por dispatch");

    //ENVIAR MENSAJE A CPU-DISPATCH
    enviar_mensaje("Kernel manda mensaje a CPU-DISPATCH", socket_kernel_cpu_dispatch);
    log_info(logger_debug, "Se envio el primer mensaje a CPU-DISPATCH");

    gestionar_dispatch ();
    
}

