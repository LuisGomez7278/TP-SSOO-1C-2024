#include "../include/memES.h"

void conexion_con_es(){

    while (1)
    {
    // ESPERO QUE SE CONECTE E/S
    
        log_trace(logger_debug, "Esperando que se conecte E/S");
        socket_entradasalida_memoria = esperar_cliente(socket_escucha,logger_debug);
    
        int32_t* socket_de_hilo= malloc(sizeof(int32_t));
        if (socket_de_hilo== NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

        *socket_de_hilo=socket_entradasalida_memoria;
    
        pthread_t hilo_escucha_ENTRADASALIDA_MEMORIA;
        pthread_create(&hilo_escucha_ENTRADASALIDA_MEMORIA,NULL,(void*)escuchar_nueva_Interfaz_mem,socket_de_hilo);    //Hilo donde escucho los mensajes que envia la interfaz recien creada
        pthread_detach(hilo_escucha_ENTRADASALIDA_MEMORIA);
    }
}

void escuchar_nueva_Interfaz_mem(void*  socket)
{
    int32_t socket_IO= *((int32_t*)socket);

    enviar_mensaje("CONEXION CON MEMORIA OK", socket_IO);
    log_info(logger, "Handshake enviado: IO");
    
    bool continuarIterando = true;
    while(continuarIterando){
        op_code codigo = recibir_operacion(socket_IO);
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_IO,logger_debug);
            break;
        case SOLICITUD_IO_STDIN_READ:
            write_es(socket_IO);
            break;
        case SOLICITUD_IO_STDOUT_WRITE:
            read_es(socket_IO);
            break;
        case DESALOJO_POR_IO_FS_READ:
            write_es(socket_IO);
            break;
        case DESALOJO_POR_IO_FS_WRITE:

    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket);
    int i=0;
    char* leido = string_new();
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tam_total = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        while(i<n){
        uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, &desplazamiento);

        char* leido2 = leer_memoria(dir_fisica_leer, bytes, PID);
        string_append(&leido, leido2);
        free(leido2);
        ++i;
        }

        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(DESALOJO_POR_IO_FS_WRITE);
        agregar_a_paquete_string(paquete, strlen(leido)+1, leido);
        enviar_paquete(paquete, socket_IO);
        eliminar_paquete(paquete);            
        log_info(logger_debug, "Solicitud de lectura de E/S completada");
        }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
        }
    free(buffer);
    free(leido);

            break;
        default:
            log_error(logger_debug, "Modulo ENTRADA SALIDA se desconectó. Cerrando el socket");
            continuarIterando = 0;
            break;
        }
    }
}


void read_es(int32_t socket){                                                                     
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket);
    int i=0;
    char* leido = string_new();
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tam_total = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        while(i<n){
        uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, &desplazamiento);

        char* leido2 = leer_memoria(dir_fisica_leer, bytes, PID);
        string_append(&leido, leido2);
        free(leido2);
        ++i;
        }

        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(DESALOJO_POR_IO_STDOUT);
        agregar_a_paquete_string(paquete, strlen(leido)+1, leido);
        enviar_paquete(paquete, socket);
        eliminar_paquete(paquete);            
        log_info(logger_debug, "Solicitud de lectura de E/S completada");
        }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
        }
    free(buffer);
    free(leido);
}

void write_es(int32_t socket){                                                                   
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket);
    int i=0;
    bool escrito = true;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tam_total = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        while(i<n && escrito){
        uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
        char* escribir = leer_de_buffer_string(buffer, &desplazamiento);

        escrito = escribir_memoria(dir_fisica, string_length(escribir), escribir, PID);
        free(escribir);
        ++i;
        }

        usleep(retardo*1000);

        if(escrito){
            t_paquete* paquete = crear_paquete(OK);
            enviar_paquete(paquete, socket);
            eliminar_paquete(paquete);            
            log_info(logger_debug, "El pedido de escritura desde E/S resulto perfecto");
        }else{
            t_paquete* paquete = crear_paquete(FALLO);
            enviar_paquete(paquete, socket);
            eliminar_paquete(paquete);  
            log_info(logger_debug, "El pedido de escritura desde E/S resulto fallido");
        }   
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}