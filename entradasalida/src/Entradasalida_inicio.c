#include "../include/entradasalida_inicio.h"

void iniciar_entradasalida(char* nombre_interfaz, char* config_interfaz)
{
    iniciar_logs(nombre_interfaz);
    iniciar_config(config_interfaz);

    sem_init(&respuesta_memoria, 0, 0);
}

void iniciar_logs( char* nombre_interfaz) {
    char* path_log = string_duplicate("log_");
    string_append(&path_log, nombre_interfaz);
    char* nombre_log = string_duplicate(path_log);
    string_append(&path_log, ".log");

    logger = start_logger(path_log, nombre_log, LOG_LEVEL_INFO);

    if (logger == NULL) {
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }

    logger = start_logger("log_debug_E-S.log", "log_debug_E-S", LOG_LEVEL_TRACE);

    if (logger == NULL) {
        perror("No se pudo crear el logger debug");
        exit(EXIT_FAILURE);
    }
    free(path_log);
    free(nombre_log);
}

void iniciar_config(char* config_interfaz)
{   
    char* path_config = string_duplicate("./");
    string_append(&path_config, config_interfaz);
    config = start_config(path_config);
	if(config==NULL){
		perror("No se pudo crear la config");
		exit(EXIT_FAILURE);
        
	}

    TIPO_INTERFAZ = config_get_string_value(config, "TIPO_INTERFAZ");
    cod_interfaz interfaz = get_tipo_interfaz(TIPO_INTERFAZ);

    switch (interfaz)
    {
    case GENERICA:
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");    
        IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        break;
    case STDIN:
    case STDOUT:
        IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
        break;
    case DIALFS:
        IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
        PATH_BASE_DIALFS = config_get_string_value(config, "PATH_BASE_DIALFS");
        BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
        BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
        RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
        break;
    default:
		perror("No se pudo cargar datos de la config");
		exit(EXIT_FAILURE);
        break;
    }
    free(path_config);
}

cod_interfaz get_tipo_interfaz(char* TIPO_INTERFAZ){
    if (string_equals_ignore_case(TIPO_INTERFAZ, "GENERICA")) {return GENERICA;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "STDIN")) {return STDIN;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "STDOUT")) {return STDOUT;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS")) {return DIALFS;}
    else {
        log_error(logger, "Se intento crear una interfaz de un tipo no valido");
        return -1;
    }
}