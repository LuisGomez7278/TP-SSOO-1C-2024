#include "../include/inicioMemoria.h"

void iniciar_memoria(){
    //INICIALIZO LOGGER
    logger = start_logger("log_memoria.log", "LOG MEMORIA", LOG_LEVEL_INFO);
    logger_debug = start_logger("log_memoria_debug.log", "LOG MEMORIA DEBUG", LOG_LEVEL_TRACE);
      
    //INICIALIZO config
    config = start_config("./memoria.config");

    //OBTENER VALORES CONFIG
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    log_info(logger, "PUERTO leido: %s", puerto_escucha);

    path_base = config_get_string_value(config, "PATH_INSTRUCCIONES");

    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    log_info(logger, "TAMANIO MEMORIA: %d", tam_memoria);

    tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    log_info(logger, "TAMANIO PAGINA: %d", tam_pagina);

    retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    log_info(logger, "RETARDO RESPUESTA: %d", retardo);
}
