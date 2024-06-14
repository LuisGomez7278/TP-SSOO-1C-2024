#include "../include/entradasalidaMain.h"

int main(int argc, char* argv[]) {

    decir_hola("una Interfaz de Entrada/Salida");


    //INSTANCIO LOGGER
    t_log* logger;
    //INICIALIZO LOGGER
    logger = start_logger("log_entradasalida.log", "LOG ENTRADA SALIDA", LOG_LEVEL_INFO);

    //INSTACION CONFIG
    t_config* config;
    //INICIALIZO config
    config = start_config("./entradasalida.config");

    //Cargo y compruebo la config
    ip_kernel = config_get_string_value(config, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //Pruebas con memoria
    //crear conexion
    socket_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria);
    //recibir mensaje
    op_code codop1 = recibir_operacion(socket_entradasalida_memoria);    
    if (codop1 == MENSAJE) {log_info(logger, "LLego un mensaje");}
    else {log_info(logger, "LLego otra cosa");}
    recibir_mensaje(socket_entradasalida_memoria, logger);
    //enviar mensaje
    enviar_mensaje("entradasalida manda mensaje a Memoria", socket_entradasalida_memoria);
    log_info(logger, "Se envio el primer mensaje a memoria");

    //Pruebas con kernel
    //crear conexion
    socket_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel);
    //recibir mensaje
    op_code codop2 = recibir_operacion(socket_entradasalida_kernel);    
    if (codop2 == MENSAJE) {log_info(logger, "LLego un mensaje");}
    else {log_info(logger, "LLego otra cosa");}
    recibir_mensaje(socket_entradasalida_kernel, logger);
    //enviar mensaje
    enviar_mensaje("entradasalida manda mensaje a kernel", socket_entradasalida_kernel);
    log_info(logger, "Se envio el primer mensaje a kernel");

    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}



    return 0;
}
