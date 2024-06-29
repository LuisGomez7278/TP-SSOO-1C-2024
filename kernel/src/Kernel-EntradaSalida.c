#include "../include/Kernel-EntradaSalida.h"

void atender_conexion_ENTRADASALIDA_KERNEL(){

    while (1)
    {
        IO_type* nueva_interfaz = malloc(sizeof(IO_type));
        if (nueva_interfaz == NULL) {
            perror("malloc");
            return;
        }
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
        crear_nodo_interfaz(nueva_interfaz);
        
        pthread_t hilo_escucha_ENTRADASALIDA_KERNEL;
        pthread_create(&hilo_escucha_ENTRADASALIDA_KERNEL,NULL,(void*)escuchar_a_Nueva_Interfaz,nueva_interfaz);
        pthread_detach(hilo_escucha_ENTRADASALIDA_KERNEL);

        pthread_t hilo_gestion_Cola_interfaz;
        pthread_create(&hilo_gestion_Cola_interfaz,NULL,(void*)gestionar_cola_nueva_interfaz,nueva_interfaz);
        pthread_detach(hilo_gestion_Cola_interfaz);

        default:
            log_error(logger_debug,"Error al recibir nueva interfaz");
            break;
        }

    }
}


void crear_nodo_interfaz (IO_type* nueva_interfaz){                                 ///RECIBO DE BUFFER:  COD_INTERFAZ  STRING_NOMBRE 
        uint32_t size;
        void* buffer=recibir_buffer(&size,nueva_interfaz->socket_interfaz);
        uint32_t desplazamiento=0;
        nueva_interfaz->tipo_interfaz= leer_de_buffer_tipo_interfaz(buffer,&desplazamiento);
        nueva_interfaz->nombre_interfaz=leer_de_buffer_string(buffer,&desplazamiento);
        
        nueva_interfaz->cola_de_espera=list_create();
        sem_init(&nueva_interfaz->control_envio_interfaz, 0, 0);
        pthread_mutex_lock(&semaforo_lista_interfaces);
        list_add(lista_de_interfaces,nueva_interfaz);  //-------------------------------------         //LO AGREGO A LA LISTA DE INTERFACES
        pthread_mutex_unlock(&semaforo_lista_interfaces);
        log_info(logger_debug,"Creada nueva interfaz: %s",nueva_interfaz->nombre_interfaz);
        free(buffer);



}


    

void escuchar_a_Nueva_Interfaz(void* interfaz){
    //IO_type* interfaz_puntero_hilo= (IO_type*) interfaz;
    bool continuarIterando=true;
    op_code operacion;

    while(continuarIterando){    

        operacion = recibir_operacion(socket_kernel_cpu_dispatch);
   
        switch (operacion)
        {
        case SOLICITUD_EXITOSA_IO:
            //quitar proceso de la cola
            break;
        case ERROR_SOLICITUD_IO:
            //loggear el error
            break;
        default:
            break;
        }
    
    }
}

void gestionar_cola_nueva_interfaz(void* interfaz){

}

bool validar_conexion_interfaz_y_operacion (char* nombre_interfaz, op_code operacion_solicitada){
    t_link_element *auxiliar=lista_de_interfaces->head;
    IO_type* puntero_interfaz=NULL;

    while (auxiliar!=NULL)                                  //BUSCO LA INTERFAZ
    {
        puntero_interfaz= (IO_type*) auxiliar->data;
        
        if(strcmp(puntero_interfaz->nombre_interfaz,nombre_interfaz)==0){
            break;
        }else{
            auxiliar=auxiliar->next;
        }
    }

    if (auxiliar==NULL)
    {   
        log_error(logger_debug,"La interfaz %s no existe", nombre_interfaz);
        return false;
    }
    
    switch (operacion_solicitada)                   //COMO SE ENCONTRO LA INTERFAZ VERIFICO SI ADMITE LA OPERACION
    {
        case DESALOJO_POR_IO_GEN_SLEEP:
            if (puntero_interfaz->tipo_interfaz!= GENERICA)
            {
                log_error(logger_debug,"La interfaz: %s no admite esta operacion",puntero_interfaz->nombre_interfaz);
                return false;
            }
                break;
        case DESALOJO_POR_IO_STDIN:
            if (puntero_interfaz->tipo_interfaz!= STDIN)
            {
                log_error(logger_debug,"La interfaz: %s no admite esta operacion",puntero_interfaz->nombre_interfaz);
                return false;
            }
                break;                   
        case DESALOJO_POR_IO_STDOUT:
            if (puntero_interfaz->tipo_interfaz!= STDOUT)
            {
                log_error(logger_debug,"La interfaz: %s no admite esta operacion",puntero_interfaz->nombre_interfaz);
                return false;
            }
                break;          
        case DESALOJO_POR_IO_FS_CREATE:
        case DESALOJO_POR_IO_FS_DELETE:
        case DESALOJO_POR_IO_FS_TRUNCATE:
        case DESALOJO_POR_IO_FS_WRITE:
        case DESALOJO_POR_IO_FS_READ:
            if (puntero_interfaz->tipo_interfaz!=DIALFS)
            {
                log_error(logger_debug,"Se solicito a la interfaz: %s un operacion que no existe",nombre_interfaz);
                return false;
            }
                break;  
    
        default:
                log_error(logger_debug,"La operacion solicitada a la interfaz %s no existe",puntero_interfaz->nombre_interfaz);
                return false;
            break;
    }

    op_code a_enviar=VERIFICAR_CONEXION;
    int32_t comprobar_conexion=send(puntero_interfaz->socket_interfaz,&a_enviar,sizeof(op_code),MSG_NOSIGNAL);   //// ENVIO UN MENSAJE DE COMPROBACION Y SI ESTA DESCONECTADO DEVUELVE "-1"
   
   
    if (comprobar_conexion<0)                           //COMPRUEBO QUE ESTE CONECTADA
    {
        log_error(logger_debug,"La interfaz %s se encuentra desconectada",puntero_interfaz->nombre_interfaz);
        return false;
    }
    
    return true;

}

void agregar_a_cola_interfaz(char* nombre_interfaz, uint32_t PID, t_paquete* paquete){
    IO_type* interfaz=buscar_interfaz_con_nombre (nombre_interfaz);
    t_pid_paq* pidConPaq=malloc(sizeof(t_pid_paq));
    
    if (pidConPaq == NULL) {
    log_error(logger_debug,"Fallo el malloc de agregar cola interfaz");
    }
    
    pidConPaq->paquete_cola=paquete;
    pidConPaq->PID_cola=PID;

    pthread_mutex_lock(&semaforo_lista_interfaces);
    list_add(interfaz->cola_de_espera,pidConPaq);
    pthread_mutex_unlock(&semaforo_lista_interfaces);

    sem_post(&interfaz->control_envio_interfaz);

   
}

   


IO_type* buscar_interfaz_con_nombre(char*nombre_a_buscar){
    t_link_element *auxiliar=lista_de_interfaces->head;
    IO_type* puntero_interfaz=NULL;

    while (auxiliar!=NULL)                                  //BUSCO LA INTERFAZ
    {
        puntero_interfaz= (IO_type*) auxiliar->data;
        
        if(strcmp(puntero_interfaz->nombre_interfaz,nombre_a_buscar)==0){
            break;
        }else{
            auxiliar=auxiliar->next;
        }
    }

    if (auxiliar==NULL)
    {   
        log_error(logger_debug,"Error al obtener socket con nombre de la interfaz %s. Devuelvo 0", nombre_a_buscar);
        return NULL;
    }
    
return puntero_interfaz;


}











