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
        recibir_mensaje(nueva_interfaz->socket_interfaz,logger_debug);
            enviar_mensaje("CONEXION CON KERNEL OK", socket_kernel_cpu_dispatch);
             log_info(logger, "Handshake enviado: CPU-DISPATCH");
        enviar_mensaje("CONEXION CON KERNEL OK", nueva_interfaz->socket_interfaz);
        log_info(logger_debug, "Handshake enviado: INTERFAZ");

        /*
    //RECIBIR TIPO Y NOMBRE DE INTERFAZ
        op_code cod_operacion= recibir_operacion(nueva_interfaz->socket_interfaz);

        switch (cod_operacion)
        {
        case NUEVA_IO:
        crear_nodo_interfaz(nueva_interfaz);
        
        pthread_t hilo_escucha_ENTRADASALIDA_KERNEL;
        pthread_create(&hilo_escucha_ENTRADASALIDA_KERNEL,NULL,(void*)escuchar_a_Nueva_Interfaz,nueva_interfaz);    //Hilo donde escucho los mensajes que envia la interfaz recien creada
        pthread_detach(hilo_escucha_ENTRADASALIDA_KERNEL);

        pthread_t hilo_gestion_Cola_interfaz;
        pthread_create(&hilo_gestion_Cola_interfaz,NULL,(void*)gestionar_envio_cola_nueva_interfaz,nueva_interfaz);       //Hilo que envia instrucciones a medida que se desocupa la interfaz
        pthread_detach(hilo_gestion_Cola_interfaz);

        default:
            log_error(logger_debug,"Error al recibir nueva interfaz");
            break;
        }
    */
    }
}


void crear_nodo_interfaz (IO_type* nueva_interfaz){                                 ///RECIBO DE BUFFER:  COD_INTERFAZ  STRING_NOMBRE 
        uint32_t size;
        void* buffer=recibir_buffer(&size,nueva_interfaz->socket_interfaz);
        uint32_t desplazamiento=0;
        nueva_interfaz->tipo_interfaz= leer_de_buffer_tipo_interfaz(buffer,&desplazamiento);
        nueva_interfaz->nombre_interfaz=leer_de_buffer_string(buffer,&desplazamiento);
        free(buffer);

        IO_type* vieja_interfaz=buscar_interfaz_con_nombre(nueva_interfaz->nombre_interfaz);        //Busco en la lista de interfaces si ya existe una interfaz con ese nombre, si no existe la creo
        if(vieja_interfaz==NULL){
            nueva_interfaz->cola_de_espera=list_create();
            sem_init(&nueva_interfaz->control_envio_interfaz, 0, 0);
            sem_init(&nueva_interfaz->utilizacion_interfaz,0,0);
            pthread_mutex_lock(&semaforo_lista_interfaces);
            list_add(lista_de_interfaces,nueva_interfaz);  //-------------------------------------         //LO AGREGO A LA LISTA DE INTERFACES
            pthread_mutex_unlock(&semaforo_lista_interfaces);
            log_info(logger_debug,"Creada nueva interfaz: %s",nueva_interfaz->nombre_interfaz);

  
        }else{             
            log_info(logger_debug,"Se conecto nuevamente la interfaz: %s",nueva_interfaz->nombre_interfaz);                                                                                 //si ya existe una interfaz con ese nombre solo actualizo el socket
            vieja_interfaz->socket_interfaz= nueva_interfaz->socket_interfaz;
            sem_init(&vieja_interfaz->utilizacion_interfaz,0,0);
           
            free(nueva_interfaz);
        }
      
}


    

void escuchar_a_Nueva_Interfaz(void* interfaz){
    IO_type* interfaz_puntero_hilo= (IO_type*) interfaz;
    bool continuarIterando=true;
    op_code operacion;
    t_pid_paq* elemento_lista_espera;
    while(continuarIterando){    
        
        operacion = recibir_operacion(interfaz_puntero_hilo->socket_interfaz);
        
        switch (operacion) 
        {
        case SOLICITUD_EXITOSA_IO:
            elemento_lista_espera= (t_pid_paq*) list_remove(interfaz_puntero_hilo->cola_de_espera,0);
            sem_wait(&interfaz_puntero_hilo->control_envio_interfaz);
            cambiar_proceso_de_block_a_ready(elemento_lista_espera->PID_cola);
            free(elemento_lista_espera);
            sem_post(&interfaz_puntero_hilo->utilizacion_interfaz);
            break;
        case ERROR_SOLICITUD_IO:                                            //Como no solicita esta funcionalidad sigo enviando procesos y el que arrojo error queda bloqueado forever
            elemento_lista_espera= (t_pid_paq*)list_remove(interfaz_puntero_hilo->cola_de_espera,0);
            sem_wait(&interfaz_puntero_hilo->control_envio_interfaz);
            log_error(logger_debug,"La interfaz: %s, no pudo realizar la operacion para el proceso PID: %u. Enviando proximo proceso.",interfaz_puntero_hilo->nombre_interfaz,elemento_lista_espera->PID_cola);
            free(elemento_lista_espera);
            break;
        case FALLO:
            log_error(logger_debug,"La interfaz %s se deconecto, cerrando socket",interfaz_puntero_hilo->nombre_interfaz);      //si falla el recv cierro el hilo
            continuarIterando=false;
            break;
        default:
            log_error(logger_debug,"Llegó una operacion desconocida");
            break;
        }
    
    }
}




void gestionar_envio_cola_nueva_interfaz(void* interfaz){
    IO_type* interfaz_puntero_hilo= (IO_type*) interfaz;

    while (interfaz_puntero_hilo->socket_interfaz>0)                            //sigo iterando hasta que se cierre el socket de esa interfaz
    {    
    sem_wait(&interfaz_puntero_hilo->control_envio_interfaz);
    sem_wait(&interfaz_puntero_hilo->utilizacion_interfaz);
    pthread_mutex_lock(&semaforo_lista_interfaces);
    t_pid_paq* a_enviar= (t_pid_paq*)list_get(interfaz_puntero_hilo->cola_de_espera,0);
    pthread_mutex_unlock(&semaforo_lista_interfaces);
    enviar_paquete(a_enviar->paquete_cola,interfaz_puntero_hilo->socket_interfaz);
    free(a_enviar->paquete_cola);
    }
    log_error(logger_debug,"Gestor de cola de la INTERFAZ: '%s' terminado por cierre de socket",interfaz_puntero_hilo->nombre_interfaz);
 
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


void cambiar_proceso_de_block_a_ready(uint32_t PID){
     t_pcb* pcb;

if (list_size(lista_bloqueado_prioritario)>0)
{
    pthread_mutex_lock(&semaforo_bloqueado_prioridad);
    pcb=buscar_pcb_por_PID_en_lista(lista_bloqueado_prioritario ,PID,&semaforo_bloqueado_prioridad);
    list_remove_element(lista_bloqueado_prioritario,pcb);
    pthread_mutex_unlock(&semaforo_bloqueado_prioridad);
}
if (pcb==NULL && list_size(lista_bloqueado)>0)
{
    pthread_mutex_lock(&semaforo_bloqueado);
    pcb=buscar_pcb_por_PID_en_lista(lista_bloqueado,PID,&semaforo_bloqueado);
    list_remove_element(lista_bloqueado,pcb);
    pthread_mutex_unlock(&semaforo_bloqueado);

}

if (pcb==NULL)
{
    log_error(logger_debug,"No se encontro el proceso PID: %u a desbloquear por fin de IO",PID);
    return;
}


if (strcmp("VRR",algoritmo_planificacion)==0)
{
    ingresar_en_lista(pcb,lista_ready_prioridad,&semaforo_ready_prioridad,&cantidad_procesos_en_algun_ready,READY_PRIORITARIO);
}else{
    ingresar_en_lista(pcb,lista_ready,&semaforo_ready,&cantidad_procesos_en_algun_ready,READY);

}


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











