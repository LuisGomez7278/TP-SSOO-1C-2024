
#include "../include/recursos.h"



uint32_t* convertir_a_enteros_la_lista_de_instancias(char** array_de_cadenas) {
    
    uint32_t contador = 0;
    while (array_de_cadenas[contador] != NULL) {
        contador++;
    }

    // Aloca memoria para el array de enteros
    uint32_t* array_de_enteros = malloc(contador * sizeof(int));

    // Convierte cada cadena a un entero y almacénalo en el array de enteros
    for (int32_t i = 0; i < contador; i++) {
        array_de_enteros[i] = atoi(array_de_cadenas[i]);
    }
    cantidadDeRecursos=contador;
    return array_de_enteros;
}



 void construir_lista_de_recursos() {
    
    t_recurso* auxiliar = NULL;
    t_recurso* ultimo = NULL;

    for (int32_t i = 0; i < cantidadDeRecursos; i++) {
        auxiliar = malloc(sizeof(t_recurso));
        

        auxiliar->nombre_recurso = malloc(strlen(recursos[i]) + 1);
 
        strcpy(auxiliar->nombre_recurso, recursos[i]);

        auxiliar->instancias_del_recurso = instancias_recursos_int[i];
        auxiliar->instancias_solicitadas_del_recurso = 0;
        auxiliar->lista_de_espera = list_create();
        auxiliar->siguiente_recurso = NULL;

        if (lista_de_recursos == NULL) {
            lista_de_recursos = auxiliar;  
        } else {
            ultimo->siguiente_recurso = auxiliar;  
        }
        ultimo = auxiliar; 
    }
    
}



void imprimir_recursos(){
    t_recurso* auxiliar = lista_de_recursos;

    while(auxiliar!=NULL){
        log_info(logger_debug, "El recurso '%s', tiene %d instancias, de las cuales utiliza %d",auxiliar->nombre_recurso,auxiliar->instancias_del_recurso,auxiliar->instancias_solicitadas_del_recurso);
        auxiliar = auxiliar->siguiente_recurso;
    }


}




uint32_t wait_recursos(char* recurso_solicitado,t_pcb* pcb_solicitante){
    t_recurso* auxiliar = lista_de_recursos;
    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }

    if(auxiliar==NULL){                                                                       //RECURSO NO ENCONTRADO
        return -1;
    }
    if (auxiliar->instancias_del_recurso - auxiliar->instancias_solicitadas_del_recurso<=0)             //RECURSO ENCONTRADO SIN INSTANCIAS DISPONIBLES
    {
        log_info(logger, "PID: %d - Cambio de estado READY -> BLOQUEADO", pcb_solicitante->PID);
        pthread_mutex_lock(&semaforo_recursos);
        list_add(auxiliar->lista_de_espera,pcb_solicitante);
        pthread_mutex_unlock(&semaforo_recursos);
        auxiliar->instancias_solicitadas_del_recurso-=1;
        return 1;
        
    }else{                                                                                                  // //RECURSO ENCONTRADO CON INSTANCIAS DISPONIBLES
        auxiliar->instancias_solicitadas_del_recurso-=1;
        return 2;
    }
    

}


uint32_t signal_recursos ( char*recurso_solicitado,uint32_t PID){                                    
    t_recurso* auxiliar = lista_de_recursos;

    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }
  
    if(auxiliar==NULL){                                                                //RECURSO NO ENCONTRADO
        return -1;
    }
    
    if(buscar_pcb_por_PID_en_lista(auxiliar->lista_de_espera,PID,&semaforo_recursos)==NULL){               //RECURSO NO ASIGNADO AL PROCESO
        return -2;
    }
       
     auxiliar->instancias_del_recurso+=1;
    


    if (auxiliar->instancias_del_recurso>0 && list_size(auxiliar->lista_de_espera)>0)                               //VERIFICO SI HABIA UN PROCESO ESPERANDO EL RECURSO Y LO LIBERO
    {   
        t_pcb *pcb_liberado=list_remove(auxiliar->lista_de_espera,0);
        ingresar_en_lista(pcb_liberado, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);
        
    }

    return 1;                                                                                                       //SIGNAL EXITOSO

}



void eliminar_proceso_de_lista_recursos (uint32_t PID){
    t_pcb* pcb_a_eliminar;
    t_recurso* auxiliar = lista_de_recursos;

    while(auxiliar!=NULL){
        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(auxiliar->lista_de_espera,PID,&semaforo_recursos);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            if(list_remove_element(auxiliar->lista_de_espera,pcb_a_eliminar)){
                auxiliar->instancias_solicitadas_del_recurso+=1;
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista de recursos pero no se pudo eliminar.",PID);
            }
        }

        auxiliar=auxiliar->siguiente_recurso;
    }
}









