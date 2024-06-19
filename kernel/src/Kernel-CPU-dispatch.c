#include "../include/Kernel-CPU-dispatch.h"
    
//void atender_conexion_CPU_DISPATCH_KERNEL(){ Viejo nombre de la funcion
void gestionar_dispatch (op_code motivo_desalojo , t_pcb PCB_desalojado, void* serializado_para_IO){ //esta funcion va con un while(1) abajo del recibe

    log_info(logger, "Inicio conexion entre CPU y Kernel por dispatch");

    if ((strcmp(algoritmo_planificacion,"VRR")==0 ||strcmp(algoritmo_planificacion,"RR")==0 ) && temporizador!=NULL)
    {
        tiempo_ya_ejecutado= temporal_gettime(temporizador); //recupero el valor antes de eliminar el temporizador
        temporal_destroy(temporizador);
        pthread_cancel(hilo_de_desalojo_por_quantum);
    }

    char* recurso_solicitado = "Esto es una prueba";

    op_code codigo;
    uint32_t size;
    int desplazamiento = 0;
    void* buffer;

    uint32_t PID;
    t_contexto_ejecucion CE;
    char* recurso_solicitado;
    t_pcb pcb_dispatch;

    t_paquete* paquete;// es para los IO, seguro se cambie

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

            char* nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %d envia peticion a interfaz %s", PID, nombre_interfaz);

            //hay que replantearlo con multiplexacion
            paquete = crear_paquete(codigo);
            agregar_a_paquete_uint32(paquete, PID);
            agregar_a_paquete_string(paquete, size-desplazamiento, buffer+desplazamiento);
            enviar_paquete(paquete, socket_entradasalida_kernel);
            eliminar_paquete(paquete);
            //

            pcb_dispatch = pcb_de_cpu(PID, CE);
            enviar_siguiente_proceso_a_ejecucion();            
            break;

        case DESALOJO_POR_WAIT:
            buffer = recibir_buffer(&size, socket_kernel_cpu_dispatch);
            
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            leer_de_buffer_CE(buffer, &desplazamiento, &CE);
            recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);
            pcb_dispatch = pcb_de_cpu(PID, CE);
            log_info(logger, "PID: %d solicita un WAIT del recurso: %s", PID, recurso);

            switch (wait_recursos(recurso_solicitado, pcb_dispatch)) {
                //PCB QUEDO EN COLA DE ESPERA DEL RECURSO
                case 1:
                    log_info(logger, "PID: %d - Bloqueado por recurso: %s", PID, recurso_solicitado);
                    respuesta_CPU_recurso(FALLO);
                    enviar_siguiente_proceso_a_ejecucion();	

                    break;
                //WAIT REALIZADO, DEVOLVER EL PROCESO A EJECUCION
                case 2:
                    log_info(logger, "PID: %d hace WAIT de recurso: %s exitosamente", PID, recurso_solicitado);
                    respuesta_CPU_recurso(OK);
                    enviar_nuevamente_proceso_a_ejecucion(pcb_dispatch,tiempo_ya_ejecutado);

                    break;
                //RECURSO NO ENCONTRADO, ENVIAR PROCESO A EXIT
                case -1:                  
                    log_info(logger, "Finaliza el proceso PID: %d Motivo: INVALID_RESOURCE: %s", PID, recurso_solicitado);
                    respuesta_CPU_recurso(FALLO);
                    enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                    enviar_siguiente_proceso_a_ejecucion();
                    break;
                default:
                    log_error(logger_debug,"La funcion wait devolvio error");
                    break;
                }
            break;

        case DESALOJO_POR_SIGNAL:
            buffer = recibir_buffer(&size, socket_kernel_cpu_dispatch);
            
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            leer_de_buffer_CE(buffer, &desplazamiento, &CE);
            recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);
            pcb_dispatch = pcb_de_cpu(PID, CE);
            log_info(logger, "PID: %d solicita un SIGNAL del recurso: %s", PID, recurso);

            switch(signal_recursos (recurso_solicitado)){
                case 1:
                    log_info(logger, "PID: %d hace SIGNAL a un recurso: %s exitosamente", PID, recurso_solicitado);
                    respuesta_CPU_recurso(OK);
                    enviar_nuevamente_proceso_a_ejecucion(pcb_dispatch,tiempo_ya_ejecutado);            //SIGNAL EXITOSO, DEVUELVO EL PROCESO A EJECUCION
                break;
                case -1:
                    log_info(logger, "Finaliza el proceso PID: %d Motivo: INVALID_RESOURCE: %s", PID, recurso_solicitado);
                    enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                    respuesta_CPU_recurso(FALLO);
                    enviar_siguiente_proceso_a_ejecucion();
                    break;
            }
            break;

        case DESALOJO_POR_QUANTUM:
            log_info(logger, "PID: %d - Desalojado por fin de Quantum", PID);
            pcb_dispatch->quantum_ejecutado=0;                                                                  //RESETEO EL CONTADOR Y LO PONGO NUEVAMENTE EN READY
            ingresar_en_lista(pcb_dispatch, lista_ready, &semaforo_ready, &cantidad_procesos_ready , READY);
            sem_post(&cantidad_procesos_en_algun_ready); 
            sem_post(&cantidad_procesos_ready);
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_FIN_PROCESO:
            log_info(logger, "Finaliza el proceso PID: %d Motivo: SUCCESS: %s", PID);
            //ENVIO PID A MEMORIA PARA QUE ELIMINE EL PROCESO
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_CONSOLA:
            log_info(logger, "Finaliza el proceso PID: %d Motivo: INTERRUPTED_BY_USER: %s", PID);
            //ENVIO PID A MEMORIA PARA QUE ELIMINE EL PROCESO
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case OUT_OF_MEMORY:
            log_info(logger, "Finaliza el proceso PID: %d Motivo: OUT_OF_MEMORY : %s", PID);
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
            enviar_siguiente_proceso_a_ejecucion();
            break;
        default:
            
            break;
        }

    }
}

void respuesta_CPU_recurso(op_code respuesta)
{
    t_paquete* paquete = crear_paquete(respuesta);
    enviar_paquete(paquete, socket_kernel_cpu_dispatch);
    eliminar_paquete(paquete);
}

t_pcb pcb_de_cpu(uint32_t PID, t_contexto_ejecucion CE)
{
    t_pcb* pcb;
    pcb->PID = PID;
    pcb->CE = CE;
    pcb->estado = EXEC;
    pcb->quantum_restante = 0;//por lo que entendi de la implementacion, pero puede que este mal

    return pcb;
}

	