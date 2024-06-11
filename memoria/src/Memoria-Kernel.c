#include "../include/Memoria-Kernel.h"


void atender_conexion_KERNEL_MEMORIA(){


    //ENVIAR MENSAJE A KERNEL
        enviar_mensaje("MEMORIA manda mensaje a Kernel", socket_kernel_memoria);
        log_info(logger, "Se envio el primer mensaje a kernel");

    // CREO HILO ESCUCHA KERNEL
            pthread_t hilo_escucha_kenel_memoria;
            pthread_create(&hilo_escucha_kenel_memoria,NULL,(void*)escuchando_KERNEL_memoria,NULL);
            pthread_join(hilo_escucha_kenel_memoria,NULL);
        

}





void escuchando_KERNEL_memoria(){

//RECIBO PRIMER MENSAJE DE KERNEL

    op_code codop2 = recibir_operacion(socket_kernel_memoria);
    if (codop2 == MENSAJE) {log_info(logger, "LLego un mensaje");}
    else {log_info(logger, "LLego otra cosa");}
    recibir_mensaje(socket_kernel_memoria, logger);



    bool continuarIterando=1;
       

        while (continuarIterando) {
            int cod_op = recibir_operacion(socket_kernel_memoria);   ////se queda esperando en recv por ser bloqueante
            switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(socket_kernel_memoria,logger_debug);
                break;
            case CREAR_PROCESO:
                crear_proceso_solicitado_por_kernel();
                break;
            case -1:
                log_error(logger_debug, "el MODULO DE KERNEL SE DESCONECTO. Terminando servidor");
                continuarIterando=0;
                break;
            default:
                log_warning(logger_debug,"Operacion desconocida de KERNEL. No quieras meter la pata");
                break;
            }
        }
}




void crear_proceso_solicitado_por_kernel(){  ///PID Y PATH PARCIAL SON LOS QUE SIRVEN PARA CARGAR EL PROCESO

    uint32_t *sizeTotal=malloc(sizeof(uint32_t));
    int *desplazamiento=malloc(sizeof(int));
    *desplazamiento=0;
    void* buffer= recibir_buffer(sizeTotal,socket_kernel_memoria);
    
   // verificar_paquete(buffer);

    if (buffer != NULL) {
        uint32_t PID= leer_de_buffer_uint32(buffer,desplazamiento);
        char* path_parcial= leer_de_buffer_string(buffer,desplazamiento);


        log_info(logger_debug,"LLEGO UN PROCESO PARA CARGAR: PID= %u  direccion= %s  ",PID,path_parcial);
        
        free(sizeTotal);
        free(desplazamiento);
        free(buffer);
    
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
}






