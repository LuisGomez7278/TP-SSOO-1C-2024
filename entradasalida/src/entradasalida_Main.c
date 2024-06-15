#include "../include/entradasalida_Main.h"

int main(int argc, char* argv[]) {

    //VALIDO ARGUMENTOS
    validar_argumentos(argv[1],argv[2]);
    
    nombre_interfaz = argv[1];
    config_interfaz = argv[2];

    iniciar_entradasalida(nombre_interfaz, config_interfaz);

    socket_entradasalida_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(logger, "Se creo la conexion entre IO y Memoria");
    socket_entradasalida_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
    log_info(logger, "Se creo la conexion entre IO y Kernel");

    bool continuarIterando = true;
    while (continuarIterando) {
        uint32_t PID;
        char* interfaz_pedida;
        op_code cod_op = recibir_operacion(socket_entradasalida_kernel);   ////se queda esperando en recv por ser bloqueante
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_entradasalida_kernel, logger);
            break;
        case DESALOJO_POR_IO_GEN_SLEEP:
            uint32_t unidades_trabajo;
            uint32_t size;
            void* buffer;
            int desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            interfaz_pedida = leer_de_buffer_string(buffer, &desplazamiento);
            unidades_trabajo = atoi(leer_de_buffer_string(buffer, &desplazamiento));
            
            free(buffer);
            log_info(logger,"Se ha recibido la instruccion de Kernel");
            log_info(logger,"PID: %d - Operacion:IO_GEN_SLEEP", PID);
            if (string_equals_ignore_case(nombre_interfaz, interfaz_pedida))
            {
                usleep(unidades_trabajo);
                log_info(logger,"Se ha ejecutado la instruccion de Kernel");
            }
        case DESALOJO_POR_IO_STDIN:
        case DESALOJO_POR_IO_STDOUT:
        case DESALOJO_POR_IO_FS_CREATE:
            
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

t_instruccion* recibir_instruccion_IO(uint32_t* PID)
{
    uint32_t size;
    void* buffer;
    int desplazamiento = 0;
    buffer = recibir_buffer(&size, socket_entradasalida_kernel);
    *PID = leer_de_buffer_uint32(buffer, &desplazamiento);

    t_instruccion* instruccion = malloc(sizeof(t_instruccion));

    instruccion->ins = leer_de_buffer_cod_ins(buffer, &desplazamiento);
    instruccion->arg1 = leer_de_buffer_string(buffer, &desplazamiento);
    instruccion->arg2 = leer_de_buffer_string(buffer, &desplazamiento);
    instruccion->arg3 = leer_de_buffer_string(buffer, &desplazamiento);
    instruccion->arg4 = leer_de_buffer_string(buffer, &desplazamiento);
    instruccion->arg5 = leer_de_buffer_string(buffer, &desplazamiento);

    log_info(logger, "Recibi instruccion, codigo: %d", instruccion->ins);
    free(buffer);

    return instruccion;
}

void ejecutar_instruccion_IO(t_instruccion* instruccion, uint32_t PID)
{
    cod_ins codigo_instruccion = instruccion->ins;
	switch(codigo_instruccion) {
	case IO_GEN_SLEEP:
        int unidadesDeTrabajo = atoi(instruccion->arg2);
        usleep(unidadesDeTrabajo);
        notificar_kernel(PID);
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

void notificar_kernel(uint32_t PID)
{
    t_paquete* paquete = crear_paquete(FINALIZA_IO);
    agregar_a_paquete_uint32(paquete, PID);
    enviar_paquete(paquete, socket_entradasalida_kernel);
    eliminar_paquete(paquete);
}
