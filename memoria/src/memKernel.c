#include "../include/memKernel.h"


void atender_conexion_KERNEL_MEMORIA(){
    //ENVIAR MENSAJE A KERNEL
    enviar_mensaje("MEMORIA manda mensaje a Kernel", socket_kernel_memoria);
    log_info(logger, "Se envio el primer mensaje a kernel");

    // CREO HILO ESCUCHA KERNEL
    pthread_t hilo_escucha_kenel_memoria;
    pthread_create(&hilo_escucha_kenel_memoria,NULL,(void*)conexion_con_kernel,NULL);
    pthread_join(hilo_escucha_kenel_memoria,NULL);
}

void conexion_con_kernel(){
    bool continuarIterando = true;
    while (continuarIterando) {
        op_code codigo = recibir_operacion(socket_kernel_memoria);   
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_kernel_memoria,logger_debug);
            break;
        case CREAR_PROCESO:
            crear_proceso();
            break;
        case ELIMINAR_PROCESO:
            eliminar_proceso();
            break;
        default:
            log_error(logger_debug, "el MODULO DE KERNEL SE DESCONECTO. Terminando servidor");
            continuarIterando = 0;
            break;
        }
    }
}

void crear_proceso(){ // llega el pid y el path de instrucciones
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int32_t *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal,socket_kernel_memoria);

    if (buffer != NULL) {
        uint32_t PID = leer_de_buffer_uint32(buffer,desplazamiento);
        char* path_parcial = leer_de_buffer_string(buffer,desplazamiento);

        log_info(logger_debug,"LLEGO UN PROCESO PARA CARGAR: PID = %u  direccion= %s  ",PID,path_parcial);
        
        bool creado = crear_procesoM(path_parcial, PID);

        if(creado){
        enviar_instruccion_con_PID_por_socket(CARGA_EXITOSA_PROCESO, PID,socket_kernel_memoria);
        }    
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void eliminar_proceso(){ // llega un pid
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int32_t *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal,socket_kernel_memoria);

    if(buffer!=NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer,desplazamiento);

        log_info(logger_debug,"LLEGO UN PROCESO PARA ELIMINAR: PID = %u",PID);

        eliminar_procesoM(PID);
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

/*
void contestar_a_kernel_carga_proceso(op_code codigo_operacion, uint32_t PID){                  ///ESTA FUNCION YA LA HICE GENERICA, "enviar_instruccion_con_PID_por_socket"
                                                                                                //(DESPUES HABRIA QUE ELIMINARLA)

    t_paquete *paquete= crear_paquete (codigo_operacion);
    agregar_a_paquete_uint32(paquete,PID);
    enviar_paquete(paquete,socket_kernel_memoria);              //--------------ESTA FUNCION SERIALIZA EL PAQUETE ANTES DE ENVIARLO --quedaria un void*= cod_op||SIZE TOTAL||PID(uint_32)
    eliminar_paquete(paquete);

}
*/                  


