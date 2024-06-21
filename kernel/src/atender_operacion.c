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
        int valor= atoi (array_de_comando[1]);
        cambiar_grado_multiprogramacion(valor );   
        
    }else if (strcmp(array_de_comando[0],"PROCESO_ESTADO")==0){
        
        
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







    
