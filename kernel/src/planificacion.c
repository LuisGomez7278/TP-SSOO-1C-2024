//
// SOLAMENTE FIFO Y RR PARA PRIMER CHECK
//
#include "../include/planificacion.h"
/*
void* pcp_planificar(void* args)
{

    algoritmo_utilizado = determinar_algoritmo();


    switch (algoritmo_utilizado)
    {
        case FIFO:

            planificador_corto_plazo = list_create();
            while (1)
            {
                planificar_fifo();
                //enviar_CE(socket_kernel_cpu_dispatch,  contexto_actual, CE);
                enviar_CE(socket_kernel_cpu_dispatch, CE, contexto_actual);

                recibir_CE(socket_kernel_cpu_dispatch, CE, contexto_actual);
                log_info(logger, "Contexto de ejecución recibido de CPU");
                modificar_pcb(proceso_actual);

                //operar_desalojo(proceso_actual);
            }
            break;

        case RR:

            planificador_corto_plazo = list_create();
            int64_t quantum = config_get_int_value(config, "QUANTUM");
            pthread_t hilo_cronometro;

            while (1)
            {
                planificar_rr();
                pthread_create(&hilo_cronometro, NULL, cronometrar, &quantum);
                enviar_CE(conexion_CPU_DISPATCH, CE, contexto_actual);

                recibir_CE(conexion_CPU_DISPATCH, CE, contexto_actual);
                log_info(logger, "Contexto de ejecución recibido de CPU");
                modificar_pcb(proceso_actual);

                //operar_desalojo(proceso_actual);

                pthread_cancel(hilo_cronometro);
            }
            break;

        default:
            break;
    }
}

void planificar_fifo(){
    sem_wait(&controlador_pcp);
    proceso_actual = list_remove(planificador_corto_plazo, 0);
    cambiar_estado(proceso_actual, EJECUCION);
    log_info(logger, "Proceso %d cambia de estado a EJECUCION", proceso_actual->PID);
    contexto_actual = *obtener_contexto_ejecucion(proceso_actual);
    log_info(logger, "Contexto de ejecución del proceso %d enviado a CPU", proceso_actual->PID);

}

t_contexto_ejecucion* obtener_contexto_ejecucion(t_pcb* proceso){
    t_contexto_ejecucion* auxiliar;
    auxiliar-> PC = proceso->CE.PC;
    auxiliar -> AX = proceso->CE.AX;
    auxiliar -> BX = proceso->CE.BX;
    auxiliar ->CX = proceso->CE.CX;
    auxiliar ->DX = proceso->CE.DX;
    auxiliar ->EAX = proceso->CE.EAX;
    auxiliar ->EBX = proceso->CE.EBX;
    auxiliar ->ECX = proceso->CE.ECX;
    auxiliar ->EDX = proceso->CE.EDX;
    auxiliar ->SI = proceso->CE.SI;
    auxiliar ->DI = proceso->CE.DI;
    return auxiliar;
}


void planificar_rr(){

    sem_wait(&controlador_pcp);
    sem_post(&controlador_pcp);
    sem_wait(&proceso_ready);
    proceso_actual = list_remove(planificador_corto_plazo, 0);
    cambiar_estado(proceso_actual, EJECUCION);
    log_info(logger, "Proceso %d cambia de estado a EJECUCION", proceso_actual->PID);
    contexto_actual = *obtener_contexto_ejecucion(proceso_actual);
    log_info(logger, "Contexto de ejecución del proceso %d enviado a CPU", proceso_actual->PID);

}

void cambiar_estado(t_pcb* proceso, t_estado nuevo_estado)
{
    proceso->estado = nuevo_estado;
}

t_algoritmo_planificacion determinar_algoritmo()
{
    char* cadena_aux = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (string_equals_ignore_case(cadena_aux, "FIFO"))
    {
        return FIFO;
    } else if (string_equals_ignore_case(cadena_aux, "RR"))
    {
        return RR;
    } else {return  ERROR;}
}


void* cronometrar(void* args)
{
    int* ptr = (int*) args;
    usleep((*ptr) * 1000);
    return NULL;
}
*/