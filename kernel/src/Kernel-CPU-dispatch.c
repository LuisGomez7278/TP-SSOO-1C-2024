#include "../include/Kernel-CPU-dispatch.h"
    
    void atender_conexion_CPU_DISPATCH_KERNEL (){


    //Recibir un mensaje de CPU


        op_code codop1 = recibir_operacion(socket_kernel_cpu_dispatch);
        if (codop1 == MENSAJE) {printf("LLego un mensaje\n");}
        else {printf("LLego otra cosa");}
        recibir_mensaje(socket_kernel_cpu_dispatch, logger);

        //Enviar un mensaje a CPU
        enviar_mensaje("Kernel manda mensaje a CPU", socket_kernel_cpu_dispatch);
        log_info(logger, "Se envio el primer mensaje a CPU");

    }