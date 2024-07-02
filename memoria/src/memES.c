#include "../include/memES.h"

void conexion_con_es(){
    enviar_mensaje("CONEXION CON MEMORIA OK", socket_kernel_memoria);
    log_info(logger, "Handshake enviado: IO");
    bool continuarIterando = true;
    while(continuarIterando){
        op_code codigo = recibir_operacion(socket_cpu_memoria);
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_cpu_memoria,logger_debug);
            break;
        case SOLICITUD_IO_STDIN_READ:
            read_es();
            break;
        case SOLICITUD_IO_STDOUT_WRITE:
            write_es();
            break;
        default:
            log_error(logger_debug, "Modulo ENTRADA SALIDA se desconectó. Terminando servidor");
            continuarIterando = 0;
            break;
        }
    }
}

void read_es(){                                                                     
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    uint32_t *desplazamiento = malloc(sizeof(uint32_t));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    int i=0;
    char* leido = string_new();
    
    if(buffer != NULL){
        uint32_t n = leer_de_buffer_uint32(buffer, desplazamiento);

        while(i<n){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);

        char* leido2 = leer_memoria(dir_fisica_leer, bytes, PID);
        string_append(&leido, leido2);
        free(leido2);
        ++i;
        }

        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(SOLICITUD_IO_STDIN_READ);
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
    free(leido);
}

void write_es(){                                                                   
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    uint32_t *desplazamiento = malloc(sizeof(uint32_t));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    int i=0;
    bool escrito = true;
    
    if(buffer != NULL){
        uint32_t n = leer_de_buffer_uint32(buffer, desplazamiento);

        while(i<n && escrito){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);
        char* escribir = leer_de_buffer_string(buffer, desplazamiento);

        escrito = escribir_memoria(dir_fisica, bytes, escribir, PID);
        free(escribir);
        ++i;
        }

        usleep(retardo*1000);

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