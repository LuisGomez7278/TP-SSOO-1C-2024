#include "../include/CPU-memoria.h"

t_instruccion* fetch(uint32_t PID, uint32_t PC, int socket_cpu_memoria){
    t_paquete* p = crear_paquete(FETCH);
    agregar_a_paquete_uint32(p, PID);
    agregar_a_paquete_uint32(p, PC);
    enviar_paquete(p, socket_cpu_memoria);
    eliminar_paquete(p);

    return recibir_instruccion(socket_cpu_memoria);
}

t_instruccion* recibir_instruccion(int socket_cpu_memoria){
    op_code op = recibir_operacion(socket_cpu_memoria);
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    if (op != FETCH){
        log_error(logger, "Llego otra cosa en lugar de una instruccion, codigo:%d", op);
        instr->ins = EXIT;
        return instr;
    }

    
    uint32_t size;
    int desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    instr->ins = leer_de_buffer_cod_ins(buffer, &desplazamiento);
    instr->arg1 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg2 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg3 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg4 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg5 = leer_de_buffer_string(buffer, &desplazamiento);

    free(buffer);

    return instr;
}