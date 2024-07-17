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
        operacion = recibir_operacion(socket_cpu_kernel_dispatch);
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_kernel_dispatch,logger_debug);
            break;
        case CONTEXTO:
            t_contexto_ejecucion contexto_espera;
            
            recibir_CE(socket_cpu_kernel_dispatch, &PID, &contexto_espera);
            log_trace(logger, "Llega un proceso de PID: %u, esperando CPU", PID);
            
            sem_wait(&espera_iterador);
            log_trace(logger, "Se carga nuevo contexto de ejecucion");
            
            interrupcion = INT_NO;
            detener_ejecucion=false;                                         ///ESTE SEMAFORO LO PUSE PARA SINCRONIZAR WL WHILW(1) CON RECIBIR MENSAJE
            contexto_interno.PC = contexto_espera.PC;
            contexto_interno.AX = contexto_espera.AX;
            contexto_interno.BX = contexto_espera.BX;
            contexto_interno.CX = contexto_espera.CX;
            contexto_interno.DX = contexto_espera.DX;
            contexto_interno.EAX = contexto_espera.EAX;
            contexto_interno.EBX = contexto_espera.EBX;
            contexto_interno.ECX = contexto_espera.ECX;
            contexto_interno.EDX = contexto_espera.EDX;
            contexto_interno.SI = contexto_espera.SI;
            contexto_interno.DI = contexto_espera.DI;
            sem_post(&hay_proceso_ejecutando);

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
    detener_ejecucion=true;
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %s", PID, codigo_operacion_string(motivo_desalojo));
    
    
}

void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1)
{
    detener_ejecucion=true;
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    
    
};

void enviar_CE_con_2_arg(op_code motivo_desalojo, char* arg1, char* arg2)
{
    detener_ejecucion=true;
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    agregar_a_paquete_string(paquete, strlen(arg2) + 1, arg2);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);    
};

void solicitar_IO_GEN_SLEEP(op_code motivo_desalojo, char* nombre_interfaz, uint32_t unidades_trabajo)
{
    detener_ejecucion=true;
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(nombre_interfaz) + 1, nombre_interfaz);
    agregar_a_paquete_uint32(paquete, unidades_trabajo);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);    
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
            recibir_de_buffer_solo_PID(socket_cpu_kernel_interrupt);
            if (!detener_ejecucion)
            {
                log_info(logger, "El usuario finaliza el proceso PID: %u por consola", PID);
                interrupcion = INT_CONSOLA;                
            }else{
                log_warning(logger_debug,"Llego una interrupcion por consola mientras no se estaba ejecutando");
            }
            break;

        case DESALOJO_POR_QUANTUM:
            recibir_de_buffer_solo_PID(socket_cpu_kernel_interrupt);
            if (!detener_ejecucion )
            {
                log_info(logger, "El proceso PID: %u termino su quantum sera desalojado", PID);
                interrupcion = INT_QUANTUM;
            }else{
                log_warning(logger_debug,"Llego una interrupcion de quantum mientras no se estaba ejecutando");
            }
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