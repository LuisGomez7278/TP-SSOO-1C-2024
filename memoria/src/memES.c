#include "../include/memES.h"

void conexion_con_es(){
    bool continuarIterando = true;
    while(continuarIterando){
        op_code codigo = recibir_operacion(socket_cpu_memoria);
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_cpu_memoria,logger_debug);
            break;
        case SOLICITUD_IO_READ:
            read_es();
            break;
        case SOLICITUD_IO_WRITE:
            write_es();
            break;
        default:
            log_error(logger_debug, "el MODULO DE ES SE DESCONECTO. Terminando servidor");
            continuarIterando = 0;
            break;
        }
        
    }
}

void read_es(){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    uint32_t* desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer!=NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);

        char* leido = leer_memoria(dir_fisica, bytes, PID);

        t_paquete* paquete = crear_paquete(SOLICITUD_IO_READ);
        agregar_a_paquete_string(paquete, strlen(leido), leido);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);            
        //log_info(logger_debug, "IO_READ completado");
        }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
        }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void write_es(){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    uint32_t* desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint8(buffer, desplazamiento);
        char* escribir = leer_de_buffer_string(buffer, desplazamiento);

        bool escrito = escribir_memoria(dir_fisica, bytes, escribir, PID);

        if(escrito){
            t_paquete* paquete = crear_paquete(OK);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "IO_WRITE perfecto");
            }else{
            t_paquete* paquete = crear_paquete(FALLO);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);  
            log_info(logger_debug, "IO_WRITE fallido");
            }   
        }else{
            // Manejo de error en caso de que recibir_buffer devuelva NULL
            log_error(logger_debug,"Error al recibir el buffer");
        }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}