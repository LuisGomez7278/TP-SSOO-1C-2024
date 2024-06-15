#include "../include/memCpu.h"

/*
void atender_conexion_CPU_MEMORIA(){
    //ENVIAR MENSAJE A CPU
    enviar_mensaje("MEMORIA manda mensaje a CPU", socket_cpu_memoria);
    log_info(logger, "Se envio el primer mensaje a kernel");

    // CREO HILO ESCUCHA CPU
    pthread_t hilo_escucha_cpu_memoria;
    pthread_create(&hilo_escucha_cpu_memoria,NULL,(void*)conexion_con_cpu,NULL);
    pthread_detach(hilo_escucha_cpu_memoria);
}
*/

void conexion_con_cpu(){
    bool continuarIterando = true;
    while(continuarIterando){
        op_code codigo = recibir_operacion(socket_cpu_memoria);

        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_cpu_memoria,logger_debug);
            break;
        case FETCH:
            fetch(socket_cpu_memoria);
            break;
        case TAM_PAG:
            uint32_t tam_pag = tam_pagina;
            t_paquete* paquete = crear_paquete(TAM_PAG);
            agregar_a_paquete_uint32(paquete, tam_pag);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "Se envia el tamaño de pagina a CPU");
            break;
        case TLB_MISS:
            frame(socket_cpu_memoria);
            break;
        case SOLICITUD_MOV_IN: 
            movIn(socket_cpu_memoria);
            break;
        case SOLICITUD_MOV_OUT: 
            movOut(socket_cpu_memoria);
            break;
        case SOLICITUD_COPY_STRING: 
            copiar_string(socket_cpu_memoria);
            break;
        case SOLICITUD_RESIZE:
            ins_resize(socket_cpu_memoria);
            break;
        default:
            log_error(logger_debug, "el MODULO DE CPU SE DESCONECTO. Terminando servidor");

            break;
        }
        
    }
}

void fetch(int socket_cpu_memoria){ 
    uint32_t PID=0; 
    uint32_t PC=0;
    recibir_fetch(socket_cpu_memoria, &PID, &PC);
    log_info(logger_debug, "CPU solicita instruccion, PID: %d, PC: %d", PID, PC);
    
    t_list* lista_instrucciones = obtener_instrs(PID);

    if(lista_instrucciones==NULL){ 
        t_paquete* paquete = crear_paquete(PROCESO_NO_CARGADO);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
    }

    t_instruccion* sig_ins = get_ins(lista_instrucciones, PC);
    usleep(retardo);
    enviar_instruccion(socket_cpu_memoria, sig_ins);
    log_info(logger_debug, "instruccion enviada");
}

void frame(int socket_cpu_memoria){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer,desplazamiento);
        uint32_t pagina = leer_de_buffer_uint32(buffer,desplazamiento);

        log_info(logger_debug,"ACCESO A TABLA DE PAGINAS: PID = %u  Pagina = %d  ",PID,pagina);

        uint32_t marco = encontrar_frame(PID, pagina); 

        t_paquete* paquete = crear_paquete(TLB_MISS);
        agregar_a_paquete_uint32(paquete, marco);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);            
        log_info(logger_debug, "Se envia el marco: %d, asignado a la pagina: %d, a CPU", marco, pagina);
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void movIn(int socket_cpu_memoria){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, desplazamiento);
        uint8_t bytes = leer_de_buffer_uint8(buffer, desplazamiento);

        uint32_t leido = leer_memoria_uint32_t(dir_fisica, bytes, PID);

        t_paquete* paquete = crear_paquete(SOLICITUD_MOV_IN);
        agregar_a_paquete_uint32(paquete, leido);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);            
        log_info(logger_debug, "Mov_In completado");
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void movOut(int socket_cpu_memoria){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t escribir = leer_de_buffer_uint32(buffer, desplazamiento);

        bool escrito = escribir_uint32_t_en_memoria(dir_fisica, bytes, escribir, PID);

        if(escrito){
            t_paquete* paquete = crear_paquete(OK);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "Mov_Out perfecto");
        }else{
            t_paquete* paquete = crear_paquete(FALLO);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);  
            log_info(logger_debug, "Mov_Out fallido");
        }        
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void copiar_string(int socket_cpu_memoria){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t dir_fisica_escribir = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);

        char* leido = leer_memoria(dir_fisica_leer, bytes, PID);
        bool escrito = escribir_memoria(dir_fisica_escribir, bytes, leido, PID);

        if(escrito){
            t_paquete* paquete = crear_paquete(OK);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "Copy_String perfecto");
        }else{
            t_paquete* paquete = crear_paquete(FALLO);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);  
            log_info(logger_debug, "Copy_String fallido");
        }        
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void ins_resize(int socket_cpu_memoria){
    uint32_t *sizeTotal = malloc(sizeof(uint32_t));
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal, socket_cpu_memoria);
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, desplazamiento);

        bool exito = resize(PID, bytes);

        if(exito){
            t_paquete* paquete = crear_paquete(OK);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "Resize perfecto");
        }else{
            t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
            enviar_paquete(paquete, socket_cpu_memoria);
            eliminar_paquete(paquete);  
            log_info(logger_debug, "Resize fallido");
        }        
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(sizeTotal);
    free(desplazamiento);
    free(buffer);
}

void recibir_fetch(int socket_cpu_memoria, uint32_t* PID, uint32_t* PC){
    uint32_t size;
    int desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    *PID = leer_de_buffer_uint32(buffer, &desplazamiento);
    *PC = leer_de_buffer_uint32(buffer, &desplazamiento);
}

void enviar_instruccion(int socket_cpu_memoria, t_instruccion* instruccion){
    t_paquete* paquete = crear_paquete(FETCH);
    //log_info(logger, "Paquete creado");
    agregar_a_paquete_cod_ins(paquete, instruccion->ins);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg1) + 1, instruccion->arg1);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg2) + 1, instruccion->arg2);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg3) + 1, instruccion->arg3);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg4) + 1, instruccion->arg4);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg5) + 1, instruccion->arg5);

    enviar_paquete(paquete, socket_cpu_memoria);
    //log_info(logger, "Paquete enviado");
    eliminar_paquete(paquete);
}