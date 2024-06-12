#include "../include/memPaginacion.h"

t_bitarray* bitmap;
t_list* tablaDePaginas;
t_list* procesos;

bool crear_procesoM(char* path_instrucciones, uint32_t PID){ 
    procesoM* proceso = malloc(sizeof(procesoM));
    proceso->pid = PID;
    t_list* lista_inst = leer_pseudocodigo(path_instrucciones);
    if(lista_inst==NULL){
        perror("Error al crear la lista de instrucciones");
        return false;
    }else{proceso->instrs = lista_inst;}

    t_list* tabla_paginas = list_create(); //empieza con tabla de paginas vacia
    proceso->paginas = tabla_paginas;

    añadirTablaALista(tabla_paginas, PID);
    list_add(procesos, proceso);
    log_info(logger_debug, "Proceso con PID %d creado", PID);
    return true;
} 

void eliminar_procesoM(uint32_t PID){
    // Buscar el proceso en la lista de procesos
    procesoM* proceso = buscar_proceso_por_pid(PID);
    if (proceso == NULL) {
        log_error(logger_debug, "Proceso con PID %d no encontrado para finalizar", PID);
        return;
    }

    // Liberar los frames del proceso
    liberar_frames(proceso->paginas);

    // Eliminar el proceso de la lista de procesos
    for (int i = 0; i < list_size(procesos); i++) {
        procesoM* p = list_get(procesos, i);
        if (p->pid == PID) {
            list_remove_and_destroy_element(procesos, i, free);
            break;
        }
    }

    // Liberar la memoria asociada al proceso
    list_destroy_and_destroy_elements(proceso->instrs, free);
    list_destroy_and_destroy_elements(proceso->paginas, free);
    free(proceso);

    // Buscar y eliminar la tabla de páginas del proceso
    tabla_pag_proceso* tabla_pag_p = obtener_tabla_pag_proceso(PID);
    if (tabla_pag_p != NULL) {
        for (int i = 0; i < list_size(tablaDePaginas); i++) {
            tabla_pag_proceso* tabla = list_get(tablaDePaginas, i);
            if (tabla->pid == PID) {
                list_remove_and_destroy_element(tablaDePaginas, i, free);
                break;
            }
        }

        // Liberar la memoria asociada a la tabla de páginas del proceso
        list_destroy_and_destroy_elements(tabla_pag_p->paginas, free);
        free(tabla_pag_p);
    }

    log_info(logger_debug, "Proceso con PID %d finalizado y eliminado", PID);
}

void liberar_frames(t_list* paginas){
    for (int i = 0; i < list_size(paginas); i++) {
        tabla_pag* pagina = list_get(paginas, i);
        if (pagina->presencia) {
            bitarray_clean_bit(bitmap, pagina->marco);
        }
    }
}

void resize(uint32_t PID, int size) {
    tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
    if (tabla_proceso == NULL) {
        log_error(logger, "Proceso con PID %d no encontrado", PID);
        exit(EXIT_FAILURE);
    }

    int num_paginas_actuales = list_size(tabla_proceso->paginas);
    int num_paginas_requeridas;

    if (size > 0) {
        // Añadir páginas
        num_paginas_requeridas = (abs(size) + tam_pagina - 1) / tam_pagina;  // Redondea hacia arriba
        añadir_pagina_a_proceso(tabla_proceso, num_paginas_requeridas, PID);
    } else if (size < 0) {
        // Eliminar páginas
        num_paginas_requeridas = abs(size)/tam_pagina; // Redondea hacia abajo
        eliminar_pagina_de_proceso(tabla_proceso, num_paginas_requeridas);
    }
}

void añadir_pagina_a_proceso(tabla_pag_proceso* tabla, uint32_t num_paginas, uint32_t PID) {
    for (int i = 0; i < num_paginas; i++) {
        tabla_pag* nueva_pagina = malloc(sizeof(tabla_pag));
        if (nueva_pagina == NULL) {
            log_error(logger, "Error al asignar memoria para la nueva página");
            exit(EXIT_FAILURE);
        }
        nueva_pagina->marco = -1;  // Asignar marco apropiado
        nueva_pagina->presencia = false;  // Inicialmente no presente

        list_add(tabla->paginas, nueva_pagina);
        int numero_pagina = list_size(tabla->paginas) - 1;
        int marco = asignar_marco(PID, numero_pagina);
        if (marco == -1) {
            log_error(logger, "Out Of Memory");
            exit(EXIT_FAILURE);
        } else {
            log_info(logger, "Página añadida y asignada al proceso %d. Total de páginas: %d", PID, list_size(tabla->paginas));
        }    
    }
}

void eliminar_pagina_de_proceso(tabla_pag_proceso* tabla, int num_paginas) {
    for (int i = 0; i < num_paginas; i++) {
        int index = list_size(tabla->paginas) - 1;
        if (index < 0) break;

        // Obtener la página a eliminar antes de eliminarla de la lista
        tabla_pag* pagina_a_eliminar = list_get(tabla->paginas, index);
        if (pagina_a_eliminar != NULL) {
            // Liberar el marco correspondiente si es necesario
            if (pagina_a_eliminar->presencia) {
                bitarray_clean_bit(bitmap, pagina_a_eliminar->marco);  // Libera el marco en el bitmap
                log_info(logger, "Liberado el marco %d del proceso %d", pagina_a_eliminar->marco, tabla->pid);
            }
            free(pagina_a_eliminar);
        }

        // Ahora eliminar la página de la lista
        list_remove(tabla->paginas, index);
        log_info(logger, "Página eliminada del proceso %d. Total de páginas: %d", tabla->pid, list_size(tabla->paginas));
    }
}

void añadirTablaALista(t_list* paginas, uint32_t PID){  
    // añade una tabla de pagina a la lista global de todas las tablas con su PID

    tabla_pag_proceso* nueva_tabla_proceso = malloc(sizeof(tabla_pag_proceso)); 
    if (nueva_tabla_proceso == NULL) {
        perror("Error al crear la tabla del proceso");
        return 1;
    }

    nueva_tabla_proceso->pid = PID;
    nueva_tabla_proceso->paginas = paginas;

    log_info(logger, "Creacion de tabla de paginas: PID: %d. Tamaño: %d", PID, list_size(paginas));

    list_add(tablaDePaginas, nueva_tabla_proceso);
}

tabla_pag_proceso* obtener_tabla_pag_proceso(uint32_t PID){
    for (int i = 0; i < list_size(tablaDePaginas); i++) {
        tabla_pag_proceso* tabla = list_get(tablaDePaginas, i);
        if (tabla->pid == PID) {
            //log_info(logger, "Tabla de paginas del proceso PID: %d obtenida con exito", PID);
            return tabla;
        }
    }
    return NULL;
}

tabla_pag* obtener_pagina_proceso(tabla_pag_proceso* tabla_proceso, int numero_pagina) {
    if (numero_pagina < list_size(tabla_proceso->paginas)) {
        return list_get(tabla_proceso->paginas, numero_pagina);
    }
    return NULL;
}

tabla_pag* buscar_siguiente_pagina(tabla_pag_proceso* tabla_proceso, int marco_actual) {
    for (int i = 0; i < list_size(tabla_proceso->paginas); i++) {
        tabla_pag* entrada_pagina = list_get(tabla_proceso->paginas, i);
        if (entrada_pagina->marco == marco_actual && entrada_pagina->presencia) {
            // Verificar si hay una página siguiente
            if (i + 1 < list_size(tabla_proceso->paginas)) {
                return list_get(tabla_proceso->paginas, i + 1);
            }
        }
    }
    return NULL;
}

procesoM* buscar_proceso_por_pid(uint32_t pid){
    for (int i = 0; i < list_size(procesos); i++) {
        procesoM* proceso = list_get(procesos, i);
        if (proceso->pid == pid) {
            return proceso;
        }
    }
    log_error(logger, "Proceso con PID %d no encontrado", pid);
    return NULL;
}

t_list* obtener_instrs(uint32_t pid){
    procesoM* proceso = buscar_proceso_por_pid(pid);
    if (proceso != NULL) {
        return proceso->instrs;
    }
    log_error(logger, "No se pudo obtener la lista de instrucciones del proceso con PID %d", pid);
    return NULL;
}

int asignar_marco(uint32_t PID, int numero_pagina) {
    tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
    if (tabla_proceso == NULL) {
        log_error(logger, "Error: no se encontró la tabla de páginas del proceso");
        exit(EXIT_FAILURE);
    }
    tabla_pag* pagina = obtener_pagina_proceso(tabla_proceso, numero_pagina);
    if (pagina == NULL) {
        log_error(logger, "Error: no se encontró la tabla de páginas del proceso");
        exit(EXIT_FAILURE);
    }

    // Buscar un marco libre en el bitarray
    int cant_frames = bitarray_get_max_bit(bitmap);
    int marco_libre = -1;

    for (int i = 0; i < cant_frames; i++) {
        if (!bitarray_test_bit(bitmap, i)) { // Si el bit está en 0, el marco está libre
            marco_libre = i;
            bitarray_set_bit(bitmap, i); // Marcar el bit como ocupado
            break;
        }
    }

    if (marco_libre == -1) {
        return -1; // No hay marcos disponibles
    }

    // Asignar el marco a la página
    pagina->marco = marco_libre;
    pagina->presencia = true;

    return marco_libre; // Retornar el marco asignado
}


int obtener_marco(int direccion_fisica) {
    return direccion_fisica / tam_pagina;
}

int obtener_desplazamiento(int direccion_fisica) {
    return direccion_fisica % tam_pagina;
}

bool escribir_memoria(int direccion_fisica, int bytes, char* valor, uint32_t PID){
    int bytes_escritos = 0;
    int direccion_actual = direccion_fisica;
    int bytes_restantes = bytes;

    while (bytes_restantes > 0){
        int marco = obtener_marco(direccion_actual);
        int offset = obtener_desplazamiento(direccion_actual);
    
        int espacio_en_marco = tam_pagina - offset;
        int bytes_a_escribir;

        if(bytes_restantes < espacio_en_marco){
        bytes_a_escribir = bytes_restantes; 
        }else{bytes_a_escribir = espacio_en_marco;}

        memcpy((char*)memoria_usuario + (marco * tam_pagina) + offset, valor + bytes_escritos, bytes_a_escribir);

        log_info(logger, "Acceso a Espacio de Usuario: PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d bytes", PID, direccion_actual, bytes_a_escribir);

        bytes_escritos += bytes_a_escribir;
        bytes_restantes -= bytes_a_escribir;

        if (bytes_restantes > 0) {
            // Obtener la siguiente página
            //log_info(logger, "Entra en if");
            tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
            //log_info(logger, "Obtiene pagina");
            if (tabla_proceso == NULL) {
                return false;
            }
            
            tabla_pag* siguiente_pagina = buscar_siguiente_pagina(tabla_proceso, marco);
            if (siguiente_pagina == NULL || !siguiente_pagina->presencia) {
                return false;
            }

            direccion_actual = siguiente_pagina->marco * tam_pagina;  // Empezar desde el inicio del siguiente marco
        }
    }
    return true;
}

char* leer_memoria(int direccion_fisica, int size, uint32_t PID){
    char* buffer = malloc(size);
    int bytes_leidos = 0;
    int direccion_actual = direccion_fisica;
    int bytes_restantes = size;

    while (bytes_restantes > 0) {
        int marco = obtener_marco(direccion_actual);
        int offset = obtener_desplazamiento(direccion_actual);
        int espacio_en_marco = tam_pagina - offset;
        int bytes_a_leer = (bytes_restantes < espacio_en_marco) ? bytes_restantes : espacio_en_marco;

        memcpy(buffer + bytes_leidos, (char*)memoria_usuario + (marco * tam_pagina) + offset, bytes_a_leer);

        log_info(logger, "Acceso a Espacio de Usuario: PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d bytes", PID, direccion_actual, bytes_a_leer);
        bytes_leidos += bytes_a_leer;
        bytes_restantes -= bytes_a_leer;

        if (bytes_restantes > 0) {
            tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
            if (tabla_proceso == NULL) {
                free(buffer);
                return NULL;
            }

            tabla_pag* siguiente_pagina = buscar_siguiente_pagina(tabla_proceso, marco);
            if (siguiente_pagina == NULL || !siguiente_pagina->presencia) {
                free(buffer);
                return NULL;
            }

            direccion_actual = siguiente_pagina->marco * tam_pagina;
        }
    }
    log_info(logger, "%.*s", size, buffer);  
    return buffer;
}
