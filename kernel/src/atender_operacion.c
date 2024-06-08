#include "../include/atender_operacion.h"

void atender_instruccion_validada(char* leido)
{

    char** array_de_comando= string_split (leido, " ");

    if(strcmp(array_de_comando[0],"EJECUTAR_SCRIPT")==0){
        

    }else if (strcmp(array_de_comando[0],"INICIAR_PROCESO")==0){
        iniciar_proceso(leido);        
    }else if (strcmp(array_de_comando[0],"FINALIZAR_PROCESO")==0){
        
        
    }else if (strcmp(array_de_comando[0],"DETENER_PLANIFICACION")==0){
       
        
    }else if (strcmp(array_de_comando[0],"INICIAR_PLANIFICACION")==0){
        
        
    }else if (strcmp(array_de_comando[0],"MULTIPROGRAMACION")==0){
        
        
    }else if (strcmp(array_de_comando[0],"PROCESO_ESTADO")==0){
        
        
    }


}


void iniciar_proceso(char*leido){
    char** array_de_comando= string_split (leido, " ");

    t_pcb *new_pcb= malloc(sizeof(t_pcb));
    new_pcb->PID=asignar_pid();
    new_pcb->estado=NUEVO; 
    new_pcb->quantum= quantum;
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

    queue_push(cola_new, new_pcb);
    
    solicitud_de_creacion_proceso_a_memoria(new_pcb->PID,leido);



}







    
