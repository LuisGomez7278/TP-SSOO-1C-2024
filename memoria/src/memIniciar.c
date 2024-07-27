#include "../include/memIniciar.h"

void inciarlogsYsemaforos(){
    //INICIALIZO LOGGER
    logger = start_logger("log_memoria.log", "LOG MEMORIA", LOG_LEVEL_TRACE);
    logger_debug = start_logger("log_memoria_debug.log", "LOG MEMORIA DEBUG", LOG_LEVEL_TRACE);

     pthread_mutex_init(&mutex_tablaDePaginas, NULL);
     pthread_mutex_init(&mutex_procesos, NULL);
     pthread_mutex_init(&mutex_listaDeinstrucciones, NULL);
     pthread_mutex_init(& mutex_bitmap,NULL);
}

void cargarConfig(char* parametros){

    //INICIALIZO config
    config_conexiones = start_config("./memoria.config");
    config_parametros = start_config("./memoria.config");

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

    //OBTENER VALORES CONFIG
    puerto_escucha = config_get_string_value(config_conexiones, "PUERTO_ESCUCHA");
    log_info(logger, "PUERTO leido: %s", puerto_escucha);

    path_base = config_get_string_value(config_conexiones, "PATH_INSTRUCCIONES");
    log_info(logger, "PATH: %s", path_base);

    tam_memoria = config_get_int_value(config_parametros, "TAM_MEMORIA");
    log_info(logger, "TAMANIO MEMORIA: %d", tam_memoria);

    tam_pagina = config_get_int_value(config_parametros, "TAM_PAGINA");
    log_info(logger, "TAMANIO PAGINA: %d", tam_pagina);

    retardo = config_get_int_value(config_parametros, "RETARDO_RESPUESTA");
    log_info(logger, "RETARDO RESPUESTA: %d", retardo);

    cant_frames = tam_memoria/tam_pagina;
}

void inicializarEspacioMem(){
    memoria_usuario = malloc(tam_memoria);

    if(memoria_usuario==NULL){
        perror("Fallo al inicializar memoria de usuario");
        exit(EXIT_FAILURE);
    }

    memset(memoria_usuario, 0, tam_memoria); //INCIALIZA TODA LA MEMORIA EN 0

    log_info(logger, "Memoria de usuario inicializada con %d frames de %d bytes cada uno", cant_frames, tam_pagina);

    int tam_bitarray = (cant_frames + 7) / 8; //TAMAÑO EN BYTES redondeando hacia arriba
    char* bitarray_mem = malloc(tam_bitarray);
    if (bitarray_mem == NULL) {
        log_error(logger_debug, "Fallo asignacion memoria al bitarray");
        exit(EXIT_FAILURE);
    }
    memset(bitarray_mem, 0, tam_bitarray);  // Inicializa el bit array a 0 (todos los marcos libres)
    bitmap = bitarray_create_with_mode(bitarray_mem, tam_bitarray, MSB_FIRST);

    tablaDePaginas = list_create();
    procesos = list_create();
    
    //log_info(logger, "BITMAP inicializado correctamente");
}