
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
		log_info(logger,"Cola Ready %s : %s",algoritmo_planificacion, log_cola_ready);
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
free(estado_nuevo_string);    
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
    int cod_op = recibir_operacion(socket_kernel_cpu_dispatch);
    
    if ((strcmp(algoritmo_planificacion,"VRR")==0 ||strcmp(algoritmo_planificacion,"RR")==0 ) && temporizador!=NULL)
    {
        tiempo_ejecucion= temporal_gettime(temporizador); //esto hay que ponerlo 
        temporal_destroy(temporizador);
        pthread_cancel(hilo_de_desalojo_por_quantum);
    }


    switch (motivo_desalojo)
    {
    case PAGE_FAULT: 
        
        break;
    case OUT_OF_MEMORY:
        
        break;
    case MENSAJE:
        
        break;
    case DESALOJO_POR_WAIT:
         
        break;
    case DESALOJO_POR_SIGNAL:
        
        break;
    case DESALOJO_POR_QUANTUM:
        
        break;
    case DESALOJO_POR_FIN_PROCESO:
        
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
	
	
	
}

void enviar_proceso_a_ejecucion ()
{
sem_wait(&cantidad_procesos_en_algun_ready);                                 // SI NO HAY NINGUN PROCESO EN READY ESPERO QUE SE ENCOLE ALGUNO EN READY O READY PRIORIDAD
    
    if (list_size(lista_ready_prioridad)>0)
    {   
        pthread_mutex_lock(&semaforo_ready_prioridad);
        t_pcb* pcb_a_ejecutar = list_remove(lista_ready_prioridad, 0);
        pthread_mutex_unlock(&semaforo_ready_prioridad);
    }
    else{
        pthread_mutex_lock(&semaforo_ready);
        t_pcb* pcb_a_ejecutar = list_remove(lista_ready, 0);
        pthread_mutex_unlock(&semaforo_ready);
    
    }

    if(strcmp(algoritmo_planificacion,"VRR")==0 || strcmp(algoritmo_planificacion,"RR")==0){       //CREO HILO DE DESALOJO SI CORRESPONDIERA
                
        pthread_create(&hilo_de_desalojo_por_quantum,NULL,(void*) controlador_de_QUANTUM,NULL);
        pthread_detach(hilo_de_desalojo_por_quantum);  
    }
    

}


void controlador_de_QUANTUM()
{
    t_temporal * temporizador= temporal_create();
    usleep(quantum *1000);
    void *interrupcion = (void *)(intptr_t)INT_QUANTUM;
    send(socket_kernel_cpu_interrupt,interrupcion,sizeof(int_code),0);
}
















/*
void* pcp_planificar(void* args)
{

    algoritmo_utilizado = determinar_algoritmo();


    switch (algoritmo_utilizado)
    {
        case FIFO:

            planificador_corto_plazo = list_create();
            while (1)
            {
                planificar_fifo();
                //enviar_CE(socket_kernel_cpu_dispatch,  contexto_actual, CE);
                enviar_CE(socket_kernel_cpu_dispatch, CE, contexto_actual);

                recibir_CE(socket_kernel_cpu_dispatch, CE, contexto_actual);
                log_info(logger, "Contexto de ejecución recibido de CPU");
                modificar_pcb(proceso_actual);

                //operar_desalojo(proceso_actual);
            }
            break;

        case RR:

            planificador_corto_plazo = list_create();
            int64_t quantum = config_get_int_value(config, "QUANTUM");
            pthread_t hilo_cronometro;

            while (1)
            {
                planificar_rr();
                pthread_create(&hilo_cronometro, NULL, cronometrar, &quantum);
                enviar_CE(conexion_CPU_DISPATCH, CE, contexto_actual);

                recibir_CE(conexion_CPU_DISPATCH, CE, contexto_actual);
                log_info(logger, "Contexto de ejecución recibido de CPU");
                modificar_pcb(proceso_actual);

                //operar_desalojo(proceso_actual);

                pthread_cancel(hilo_cronometro);
            }
            break;

        default:
            break;
    }
}

void planificar_fifo(){
    sem_wait(&controlador_pcp);
    proceso_actual = list_remove(planificador_corto_plazo, 0);
    cambiar_estado(proceso_actual, EJECUCION);
    log_info(logger, "Proceso %d cambia de estado a EJECUCION", proceso_actual->PID);
    contexto_actual = *obtener_contexto_ejecucion(proceso_actual);
    log_info(logger, "Contexto de ejecución del proceso %d enviado a CPU", proceso_actual->PID);

}

t_contexto_ejecucion* obtener_contexto_ejecucion(t_pcb* proceso){
    t_contexto_ejecucion* auxiliar;
    auxiliar-> PC = proceso->CE.PC;
    auxiliar -> AX = proceso->CE.AX;
    auxiliar -> BX = proceso->CE.BX;
    auxiliar ->CX = proceso->CE.CX;
    auxiliar ->DX = proceso->CE.DX;
    auxiliar ->EAX = proceso->CE.EAX;
    auxiliar ->EBX = proceso->CE.EBX;
    auxiliar ->ECX = proceso->CE.ECX;
    auxiliar ->EDX = proceso->CE.EDX;
    auxiliar ->SI = proceso->CE.SI;
    auxiliar ->DI = proceso->CE.DI;
    return auxiliar;
}


void planificar_rr(){

    sem_wait(&controlador_pcp);
    sem_post(&controlador_pcp);
    sem_wait(&proceso_ready);
    proceso_actual = list_remove(planificador_corto_plazo, 0);
    cambiar_estado(proceso_actual, EJECUCION);
    log_info(logger, "Proceso %d cambia de estado a EJECUCION", proceso_actual->PID);
    contexto_actual = *obtener_contexto_ejecucion(proceso_actual);
    log_info(logger, "Contexto de ejecución del proceso %d enviado a CPU", proceso_actual->PID);

}

void cambiar_estado(t_pcb* proceso, t_estado nuevo_estado)
{
    proceso->estado = nuevo_estado;
}

t_algoritmo_planificacion determinar_algoritmo()
{
    char* cadena_aux = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (string_equals_ignore_case(cadena_aux, "FIFO"))
    {
        return FIFO;
    } else if (string_equals_ignore_case(cadena_aux, "RR"))
    {
        return RR;
    } else {return  ERROR;}
}


void* cronometrar(void* args)
{
    int* ptr = (int*) args;
    usleep((*ptr) * 1000);
    return NULL;
}
*/