#include "../include/entradasalida_inicio.h"


void iniciar_entradasalida(void){
    iniciar_logs();
    iniciar_config();

}

void iniciar_logs(void){
    logger = start_logger("log_entadasalida.log", "LOG CPU", LOG_LEVEL_INFO);
	if(logger==NULL){
		perror("No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}
}

void iniciar_config(void){
    config = start_config("./entradasalida.config");
	if(config==NULL){
		perror("No se pudo crear la config");
		exit(EXIT_FAILURE);
	}

    IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    log_info(logger, "IP KERNEL leido: %s", IP_KERNEL);

    PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
    log_info(logger, "PUERTO KERNEL leido: %s", PUERTO_KERNEL);

    IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    log_info(logger, "IP MEMORIA leido: %s", IP_MEMORIA);

    PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    log_info(logger, "PUERTO MEMORIA leido: %s", PUERTO_MEMORIA);
    
    TIPO_INTERFAZ = config_get_string_value(config, "TIPO_INTERFAZ");
    log_info(logger, "TIPO INTERFAZ  leido: %s", TIPO_INTERFAZ);

    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    log_info(logger, "TIEMPO UNIDAD TRABAJO leido: %d", TIEMPO_UNIDAD_TRABAJO);

    PATH_BASE_DIALFS = config_get_string_value(config, "PATH_BASE_DIALFS");
    log_info(logger, "PATH BASE DIALFS leido: %s", PATH_BASE_DIALFS);

    BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
    log_info(logger, "BLOCK SIZE leido: %d", BLOCK_SIZE);

    BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
    log_info(logger, "BLOCK COUNT leido: %d", BLOCK_COUNT);

    RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
    log_info(logger, "RETRASO COMPACTACION leido: %d", RETRASO_COMPACTACION);

}
