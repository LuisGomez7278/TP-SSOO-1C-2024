#include "../include/CPU-kernel.h"

void gestionar_conexion_dispatch()
{
    //ENVIO MENSAJE A KERNEL
    enviar_mensaje("CONEXION CON CPU-DISPATCH OK", socket_cpu_kernel_dispatch);
    log_info(logger, "Handshake enviado: KERNEL");
    
    op_code operacion;
    bool continuar_iterando = true;

    while (continuar_iterando)
    {
        log_trace(logger, "CPU esta esperando un proceso...");
        operacion = recibir_operacion(socket_cpu_kernel_dispatch);
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_kernel_dispatch,logger_debug);
            break;
        case CONTEXTO:
            recibir_CE(socket_cpu_kernel_dispatch, &PID, &contexto_interno);
            log_info(logger, "Llega un proceso de PID: %u", PID);
            sem_post(&hay_proceso_ejecutando);
            interrupcion = INT_NO;
            break;
        
        case FALLO:
            log_error(logger_debug, "Kernel desconectado, terminando servidor DISPATCH");
            continuar_iterando = false;
            break;

        default:
            log_warning(logger_debug, "Llego algo que no es contexto por DISPATCH, codigo: %d", operacion);
            break;
        }
    }
}

void desalojar_proceso(op_code motivo_desalojo){
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %d", PID, motivo_desalojo);
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    sem_wait(&hay_proceso_ejecutando);
}

void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1)
{
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    sem_wait(&hay_proceso_ejecutando);
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
    sem_wait(&hay_proceso_ejecutando);
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
    sem_wait(&hay_proceso_ejecutando);
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
            log_info(logger, "El proceso PID: %u termino su quantum y debe ser desalojado", PID);
            interrupcion = INT_QUANTUM;
            break;

        case FALLO:
            log_error(logger, "Kernel desconectado, terminando servidor INTERRUPT");
            continuar_iterando = false;
            break;
       
        default:
            log_warning(logger_debug, "Llego algo que no era interrupcion por socket interrupt, op_code: %d", operacion);
            break;
        }
    }
    
}