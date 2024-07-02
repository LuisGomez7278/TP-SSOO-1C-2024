#include "../include/memKernel.h"


void conexion_con_kernel(){
    
    //ENVIAR MENSAJE A KERNEL
        enviar_mensaje("CONEXION CON MEMORIA OK", socket_kernel_memoria);
        log_info(logger, "Handshake enviado: KERNEL");
    
    
    
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
                log_error(logger_debug, "Modulo KERNEL se desconectó.Cerrando Socket de kernel");
                continuarIterando = false;
                break;
            }
        }
}

void crear_proceso(){ // llega el pid y el path de instrucciones
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_kernel_memoria);

    if (buffer != NULL) {
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        char* path_parcial = leer_de_buffer_string(buffer, &desplazamiento);

        log_info(logger_debug,"Llego un proceso para cargar: PID: %u  Direccion: %s",PID,path_parcial);
        
        bool creado = crear_procesoM(path_parcial, PID);

        usleep(retardo*1000);

        if(creado){
            log_info(logger, "Proceso cargado con exito, PID: %u", PID);
            enviar_instruccion_con_PID_por_socket(CARGA_EXITOSA_PROCESO, PID, socket_kernel_memoria);
        }else{
            log_info(logger, "Falla al cargar un proceso, PID: %u", PID);
            enviar_instruccion_con_PID_por_socket(ERROR_AL_CARGAR_EL_PROCESO, PID, socket_kernel_memoria);
        }  

    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void eliminar_proceso(){ // llega un pid
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    uint32_t *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal,socket_kernel_memoria);

    if(buffer!=NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer,desplazamiento);
        log_info(logger_debug,"Llego un proceso para eliminar: PID: %u",PID);

        eliminar_procesoM(PID);
        usleep(retardo*1000);
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

            


