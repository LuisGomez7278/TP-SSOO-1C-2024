#include "../include/Memoria-EntradaSalida.h"

void atender_conexion_ENTRADASALIDA_MEMORIA(){


// ESPERO QUE SE CONECTE I/O
    log_trace(logger_debug,"Esperando que se concte I/O");
    socket_entradasalida_memoria=esperar_cliente(socket_escucha,logger_debug);



}