#include "../include/CPU-kernel.h"

void recibir_proceso(){
    op_code op = recibir_operacion(socket_cpu_kernel_dispatch);
    if (op!=CONTEXTO)
    {
        log_warning(log_debug, "Se esperaba un contexto de ejecucion y llego otra cosa, codigo: %d", op);
    }
    
    recibir_CE(socket_cpu_kernel_dispatch, &PID, &contexto_interno);
    log_info(logger, "Llega un proceso de PID: %u", PID);
    interrupcion = INT_NO;
}

void desalojar_proceso(op_code motivo_desalojo){
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
}

void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1)
{
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
};

void enviar_CE_con_2_arg(op_code motivo_desalojo, char* arg1, char* arg2)
{
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    agregar_a_paquete_string(paquete, strlen(arg2) + 1, arg2);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
};

void enviar_CE_con_5_arg(op_code motivo_desalojo, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5)
{
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    agregar_a_paquete_string(paquete, strlen(arg2) + 1, arg2);
    agregar_a_paquete_string(paquete, strlen(arg3) + 1, arg3);
    agregar_a_paquete_string(paquete, strlen(arg4) + 1, arg4);
    agregar_a_paquete_string(paquete, strlen(arg5) + 1, arg5);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
};

bool esperar_respuesta_recurso()
{
    op_code cod_op = recibir_operacion(socket_cpu_kernel_dispatch);
    switch (cod_op)
    {
    case OK:
    case FALLO:
        return (cod_op == OK);
        break;
    default:
        log_error(logger, "Se esperaba una respuesta a WAIT/SIGNAL de recurso, llego otra cosa");
        return false;
        break;
    }
}

void gestionar_conexion_interrupt()
{
    op_code operacion;
    bool continuar_iterando = true;

    while (continuar_iterando)
    {
        operacion = recibir_operacion(socket_cpu_kernel_interrupt);
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_kernel_interrupt,logger_debug);
            break;
        case DESALOJO_POR_CONSOLA:
            log_info(logger, "El usuario finaliza el proceso PID: %u por consola", PID);
            interrupcion = INT_CONSOLA;
            break;

        case DESALOJO_POR_QUANTUM:
            log_info(logger, "El proceso PID: %u termino su quantum y es desalojado", PID);
            interrupcion = INT_QUANTUM;
            break;
        case FALLO:
            log_error(logger, "Kernel desconectado, terminando servidor INTERRUPT");
            continuar_iterando = false;
       
        default:
            log_error(logger, "Llego algo que no era interrupcion por socket interrupt, op_code: %d", operacion);
            continuar_iterando = false;
            break;
        }
    }
    
}