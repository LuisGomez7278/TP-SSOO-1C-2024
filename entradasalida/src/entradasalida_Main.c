#include "../include/entradasalida_Main.h"

int main(int argc, char* argv[]) {


    //VALIDO ARGUMENTOS
    validar_argumentos(argv[1],argv[2]);
    
    nombre_interfaz = argv[1];
    config_interfaz = argv[2];

    iniciar_entradasalida(nombre_interfaz, config_interfaz);

    socket_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria);
    socket_entradasalida_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);

    bool continuarIterando = true;
    while (continuarIterando) {
        int cod_op = recibir_operacion(socket_entradasalida_kernel);   ////se queda esperando en recv por ser bloqueante
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_entradasalida_kernel,logger);
            break;
        case INSTRUCCION:
            log_info(logger,"Se va a procesar la instruccion de Kernel");
            t_instruccion* instruccion = recibir_instruccion_IO(socket_entradasalida_kernel,logger);
            log_info(logger,"Se ha recibido la instruccion de Kernel");
            ejecutar_instruccion_IO(instruccion);
            log_info(logger,"Se ha ejecutado la instruccion de Kernel");
        case -1:
            log_error(logger, "La ENTRADASALIDA SE DESCONECTO. Terminando servidor");
            continuarIterando=0;
            break;
        default:
            log_warning(logger,"Operacion desconocida de ENTRADA Y SALIDA. No quieras meter la pata");
            break;
        }
    }

    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}

    return 0;
}

void validar_argumentos(char* nombre_interfaz, char* config_interfaz)
{
    if(nombre_interfaz == NULL || config_interfaz == NULL){
        log_error(logger,"Agregar argumentos 'nombre_interfaz' y 'config_interfaz'");
        exit(EXIT_FAILURE);
    }
    if(strcmp(nombre_interfaz,"GENERICA") != 0 && strcmp(nombre_interfaz,"STDOUT") != 0 && strcmp(nombre_interfaz,"STDIN") !=0){
        log_error(logger,"Utilizar 'GENERICA', 'STDOUT' o 'STDIN' como nombre de interfaz");
        exit(EXIT_FAILURE);
    }
}

t_instruccion* recibir_instruccion_IO(int socket_cliente, t_log* logger)
{
    int size;
    void* buffer;
    int* tam_recibido = malloc(sizeof(int));
    buffer = recibir_buffer(&size, socket_cliente);
    t_instruccion* instruccion = deserializar_instruccion(buffer, tam_recibido);
    log_info(logger, "Recibi instruccion");
    free(buffer);
    free(tam_recibido);
}

void ejecutar_instruccion_IO(t_instruccion* instruccion)
{
    cod_ins codigo_instruccion = instruccion->ins;
	switch(codigo_instruccion) {
	case IO_GEN_SLEEP:
        int unidadesDeTrabajo = atoi(instruccion->arg2);
        usleep(unidadesDeTrabajo);
	case IO_STDIN_READ:
        //ejecutar_instruccion_memoria(instruccion);
	break;
	case IO_STDOUT_WRITE:
        //ejecutar_instruccion_memoria(instruccion);
		break;
	default:
		break;
	}
}

