#include "../include/atender_operacion.h"

void atender_instruccion_validada(char* leido)
{
    char** array_de_comando= string_split (leido, " ");

    if(strcmp(array_de_comando[0],"EJECUTAR_SCRIPT")==0)//---------------------------------/////////////
    {
        

    }else if (strcmp(array_de_comando[0],"INICIAR_PROCESO")==0)//---------------------------------/////////////
    {
        iniciar_proceso(leido);        


    }else if (strcmp(array_de_comando[0],"FINALIZAR_PROCESO")==0)//---------------------------------/////////////
    {
        uint32_t pid_a_finalizar= atoi (array_de_comando[1]);                          
        finalizar_proceso_con_pid(pid_a_finalizar);
                

    }else if (strcmp(array_de_comando[0],"DETENER_PLANIFICACION")==0)//---------------------------------/////////////
    {
        detener_planificacion=true;
        

    }else if (strcmp(array_de_comando[0],"INICIAR_PLANIFICACION")==0)//---------------------------------/////////////
    {   
        detener_planificacion=false;
        sem_post(&semaforo_pcp);
        sem_post(&semaforo_plp);


    }else if (strcmp(array_de_comando[0],"MULTIPROGRAMACION")==0)//---------------------------------/////////////
    {
        uint32_t valor= atoi (array_de_comando[1]);
        cambiar_grado_multiprogramacion(valor);   
        

    }else if (strcmp(array_de_comando[0],"PROCESO_ESTADO")==0)//---------------------------------/////////////
    {
        imprimir_listas_de_estados(lista_ready,"READY");
        imprimir_listas_de_estados(lista_ready_prioridad,"READY_PRIORITARIO");
        imprimir_listas_de_estados(lista_bloqueado,"BLOCKED");                                  //en bloqueados tambien imprimo los que estan esperando recursos
        imprimir_listas_de_estados(lista_bloqueado_prioritario,"BLOCKED_PRIORITARIO");

    }


}


void iniciar_proceso(char*leido){

    t_pcb *new_pcb= malloc(sizeof(t_pcb));
    new_pcb->PID=asignar_pid();
    new_pcb->estado=NEW; 
    new_pcb->quantum_ejecutado=0;
    new_pcb->CE.PC=0;
    new_pcb->CE.AX=0;
    new_pcb->CE.BX=0;
    new_pcb->CE.CX=0;
    new_pcb->CE.DX=0;
    new_pcb->CE.EAX=0;
    new_pcb->CE.EBX=0;
    new_pcb->CE.ECX=0;
    new_pcb->CE.EDX=0;
    new_pcb->CE.SI=0;
    new_pcb->CE.DI=0;

    ingresar_en_lista(new_pcb, lista_new, &semaforo_new, &cantidad_procesos_new , NEW); //loggeo el cambio de estado, loggeo el proceso si es cola ready/prioritario 
    solicitud_de_creacion_proceso_a_memoria(new_pcb->PID,leido);                // ENVIO A CARGAR EL PROGRAMA A EJECUTAR, YA QUE SUPUESTAMENTE ARRANCA CON UNA PAGINA VACIA
                                                                             // EL SEMAFORO DE CONTROL DE MULTIPROGRAMACION LO COLOCO ANTES DEL PASO A LISTA READY 
}



void finalizar_proceso_con_pid(uint32_t pid_a_finalizar){

eliminar_proceso_de_lista_recursos (pid_a_finalizar);
bool encontrado=false;

if (pid_a_finalizar==pcb_actual_en_cpu){
    encontrado=true;
    op_code a_enviar = INT_CONSOLA;
    send(socket_kernel_cpu_interrupt,&a_enviar,sizeof(int_code),0);
}


 t_pcb* pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_new,pid_a_finalizar);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            encontrado=true;
            if(list_remove_element(lista_new,pcb_a_eliminar)){
                log_info(logger_debug,"Proceso con PID:%u eliminado de la lista NEW",pid_a_finalizar);
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista NEW pero no se pudo eliminar.",pid_a_finalizar);
            }
        }



pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_ready,pid_a_finalizar);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            encontrado=true;
            if(list_remove_element(lista_ready,pcb_a_eliminar)){
                log_info(logger_debug,"Proceso con PID:%u eliminado de la lista READY",pid_a_finalizar);
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista READY pero no se pudo eliminar.",pid_a_finalizar);
            }
        }

pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_ready_prioridad,pid_a_finalizar);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            encontrado=true;
            if(list_remove_element(lista_ready_prioridad,pcb_a_eliminar)){
                log_info(logger_debug,"Proceso con PID:%u eliminado de la lista READY PRIORITARIO",pid_a_finalizar);
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista READY PRIORITARIO pero no se pudo eliminar.",pid_a_finalizar);
            }
        }

pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_bloqueado,pid_a_finalizar);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            encontrado=true;
            if(list_remove_element(lista_bloqueado,pcb_a_eliminar)){
                log_info(logger_debug,"Proceso con PID:%u eliminado de la lista BLOCKED",pid_a_finalizar);
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista BLOCKED pero no se pudo eliminar.",pid_a_finalizar);
            }
        }

pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_bloqueado_prioritario,pid_a_finalizar);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            encontrado=true;
            if(list_remove_element(lista_bloqueado_prioritario,pcb_a_eliminar)){
                log_info(logger_debug,"Proceso con PID:%u eliminado de la lista BLOCKED PRIORITARIO",pid_a_finalizar);
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista BLOCKED PRIORITARIO pero no se pudo eliminar.",pid_a_finalizar);
            }
        }

if (!encontrado)
{
    log_error(logger_debug,"Proceso con PID: %u que se solicito finalizar NO existe.",pid_a_finalizar);
}



}






void imprimir_listas_de_estados(t_list* lista,char* estado){
    t_link_element* punteroLista=lista->head;
    char* log_lista = string_new();
    t_pcb* pcb;
	string_append(&log_lista, "[");
        
        while(punteroLista!=NULL)
		{   
            pcb = (t_pcb*) punteroLista->data;
			char* string_pid = string_itoa(pcb->PID);
			string_append(&log_lista, string_pid);
			free(string_pid);
			if(punteroLista->next!=NULL){
				string_append(&log_lista, ", ");
			}
            punteroLista=punteroLista->next;
		}

        if (strcmp(estado,"BLOCKED")==0)                                    ///EN EL CASO BLOCKED IMPRIMO TAMBIEN LOS PROCESOS QUE ESTAN ESPERANDO RECURSOS
        {   
            t_recurso* auxiliar = lista_de_recursos;
            
            while (auxiliar!=NULL)
            {
                punteroLista=auxiliar->lista_de_espera->head;   
                string_append(&log_lista, ", ");
                while (punteroLista!=NULL)
                {
                    pcb = (t_pcb*) punteroLista->data;
                    char* string_pid = string_itoa(pcb->PID);
			        string_append(&log_lista, string_pid);
			        free(string_pid);
			        if(punteroLista->next!=NULL){
			        	string_append(&log_lista, ", ");
			        }
                    punteroLista =punteroLista->next;
		        }   
                auxiliar=auxiliar->siguiente_recurso;
            }
                
            }
            
        
        
		string_append(&log_lista, "]");
		log_info(logger,"Lista de PID de procesos en estado %s : %s",estado, log_lista);
		

        free(log_lista);


}




    
