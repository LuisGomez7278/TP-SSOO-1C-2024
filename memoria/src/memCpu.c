#include "../include/memCpu.h"

void conexion_con_cpu(int socket_cpu_memoria){
    op_code codigo;

    while(true){
        codigo = recibir_operacion(socket_cpu_memoria);
        switch (codigo)
        {
        case FETCH:
            fetch(socket_cpu_memoria);
            break;
        
        default:
            break;
        }
    }
}
void fetch(int socket_cpu_memoria){
    uint32_t PID; //por ahora no hace nada, sera relevante cuando lleguen varios procesos por kernel 
    uint32_t PC;
    recibir_fetch(socket_cpu_memoria, PID, PC);
    log_info(logger, "CPU solicita instruccion, PID: %d, PC: %d", PID, PC);

    t_list* lista_instrucciones = leer_pseudocodigo(path_base);
    t_instruccion* sig_ins = get_ins(lista_instrucciones, PC);
    usleep(retardo);
    enviar_instruccion(socket_cpu_memoria, sig_ins);
    log_info(logger, "instruccion enviada");
}

void recibir_fetch(int socket_cpu_memoria, uint32_t* PID, uint32_t* PC){
    uint32_t size;
    int desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    *PID = leer_de_buffer_uint32(buffer, &desplazamiento);
    *PC = leer_de_buffer_uint32(buffer, &desplazamiento);
}

void enviar_instruccion(int socket_cpu_memoria, t_instruccion* instruccion){
    t_paquete* paquete = crear_paquete(FETCH);
    //log_info(logger, "Paquete creado");
    agregar_a_paquete_cod_ins(paquete, instruccion->ins);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg1) + 1, instruccion->arg1);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg2) + 1, instruccion->arg2);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg3) + 1, instruccion->arg3);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg4) + 1, instruccion->arg4);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg5) + 1, instruccion->arg5);

    enviar_paquete(paquete, socket);
    //log_info(logger, "Paquete enviado");
    eliminar_paquete(paquete);
}