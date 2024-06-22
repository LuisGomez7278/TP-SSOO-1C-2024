
#include "../include/planificacion.h"

//////////////////   PLANIFICADOR LARGO PLAZO  /////////////////


void ingresar_en_lista(t_pcb* pcb, t_list* lista, pthread_mutex_t* semaforo_mutex, sem_t* semaforo_contador, t_estado estado_nuevo) {
	pthread_mutex_lock(semaforo_mutex);
    char* estado_nuevo_string=codigo_estado_string(estado_nuevo);

	if (pcb->estado != estado_nuevo){
		loggeo_de_cambio_estado(pcb->PID, pcb->estado, estado_nuevo);
	}

	pcb->estado = estado_nuevo;
	list_add(lista, pcb);
	sem_post(semaforo_contador);

	log_info(logger, "Proceso PID:%i ingreso en %s", pcb->PID,estado_nuevo_string );

////////////////////////    LOGGEO OBLIGATORIO DE READY Y READY PRIORITARIO /////////////


	if(strcmp(estado_nuevo_string, "READY")==0){
		char* log_cola_ready = string_new();
		string_append(&log_cola_ready, "[");
		for(int32_t i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready, ", ");
			}
		}
		string_append(&log_cola_ready, "]");
		log_info(logger,"Lista de PID de procesos en estado READY %s : %s \n",algoritmo_planificacion, log_cola_ready);
		free(log_cola_ready);
	}
    
    if(strcmp(estado_nuevo_string, "READY_PRIORITARIO")==0){
		char* log_cola_ready_prioritario = string_new();
		string_append(&log_cola_ready_prioritario, "[");
		for(int32_t i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready_prioritario, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready_prioritario, ", ");
			}
		}


		string_append(&log_cola_ready_prioritario, "]");
		log_info(logger,"Lista de PID de procesos en estado READY Prioritario %s : %s",algoritmo_planificacion, log_cola_ready_prioritario);
		free(log_cola_ready_prioritario);
	}
pthread_mutex_unlock(semaforo_mutex);
}



void loggeo_de_cambio_estado(uint32_t pid, t_estado viejo, t_estado nuevo){
    	log_info(logger, "PID: %u - Cambio de estado %s -> %s", pid, codigo_estado_string(viejo), codigo_estado_string(nuevo));
}





void cambiar_grado_multiprogramacion(int32_t nuevo_valor) {                         // CON ESTA FUNCION AGREGO O DISMINUYO INSTANCIAS DEL SEMAFORO QUE GESTIONA LA MULTIPROGRAMCION 
    int32_t actual_valor;                                                           // SIN PERDER LA INFORMACION DE LOS QUE YA ESTAN EN LA COLA DE WAIT
    sem_getvalue(&control_multiprogramacion, &actual_valor);
    
    if (nuevo_valor > actual_valor) {
        // Incrementar el semáforo
        for (int32_t i = 0; i < nuevo_valor - actual_valor; i++) {
            sem_post(&control_multiprogramacion);                   //Suma instancias al semaforo
        }
    } else if (nuevo_valor < actual_valor) {
        // Decrementar el semáforo
        for (int32_t i = 0; i < actual_valor - nuevo_valor; i++) {
            sem_wait(&control_multiprogramacion);                   //Resta instancias al semaforo
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        /////////////////////////////////////////////////////       PLANIFICADOR CORTO PLAZO               //////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------






void gestionar_dispatch (){ 
op_code cod_op;
uint32_t desplazamiento;
uint32_t size;
int32_t continuarIterando=1;
char* recurso_solicitado;

while(continuarIterando){    

    cod_op = recibir_operacion(socket_kernel_cpu_dispatch);
    
    
    
    if ((strcmp(algoritmo_planificacion,"VRR")==0 ||strcmp(algoritmo_planificacion,"RR")==0 ) && temporizador!=NULL)
    {
        tiempo_recien_ejecutado= temporal_gettime(temporizador); //recupero el valor antes de eliminar el temporizador
        temporal_destroy(temporizador);
        pthread_cancel(hilo_de_desalojo_por_quantum);
    }

////////////////////////////////   EXTRAIGO DEL SOCKET LO COMUN A TODOS LOS PROCESOS //////////////////
    
    void* buffer = recibir_buffer(&size, socket_kernel_cpu_dispatch);

    t_pcb *pcb_dispatch=malloc(sizeof(t_pcb));        
    desplazamiento = 0;
    
    pcb_dispatch->PID = leer_de_buffer_uint32(buffer, &desplazamiento);
    leer_de_buffer_CE(buffer, &desplazamiento, &pcb_dispatch->CE);
    pcb_dispatch->quantum_ejecutado=tiempo_recien_ejecutado+ backup_de_quantum_ejecutado;
    backup_de_quantum_ejecutado=0;      ////    RESETEO BACKUP DE QUANTUM   
    tiempo_recien_ejecutado=0;          ////    RESETEO EL VALOR QUE OBTIENE EL TIEMPO DEL CONTADOR

///////////////////////////////   EJECUTO SEGUN EL CODIGO DE OPERACION  ///////////////////////

       if (detener_planificacion)                      /// Si la PLANIFICACION ESTA DETENIDA QUEDO BLOQEUADO EN WAIT
    {
        sem_wait(&semaforo_pcp);
    }
    

    switch (cod_op){
    case MENSAJE:
        recibir_mensaje(socket_memoria_kernel,logger_debug);
        break;
    case OUT_OF_MEMORY:
        log_info(logger, "Finaliza el proceso PID: %u Motivo: OUT_OF_MEMORY ", pcb_dispatch->PID);
        log_info(logger, "PID: %u - Cambio de estado EXECUTE-> EXIT", pcb_dispatch->PID);
        enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel); ///ELIMINO DE MEMORIA
        eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);                                           //ELIMINO DE LISTA DE RECURSOS ASIGNADOS  
        sem_post(&control_multiprogramacion);                                                             //AGREGO UNA INSTANCIA A CANTIDAD DE PROCESOS  
        enviar_siguiente_proceso_a_ejecucion();
        break;

    case DESALOJO_POR_WAIT:
        recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);

        switch (wait_recursos(recurso_solicitado, pcb_dispatch)) {

            case 1:
                log_info(logger, "PID: %u - Bloqueado por recurso: %s", pcb_dispatch->PID, recurso_solicitado);
                respuesta_CPU_recurso(FALLO);                                                                                             //PCB QUEDO EN COLA DE ESPERA DEL RECURSO
                enviar_siguiente_proceso_a_ejecucion();	

                break;
            case 2: 
                log_info(logger, "PID: %u hace WAIT de recurso: %s exitosamente", pcb_dispatch->PID, recurso_solicitado);
                respuesta_CPU_recurso(OK);                                                                                           //WAIT REALIZADO, DEVOLVER EL PROCESO A EJECUCION
                enviar_nuevamente_proceso_a_ejecucion(pcb_dispatch);

                break;
            case -1:  
                log_info(logger, "Finaliza el proceso PID: %u Motivo: INVALID_RESOURCE: %s", pcb_dispatch->PID, recurso_solicitado);
                log_info(logger, "PID: %u - Cambio de estado EXECUTE-> EXIT", pcb_dispatch->PID);
                respuesta_CPU_recurso(FALLO);                                                                                                  //RECURSO NO ENCONTRADO, ENVIAR PROCESO A EXIT
                enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                sem_post(&control_multiprogramacion);               
                enviar_siguiente_proceso_a_ejecucion();	    
                break;
            default:
                log_error(logger_debug,"La funcion wait devolvio error");
                break;
            }
        break; 

    case DESALOJO_POR_SIGNAL:
        recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);
        log_info(logger, "PID: %u solicita un SIGNAL del recurso: %s", pcb_dispatch->PID, recurso_solicitado );
        
        switch(signal_recursos (recurso_solicitado,pcb_dispatch->PID)){
            case 1:
                log_info(logger, "PID: %u hace SIGNAL a un recurso: %s exitosamente", pcb_dispatch->PID, recurso_solicitado);
                respuesta_CPU_recurso(OK);
                enviar_nuevamente_proceso_a_ejecucion(pcb_dispatch);            //SIGNAL EXITOSO, DEVUELVO EL PROCESO A EJECUCION
                break;
            case -1:
                log_info(logger, "Finaliza el proceso PID: %u Motivo: INVALID_RESOURCE: %s", pcb_dispatch->PID, recurso_solicitado);
                log_info(logger, "PID: %u - Cambio de estado EXECUTE-> EXIT", pcb_dispatch->PID);
                respuesta_CPU_recurso(FALLO);
                enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                sem_post(&control_multiprogramacion);
                enviar_siguiente_proceso_a_ejecucion();
                break;
            case -2:
                log_info(logger, "Finaliza el proceso PID: %u Motivo: RECURSO NO ASIGNADO: %s", pcb_dispatch->PID, recurso_solicitado);
                log_info(logger, "PID: %u - Cambio de estado EXECUTE-> EXIT", pcb_dispatch->PID);
                respuesta_CPU_recurso(FALLO);
                enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                sem_post(&control_multiprogramacion);
                enviar_siguiente_proceso_a_ejecucion();
        }
    break;

    case DESALOJO_POR_QUANTUM:
            log_info(logger, "PID: %u - Desalojado por fin de Quantum", pcb_dispatch->PID);
            pcb_dispatch->quantum_ejecutado=0;                                                                  //RESETEO EL CONTADOR Y LO PONGO NUEVAMENTE EN READY
            ingresar_en_lista(pcb_dispatch, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);
            enviar_siguiente_proceso_a_ejecucion();
        break;

    case DESALOJO_POR_FIN_PROCESO:
            log_info(logger, "Finaliza el proceso PID: %u Motivo: SUCCESS", pcb_dispatch->PID);
            log_info(logger, "PID: %u - Cambio de estado EXECUTE-> EXIT", pcb_dispatch->PID);
            //ENVIO PID A MEMORIA PARA QUE ELIMINE EL PROCESO
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
            eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
            sem_post(&control_multiprogramacion);
            enviar_siguiente_proceso_a_ejecucion();

        break;

    case DESALOJO_POR_CONSOLA:
        enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
        eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
        sem_post(&control_multiprogramacion);
        break;
    case DESALOJO_POR_IO_GEN_SLEEP:
    case DESALOJO_POR_IO_STDIN:        
    case DESALOJO_POR_IO_STDOUT:
    case DESALOJO_POR_IO_FS_CREATE:
    case DESALOJO_POR_IO_FS_DELETE:
    case DESALOJO_POR_IO_FS_TRUNCATE:
    case DESALOJO_POR_IO_FS_WRITE:
    case DESALOJO_POR_IO_FS_READ:
            char* nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u envia peticion a interfaz %s", pcb_dispatch->PID, nombre_interfaz);

            //hay que replantearlo con multiplexacion
            t_paquete *paquete = crear_paquete(cod_op);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            agregar_a_paquete_string(paquete, size-desplazamiento, buffer+desplazamiento);//Serializa el resto del buffer en el nuevo paquete, lo probe y *PARECE* funcionar, sino hay que hacer otra funcion
            enviar_paquete(paquete, socket_entradasalida_kernel);
            eliminar_paquete(paquete);
            //
            if(strcmp(algoritmo_planificacion,"VRR")==0){ 
                ingresar_en_lista(pcb_dispatch, lista_ready_prioridad, &semaforo_ready_prioridad, &cantidad_procesos_en_algun_ready , READY_PRIORITARIO);  
            ///esto del ingreso a la lista de todoslos procesos que soliciten IO no estoy seguro
            }else{
                ingresar_en_lista(pcb_dispatch, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);  

            }
            enviar_siguiente_proceso_a_ejecucion();            
            break;
    default:
        log_warning(logger_debug,"Operacion desconocida para Kernel al recibir de socket CPU-Dispatch.");
        break;
    }   


    free(pcb_dispatch);	
    free(buffer);
	
}
}


void enviar_siguiente_proceso_a_ejecucion ()               
{
sem_wait(&cantidad_procesos_en_algun_ready);                                 // SI NO HAY NINGUN PROCESO EN READY.... ESPERO QUE SE ENCOLE ALGUNO EN READY O READY PRIORIDAD
    t_pcb* pcb_a_ejecutar;

    if (list_size(lista_ready_prioridad)>0)
    {   
        pthread_mutex_lock(&semaforo_ready_prioridad);
        pcb_a_ejecutar = list_remove(lista_ready_prioridad, 0);
        pthread_mutex_unlock(&semaforo_ready_prioridad);
    }
    else{
        pthread_mutex_lock(&semaforo_ready);
        pcb_a_ejecutar = list_remove(lista_ready, 0);
        pthread_mutex_unlock(&semaforo_ready);
    
    }
    
    if(strcmp(algoritmo_planificacion,"VRR")==0 || strcmp(algoritmo_planificacion,"RR")==0){       //CREO HILO DE DESALOJO SI CORRESPONDIERA
        int64_t* quantum_ptr = malloc(sizeof(int64_t));                                                    //LA UNICA DIFERENCIA ENTRE EL FIFO Y EL RR ES EL DESALOJO POR QUANTUM
        if (quantum_ptr == NULL) {
            perror("malloc");
            return;
        }
        backup_de_quantum_ejecutado= pcb_a_ejecutar->quantum_ejecutado;
        pcb_actual_en_cpu=pcb_a_ejecutar->PID;

        *quantum_ptr = pcb_a_ejecutar->quantum_ejecutado;

        pthread_t hilo_de_desalojo_por_quantum;                
        pthread_create(&hilo_de_desalojo_por_quantum, NULL,(void*) interruptor_de_QUANTUM, quantum_ptr); 
        pthread_detach(hilo_de_desalojo_por_quantum);
    }

    
    log_info(logger_debug, "Se mando a CPU para ejecutar el proceso PID:  %u, planificado por '%s' \n", pcb_a_ejecutar->PID,algoritmo_planificacion);
    enviar_CE(socket_kernel_cpu_dispatch, pcb_a_ejecutar->PID,pcb_a_ejecutar->CE);     
        
    free(pcb_a_ejecutar);
}



void interruptor_de_QUANTUM(void* quantum_de_pcb)
{
    int64_t quantumEjecutado = *((int64_t*)quantum_de_pcb);
    int64_t quantum_ejecucion=quantum - quantumEjecutado;
    
    log_trace(logger_debug,"El tiempo a ejecutar registrado es %ld",quantum_ejecucion);
    
    temporizador= temporal_create();

    usleep(quantum_ejecucion*1000);
 

    op_code interrupcion = INT_QUANTUM;
    send(socket_kernel_cpu_interrupt, &interrupcion, sizeof(op_code), 0);

    free(quantum_de_pcb);

}


void enviar_nuevamente_proceso_a_ejecucion(t_pcb* pcb_a_reenviar){                         //ESTA FUNCION ES PARA CUANDO SE SOLICITA UN RECURSO Y PUEDE SEGUIR EJECUTANDO


    uint32_t* quantum_ptr = malloc(sizeof(int));                                                    
    if (quantum_ptr == NULL) {
        perror("malloc");
        return;
    }
    *quantum_ptr = pcb_a_reenviar->quantum_ejecutado;
    pthread_t hilo_de_desalojo_por_quantum;                
    pthread_create(&hilo_de_desalojo_por_quantum, NULL,(void*) interruptor_de_QUANTUM, quantum_ptr); 
    pthread_detach(hilo_de_desalojo_por_quantum);
    

    enviar_CE(socket_kernel_cpu_dispatch, pcb_a_reenviar->PID,pcb_a_reenviar->CE);     
    log_info(logger_debug, "Se gestiono el recurso, y se envio nuevamente a CPU a ejecutar el proceso PID:  %u\n", pcb_a_reenviar->PID);
    
}




void respuesta_CPU_recurso(op_code respuesta)//Envia solo el op_code, si hace falta se puede pasar a conexiones.h
{
    op_code a_enviar = respuesta;
    send(socket_kernel_cpu_interrupt, &a_enviar, sizeof(op_code), 0);
}



