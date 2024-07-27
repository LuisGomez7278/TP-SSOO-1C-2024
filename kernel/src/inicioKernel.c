#include "../include/inicioKernel.h"


void iniciar_Kernel(char* parametros){

    iniciar_logs();
	iniciar_configs(parametros);
	//imprimir_configs();
	iniciar_estructuras_planificacion();
}


//-------------------------------------------FUNCIONES SECUNDARIAS PARA INICIAR MODULO-------------------------------------------------------//


void iniciar_logs()
{	
	logger = start_logger("log_kernel.log", "LOG KERNEL", LOG_LEVEL_TRACE);
	if(logger==NULL){
		perror("No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}
	logger_debug= log_create("log_kernel_debug.log", "KERNEL_DB_LOG", true, LOG_LEVEL_TRACE);
	if(logger_debug==NULL){
		perror("No se pudo crear el logger debug");
		exit(EXIT_FAILURE);
	}
	
}

void iniciar_configs(char* parametros){
    config_conexiones = start_config("./kernel.config");

    if(config_conexiones==NULL){
        perror("Fallo al crear el archivo config conexiones");
        exit(EXIT_FAILURE);
    }
    char* path_parametros = string_duplicate("./configs/");
    string_append(&path_parametros, parametros);
    string_append(&path_parametros, ".cfg");

    config_parametros = start_config(path_parametros);
	if(config_parametros==NULL){
		perror("No se pudo crear la config parametros");
		exit(EXIT_FAILURE);
	}
    free(path_parametros);



    ip_cpu 							 = config_get_string_value(config_conexiones, "IP_CPU");
    puerto_cpu_dispatch				 = config_get_string_value(config_conexiones, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt 			 = config_get_string_value(config_conexiones, "PUERTO_CPU_INTERRUPT");
    ip_memoria 						 = config_get_string_value(config_conexiones, "IP_MEMORIA");
    puerto_memoria 					 = config_get_string_value(config_conexiones, "PUERTO_MEMORIA");
    puerto_escucha 					 = config_get_string_value(config_conexiones, "PUERTO_ESCUCHA");
	algoritmo_planificacion			 = config_get_string_value(config_parametros,"ALGORITMO_PLANIFICACION");
	quantum							 = config_get_int_value(config_parametros,"QUANTUM");
	recursos						 = config_get_array_value(config_parametros,"RECURSOS");
	instancias_recursos				 = config_get_array_value(config_parametros,"INSTANCIAS_RECURSOS");
	grado_multiprogramacion			 = config_get_int_value(config_parametros,"GRADO_MULTIPROGRAMACION");
	path_de_comandos_base			 = config_get_string_value(config_parametros,"PATH_COMANDOS");
	instancias_recursos_int			 = convertir_a_enteros_la_lista_de_instancias(instancias_recursos);
	

	construir_lista_de_recursos(); 
	
	imprimir_recursos();

}



void iniciar_estructuras_planificacion(){

	

    lista_new = list_create();
	lista_ready = list_create();
	lista_ready_prioridad = list_create();
	lista_exit = list_create();
	lista_bloqueado= list_create();
	lista_bloqueado_prioritario= list_create();
	lista_de_interfaces= list_create();
	
//SEMAFORO MULTIPROGRAMACION

    sem_init(&control_multiprogramacion, 0, grado_multiprogramacion);
	       
	
	
// SEMAFOROS AUXILIARES 

    sem_init(&cantidad_procesos_new, 0, 0);
    sem_init(&cantidad_procesos_ready, 0, 0);
    sem_init(&cantidad_procesos_ready_prioritario, 0, 0);
	sem_init(&cantidad_procesos_en_algun_ready, 0, 0);
	sem_init(&cantidad_procesos_bloqueados, 0, 0);
	sem_init(&semaforo_plp, 0, 0);
	sem_init(&semaforo_pcp, 0, 0);

//	sem_init(&ocupacion_cpu, 0, 1);
	




//MUTEX PARA MANIPULACION SEGURA DE LISTAS 

    pthread_mutex_init(&semaforo_new, NULL);
    pthread_mutex_init(&semaforo_ready, NULL);
	pthread_mutex_init(&semaforo_bloqueado, NULL);
	pthread_mutex_init(&semaforo_bloqueado_prioridad, NULL);
	pthread_mutex_init(&semaforo_ready_prioridad, NULL);
	pthread_mutex_init(&semaforo_recursos, NULL);
	pthread_mutex_init(&semaforo_lista_interfaces, NULL);
//MUTEX PARA PLANIFICADOR LARGO Y CORTO PLAZO	
	pthread_mutex_init(&mutex_cont_plp, NULL);
	pthread_mutex_init(&mutex_cont_pcp, NULL);

//mutex asignacion pid
	pthread_mutex_init(&mutex_pid, NULL);


}

