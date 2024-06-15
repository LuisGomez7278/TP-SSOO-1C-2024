#include "../include/Kernel-CPU-dispatch.h"
    
void atender_conexion_CPU_DISPATCH_KERNEL(){
    log_info(logger, "Inicio conexion entre CPU y Kernel por dispatch");

    op_code codigo;

    uint32_t size;
    int desplazamiento = 0;
    void* buffer;
    uint32_t PID;
    t_contexto_ejecucion CE;
    t_paquete* paquete;

    while(true){
        codigo = recibir_operacion(socket_kernel_cpu_dispatch);
        switch (codigo)
        {
        case DESALOJO_POR_IO_GEN_SLEEP:
        case DESALOJO_POR_IO_STDIN:
        case DESALOJO_POR_IO_STDOUT:
        case DESALOJO_POR_IO_FS_CREATE:
        case DESALOJO_POR_IO_FS_DELETE:
        case DESALOJO_POR_IO_FS_TRUNCATE:
        case DESALOJO_POR_IO_FS_WRITE:
        case DESALOJO_POR_IO_FS_READ:
            buffer = recibir_buffer(&size, socket_kernel_cpu_dispatch);
            
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            leer_de_buffer_CE(buffer, &desplazamiento, &CE);

            log_info(logger, "PID: %d envia peticion a IO, codigo: %d", PID, codigo);
            paquete = crear_paquete(codigo);
            agregar_a_paquete_uint32(paquete, PID);
            agregar_a_paquete_string(paquete, size-desplazamiento, buffer+desplazamiento);
            enviar_paquete(paquete, socket_entradasalida_kernel);
            eliminar_paquete(paquete);

            //TODO
            // planificador_corto_plazo(codigo, PID, CE);

            break;
        case DESALOJO_POR_WAIT:

            break;
        case DESALOJO_POR_QUANTUM:

            break;
        case DESALOJO_POR_FIN_PROCESO:

            break;
        case DESALOJO_POR_CONSOLA:

            break;
        case DESALOJO_POR_INTERRUPCION:
        
            break;
        default:
            
            break;
        }

    }
}