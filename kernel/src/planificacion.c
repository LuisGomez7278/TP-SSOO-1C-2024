
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

	if(strcmp(estado_nuevo_string, "READY")==0){
		char* log_cola_ready = string_new();
		string_append(&log_cola_ready, "[");
		for(int i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready, ", ");
			}
		}
		string_append(&log_cola_ready, "]");
		log_info(logger,"Cola Ready %s : %s \n",algoritmo_planificacion, log_cola_ready);
		free(log_cola_ready);
	}
    
    if(strcmp(estado_nuevo_string, "READY_PRIORITARIO")==0){
		char* log_cola_ready_prioritario = string_new();
		string_append(&log_cola_ready_prioritario, "[");
		for(int i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready_prioritario, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready_prioritario, ", ");
			}
		}


		string_append(&log_cola_ready_prioritario, "]");
		log_info(logger,"Cola Ready Prioritario %s : %s",algoritmo_planificacion, log_cola_ready_prioritario);
		free(log_cola_ready_prioritario);
	}
pthread_mutex_unlock(semaforo_mutex);
}



void loggeo_de_cambio_estado(uint32_t pid, t_estado viejo, t_estado nuevo){
    	log_info(logger, "PID: %d - Cambio de estado %s -> %s", pid, codigo_estado_string(viejo), codigo_estado_string(nuevo));
}





void cambiar_grado_multiprogramacion(int nuevo_valor) {                         // CON ESTA FUNCION AGREGO O DISMINUYO INSTANCIAS DEL SEMAFORO QUE GESTIONA LA MULTIPROGRAMCION 
    int actual_valor;                                                           // SIN PERDER LA INFORMACION DE LOS QUE YA ESTAN EN LA COLA DE WAIT
    sem_getvalue(&control_multiprogramacion, &actual_valor);
    
    if (nuevo_valor > actual_valor) {
        // Incrementar el semáforo
        for (int i = 0; i < nuevo_valor - actual_valor; i++) {
            sem_post(&control_multiprogramacion);                   //Suma instancias al semaforo
        }
    } else if (nuevo_valor < actual_valor) {
        // Decrementar el semáforo
        for (int i = 0; i < actual_valor - nuevo_valor; i++) {
            sem_wait(&control_multiprogramacion);                   //Resta instancias al semaforo
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        /////////////////////////////////////////////////////       PLANIFICADOR CORTO PLAZO               //////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------






void gestionar_dispatch (op_code motivo_desalojo , t_pcb PCB_desalojado, void* serializado_para_IO){ //esta funcion va con un while(1) abajo del recibe
    
    op_code cod_op = recibir_operacion(socket_kernel_cpu_dispatch);
    
    
    if ((strcmp(algoritmo_planificacion,"VRR")==0 ||strcmp(algoritmo_planificacion,"RR")==0 ) && temporizador!=NULL)
    {
        tiempo_ejecucion= temporal_gettime(temporizador); //esto hay que ponerlo 
        temporal_destroy(temporizador);
        pthread_cancel(hilo_de_desalojo_por_quantum);
    }

    //////////////////////////////   ACA HAY QUE DESERIALIZAR  SEGUN CORRESPONDA CADA FUNCION suponemos que el pcb extraido se llama: pcb_dispatch
                                // LO DECLARO PARA QUE NO ME ARROJE ERRORES

    t_pcb *pcb_dispatch=malloc(sizeof(t_pcb));

    switch (cod_op)
    {

    case PAGE_FAULT: 
        
        break;
    case OUT_OF_MEMORY:
        
        break;
    case DESALOJO_POR_WAIT:
         
        break;
    case DESALOJO_POR_SIGNAL:
        
        break;
    case DESALOJO_POR_QUANTUM:
        
        break;
    case DESALOJO_POR_FIN_PROCESO:
            op_code codigo_de_operacion=ELIMINAR_PROCESO;
            //ENVIO PID A MEMORIA PARA QUE ELIMINE EL PROCESO
            enviar_instruccion_con_PID_por_socket(codigo_de_operacion,pcb_dispatch->PID,socket_memoria_kernel);


        break;
    case DESALOJO_POR_CONSOLA:
        
        break;
    case DESALOJO_POR_IO_GEN_SLEEP:
                              
    case DESALOJO_POR_IO_STDIN:        
    case DESALOJO_POR_IO_STDOUT:
    case DESALOJO_POR_IO_FS_CREATE:
    case DESALOJO_POR_IO_FS_DELETE:
    case DESALOJO_POR_IO_FS_TRUNCATE:
    case DESALOJO_POR_IO_FS_WRITE:
    case DESALOJO_POR_IO_FS_READ:
        break;        

    default:
        break;
    }

enviar_siguiente_proceso_a_ejecucion();	
free(pcb_dispatch);	
	
}



void enviar_siguiente_proceso_a_ejecucion ()    
        ///   
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
        int* quantum_ptr = malloc(sizeof(int));                                                    //LA UNICA DIFERENCIA ENTRE EL FIFO Y EL RR ES EL DESALOJO POR QUANTUM
        if (quantum_ptr == NULL) {
            perror("malloc");
            return;
        }
        *quantum_ptr = pcb_a_ejecutar->quantum_restante;

        pthread_t hilo_de_desalojo_por_quantum;                
        pthread_create(&hilo_de_desalojo_por_quantum, NULL,(void*) interruptor_de_QUANTUM, quantum_ptr); 
        pthread_detach(hilo_de_desalojo_por_quantum);
    }
    pcb_actual_en_cpu=pcb_a_ejecutar->PID;
    pcb_a_ejecutar->estado=EXEC;
    log_info(logger_debug, "Se mando a CPU para ejecutar el proceso PID:  %u\n", pcb_a_ejecutar->PID);
    enviar_CE(socket_kernel_cpu_dispatch, pcb_a_ejecutar->PID,pcb_a_ejecutar->CE);     
    sem_post(&cantidad_procesos_en_algun_ready);
}


void interruptor_de_QUANTUM(void* quantum_de_pcb)
{
    int quantumRestante = *((int*)quantum_de_pcb);
    temporizador= temporal_create();
    int quantum_ejecucion=(quantum*1000)-quantumRestante;

    log_trace(logger_debug,"El tiempo de espera registrado es %d\n",quantum_ejecucion);
    
    usleep(quantum_ejecucion);
    void *interrupcion = (void *)(intptr_t)INT_QUANTUM;
    send(socket_kernel_cpu_interrupt,interrupcion,sizeof(int_code),0);
    free(quantum_de_pcb);
}










