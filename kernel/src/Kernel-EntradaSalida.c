#include "../include/Kernel-EntradaSalida.h"

void atender_conexion_ENTRADASALIDA_KERNEL(){

    while (1)
    {
        IO_type* nueva_interfaz = malloc(sizeof(IO_type));
        nueva_interfaz->socket_interfaz = esperar_cliente(socket_escucha, logger);
        log_info(logger_debug,"Kernel conectado a  UNA I/O");

    //ENVIAR MENSAJE ENTRADA SALIDA
        enviar_mensaje("kernel manda mensaje a nueva interfaz", nueva_interfaz->socket_interfaz);
        log_info(logger, "Se envio el primer mensaje a la nueva interfaz");

    //RECIBIR TIPO Y NOMBRE DE INTERFAZ
        op_code cod_operacion= recibir_operacion(nueva_interfaz->socket_interfaz);

        switch (cod_operacion)
        {
        case NUEVA_IO:
        crear_interfaz(nueva_interfaz);
        
        pthread_t hilo_escucha_ENTRADASALIDA_KERNEL;
        pthread_create(&hilo_escucha_ENTRADASALIDA_KERNEL,NULL,(void*)escuchar_a_Nueva_Interfaz,(*void)nueva_interfaz);
        pthread_detach(hilo_escucha_ENTRADASALIDA_KERNEL);

        default:
            log_error(logger_debug,"Error al recibir nueva interfaz")
            break;
        }

    

    }

    
}


void crear_interfaz (IO_type* nueva_interfaz){
        uint32_t size;
        void* buffer=recibir_buffer(&size,nueva_interfaz->socket_interfaz);
        uint32_t desplazamiento=0;
        nueva_interfaz->tipo_interfaz= leer_de_buffer_tipo_interfaz(buffer,&desplazamiento);
        nueva_interfaz->nombre_interfaz=leer_de_buffer_string(buffer,&desplazamiento);
        
        nueva_interfaz->cola_de_espera=list_create();
        sem_init(&nueva_interfaz->control_envio_interfaz 0, 0)
        pthread_mutex_lock(&semaforo_lista_interfaces);
        list_add(lista_de_interfaces,nueva_interfaz);
        pthread_mutex_unlock(&semaforo_lista_interfaces);
        log_info(logger_debug,"Creada nueva interfaz: %s",nueva_interfaz->nombre_interfaz);
        free(buffer);
}


    

void escuchar_a_Nueva_Interfaz(){

    



}

















