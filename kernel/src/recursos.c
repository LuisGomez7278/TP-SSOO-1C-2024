
#include "../include/recursos.h"

    char* recursos[] = {"Recurso1", "Recurso2", "Recurso3", NULL};

    // Iterar sobre los elementos
    for (char** ptr = recursos; *ptr != NULL; ptr++) {
        printf("%s\n", *ptr);
    }


void wait_a_recurso (char*)




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
    t_recurso* lista_de_recursos = NULL;
    t_recurso* auxiliar = NULL;
    
    for (int i = 0; i < cantidadDeRecursos; i++) {
        auxiliar = malloc(sizeof(t_recurso));


        auxiliar->nombre_recurso = malloc(strlen(recursos[i]) + 1);
 
        strcpy(auxiliar->nombre_recurso, recursos[i]);

        auxiliar->instancias_del_recurso = instancias_recursos_int[i];
        auxiliar->instancias_utilizadas_del_recurso = 0;
        auxiliar->lista_de_espera = NULL;
        auxiliar->siguiente_recurso = NULL;

        if (lista_de_recursos == NULL) {
            lista_de_recursos = auxiliar;  // Primer nodo de la lista
        } 
    }
    
}






}






/*
int obtener_cantidad_recursos(char** config_recursos){
    int i = 0;
    while (config_recursos[i] != NULL)
    {
        i++;
    }
    return i;
}

void cargar_recursos(char** recursos, char** instancias_recursos, int cant_recursos){
    for (int i = 0; i<cant_recursos; i++)
    {
        t_recurso* rec = malloc(sizeof(t_recurso));
        rec->instancias = atoi(instancias_recursos[i]); //No se si funciona, es un char**
        rec->n_recurso = i;
        rec->bloqueados = list_create();

        dictionary_put(dict_recursos, recursos[i], rec);
    }
}

void liberar_recursos(t_pcb* pcb, char** lista_recursos)
{
    for (int i = 0; i < cantidad_recursos; i++)
    {
        t_recurso* rec = dictionary_get(dict_recursos, lista_recursos[i]);

        while (pcb->recursos_proceso[i] > 0)
        {
            rec->instancias += 1;
            pcb->recursos_proceso[i] -= 1;
        }

        dictionary_put(dict_recursos, lista_recursos[i], rec);
    }
}*/
