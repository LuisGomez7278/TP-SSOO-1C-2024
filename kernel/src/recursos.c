
#include "../include/recursos.h"



int* convertir_a_enteros_la_lista_de_instancias(char** array_de_cadenas) {
    
    int contador = 0;
    while (array_de_cadenas[contador] != NULL) {
        contador++;
    }

    // Aloca memoria para el array de enteros
    int* array_de_enteros = malloc(contador * sizeof(int));

    // Convierte cada cadena a un entero y almacénalo en el array de enteros
    for (int i = 0; i < contador; i++) {
        array_de_enteros[i] = atoi(array_de_cadenas[i]);
    }
    cantidadDeRecursos=contador;
    return array_de_enteros;
}



 void construir_lista_de_recursos() {
    
    t_recurso* auxiliar = NULL;
    t_recurso* ultimo = NULL;

    for (int i = 0; i < cantidadDeRecursos; i++) {
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








int wait_recursos(char* recurso_solicitado,t_pcb* pcb_solicitante){
    t_recurso* auxiliar = lista_de_recursos;
    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }

    if(auxiliar==NULL){
        return -1;
    }
    if (auxiliar->instancias_del_recurso-auxiliar->instancias_solicitadas_del_recurso<=0)
    {
        list_add(auxiliar->lista_de_espera,pcb_solicitante);
        auxiliar->instancias_solicitadas_del_recurso-=1;
        return 1;
        
    }else{
        auxiliar->instancias_solicitadas_del_recurso-=1;
        return 2;
    }
    

}


int signal_recursos ( char*recurso_solicitado,uint32_t PID){
    t_recurso* auxiliar = lista_de_recursos;

    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }
  
    if(auxiliar==NULL){
        return -1;
    }
    
    if(buscar_pcb_por_PID_en_lista(auxiliar->lista_de_espera,PID)==NULL){
        return -2;
    }
       
     auxiliar->instancias_del_recurso+=1;
    


    if (auxiliar->instancias_del_recurso>0 && list_size(auxiliar->lista_de_espera)>0)
    {   
        t_pcb *pcb_liberado=list_remove(auxiliar->lista_de_espera,0);
        ingresar_en_lista(pcb_liberado, lista_ready, &semaforo_ready, &cantidad_procesos_ready , READY);
    }

    return 1;

}

