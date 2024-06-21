#include "../include/CPU-memoria.h"

t_instruccion* fetch(uint32_t PID, uint32_t PC, int32_t socket_cpu_memoria){
    t_paquete* p = crear_paquete(FETCH);
    agregar_a_paquete_uint32(p, PID);
    agregar_a_paquete_uint32(p, PC);
    enviar_paquete(p, socket_cpu_memoria);
    eliminar_paquete(p);

    return recibir_instruccion(socket_cpu_memoria);
}

t_instruccion* recibir_instruccion(int32_t socket_cpu_memoria){
    op_code op = recibir_operacion(socket_cpu_memoria);
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    if (op != FETCH){
        log_error(logger, "Llego otra cosa en lugar de una instruccion, codigo:%d", op);
        instr->ins = EXIT;
        return instr;
    }

    
    uint32_t size;
    int32_t desplazamiento = 0;
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

void recibir_tamanio_de_pagina()
{
    op_code codigo = recibir_operacion(socket_cpu_memoria);
    if (codigo != TAM_PAG){
        log_error(logger, "Llego otra cosa en lugar del tamaño maximo de pagina, codigo: %d", codigo);
        tamanio_de_pagina = 0;
    }
    else
    {
        uint32_t size;
        int32_t desplazamiento = 0;
        void* buffer = recibir_buffer(&size, socket_cpu_memoria);

        tamanio_de_pagina = leer_de_buffer_uint32(buffer, &desplazamiento);
        log_info(logger, "Llego el tamanio de pagina: %d", tamanio_de_pagina);
        free(buffer);
    }

}
