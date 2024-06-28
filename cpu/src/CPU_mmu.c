#include "../include/CPU_mmu.h"

void inicializar_TLB()
{
    if (cant_entradas_TLB == 0)
    {
        usa_TLB = false;
        log_info(logger, "TLB deshabilitada");
    }
    else
    {
        usa_TLB = true;
        log_info(logger, "TLB habilitada, cantidad de entradas: %u", cant_entradas_TLB);
        tabla_TLB = list_create();

        entrada_TLB* aux = malloc(sizeof(entrada_TLB));
        int i = 0;
        do
        {
            aux->libre = true;
            aux->PID = 0;
            aux->nro_pag = 0;
            aux->marco = 0;
            aux->t_ingreso = temporal_create();
            aux->t_ultimo_uso = temporal_create();
            if (i<cant_entradas_TLB-1)
            {
                aux->siguiente_entrada = malloc(sizeof(entrada_TLB));
                list_add(tabla_TLB, aux);
                aux = aux->siguiente_entrada;
            }
            else
            {
                aux->siguiente_entrada = (entrada_TLB*) NULL;
                list_add(tabla_TLB, aux);
            }
            i++;

        } while (i<cant_entradas_TLB);
    }    
}

uint32_t obtener_nro_pagina(uint32_t direccion_logica)
{
    return floor(direccion_logica/tamanio_de_pagina);
}

uint32_t obtener_desplazamiento(uint32_t direccion_logica)
{
    return direccion_logica%tamanio_de_pagina;
}

entrada_TLB* buscar_en_tlb(uint32_t PID, uint32_t nro_pag)
{
    entrada_TLB* entrada = list_get(tabla_TLB, 0);
    for (int i = 0; i < cant_entradas_TLB; i++)
    {
        if (entrada->PID == PID && entrada->nro_pag == nro_pag) {return entrada;}
        else {entrada = entrada->siguiente_entrada;}
    }
    return TLB_miss(PID, nro_pag);
}

uint32_t marco_TLB(entrada_TLB* entrada)
{
    return entrada->marco;
}

uint32_t pedir_marco_a_memoria(uint32_t PID, uint32_t nro_pag)
{
    t_paquete* paquete = crear_paquete(TLB_MISS);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, nro_pag);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);

    op_code op = recibir_operacion(socket_cpu_memoria);
    if (op != TLB_MISS){
        log_error(logger, "Llego otra cosa en lugar de un marco para la TLB, codigo: %d", op);
    }

    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);
    uint32_t marco = leer_de_buffer_uint32(buffer, &desplazamiento);

    free(buffer);
    return marco;
}

entrada_TLB* TLB_miss(uint32_t PID, uint32_t nro_pag)
{
    uint32_t marco = pedir_marco_a_memoria(PID, nro_pag);
    entrada_TLB* entrada = buscar_entrada_para_reemplazar(PID, nro_pag, marco);
    entrada->libre = false;
    entrada->marco = marco;
    entrada->nro_pag = nro_pag;
    entrada->PID = PID;
    temporal_destroy(entrada->t_ingreso);
    entrada->t_ingreso = temporal_create();
    temporal_destroy(entrada->t_ultimo_uso);
    entrada->t_ultimo_uso = temporal_create();

    return entrada;
}

entrada_TLB* buscar_entrada_para_reemplazar(uint32_t PID, uint32_t nro_pag, uint32_t marco)
{

    entrada_TLB* entrada_actual = list_get(tabla_TLB, 0);
    entrada_TLB* entrada_a_reemplazar = entrada_actual;
    for (int i = 0; i < cant_entradas_TLB; i++)
    {
        if (entrada_actual->libre) {return entrada_actual;}
        else 
        {
            entrada_a_reemplazar = algoritmo_de_reemplazo(entrada_actual, entrada_a_reemplazar);
        }
    }
    return entrada_a_reemplazar;
}

entrada_TLB* algoritmo_de_reemplazo(entrada_TLB* entrada_actual, entrada_TLB* entrada_a_reemplazar)
{
    entrada_TLB* entrada;
    if (string_equals_ignore_case(algoritmo_TLB, "FIFO"))
    {
        //si diff>0 entonces entrada_actual esta hace mas tiempo en TLB que entrada_a_reemplazar
        entrada = temporal_diff(entrada_actual->t_ingreso, entrada_a_reemplazar->t_ingreso) > 0 ? entrada_actual : entrada_a_reemplazar;
    }
    else //(string_equals_ignore_case(algoritmo_TLB, "LRU")) 
    {
        //si diff>0 entonces entrada_actual se uso hace mas tiempo que entrada_a_reemplazar
        entrada = temporal_diff(entrada_actual->t_ultimo_uso, entrada_a_reemplazar->t_ultimo_uso) > 0 ? entrada_actual : entrada_a_reemplazar;
    }    
    return entrada;
}

char* leer_string_de_memoria(uint32_t direccion_logica_READ, uint32_t bytes_a_copiar)
{
    solicitar_lectura_string(direccion_logica_READ, bytes_a_copiar);
    if (recibir_operacion(socket_cpu_memoria) != SOLICITUD_COPY_STRING_READ) 
    {
        log_error(logger, "Esperaba una string por SOLICITUD_COPY_STRING_READ y vino otra cosa");
        return (char*) NULL;
    }
    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    char* string_leida = leer_de_buffer_string(buffer, &desplazamiento);
    log_info(logger, "Llego la string: %s de memoria", string_leida);
    free(buffer);

    return string_leida;
}

void solicitar_lectura_string(uint32_t direccion_logica_READ, uint32_t bytes_a_copiar)
{
    uint32_t bytes_restantes = bytes_a_copiar;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_READ);
    uint32_t offset = obtener_desplazamiento(direccion_logica_READ);
    entrada_TLB* entrada = buscar_en_tlb(PID, nro_pag);
    uint32_t marco = marco_TLB(entrada);
    
    uint32_t cant_accesos = ceil((bytes_a_copiar + offset) / tamanio_de_pagina);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_READ);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    //la primera lectura puede estar corrida por el offset
    agregar_a_paquete_uint32(paquete, marco);
    agregar_a_paquete_uint32(paquete, offset);
    agregar_a_paquete_uint32(paquete, (tamanio_de_pagina-offset));
    bytes_restantes-=(tamanio_de_pagina-offset);

    int i = 1;
    while (bytes_restantes>0)
    {
        nro_pag = obtener_nro_pagina(direccion_logica_READ + (tamanio_de_pagina * i));
        entrada = buscar_en_tlb(PID, nro_pag);
        marco = marco_TLB(entrada);
        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, marco);
            agregar_a_paquete_uint32(paquete, 0);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, marco);
            agregar_a_paquete_uint32(paquete, 0);
            agregar_a_paquete_uint32(paquete, bytes_restantes);
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        i++;
    }
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);  
}

void escribir_en_memoria_string(char* string_leida, uint32_t direccion_logica_WRITE, uint32_t bytes_a_copiar)
{
    uint32_t bytes_restantes = 0;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_WRITE);
    uint32_t offset = obtener_desplazamiento(direccion_logica_WRITE);
    entrada_TLB* entrada = buscar_en_tlb(PID, nro_pag);
    uint32_t marco = marco_TLB(entrada);
    
    uint32_t cant_accesos = ceil((bytes_a_copiar + offset) / tamanio_de_pagina);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_WRITE);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    agregar_a_paquete_uint32(paquete, marco);
    agregar_a_paquete_uint32(paquete, offset);
    agregar_a_paquete_string(paquete, (tamanio_de_pagina-offset), string_leida);
    bytes_restantes -= tamanio_de_pagina-offset;

    int i = 1;
    while (bytes_restantes>0)
    {
        nro_pag = obtener_nro_pagina(direccion_logica_WRITE + (tamanio_de_pagina * i));
        entrada = buscar_en_tlb(PID, nro_pag);
        marco = marco_TLB(entrada);
        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, marco);
            agregar_a_paquete_uint32(paquete, 0);
            agregar_a_paquete_string(paquete, tamanio_de_pagina, string_leida + (tamanio_de_pagina * i));
            bytes_restantes -= tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, marco);
            agregar_a_paquete_uint32(paquete, 0);
            agregar_a_paquete_string(paquete, bytes_restantes, string_leida + (tamanio_de_pagina * i));
            bytes_restantes -= bytes_restantes; //aca sale del while
        }
        i++;
    }
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete); 
}


void solicitar_MOV_IN(uint32_t marco, uint32_t offset, uint32_t tamanio_registro)
{
    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_IN);
    agregar_a_paquete_uint32(paquete, marco);
    agregar_a_paquete_uint32(paquete, offset);
    agregar_a_paquete_uint32(paquete, tamanio_registro);

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}

uint8_t recibir_respuesta_MOV_IN_8b()
{
    if (recibir_operacion(socket_cpu_memoria) != SOLICITUD_MOV_IN) 
    {
        log_error(logger, "Esperaba un numero por SOLICITUD_MOV_IN y vino otra cosa");
        return (uint8_t*) NULL;
    }

    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    uint8_t valor = leer_de_buffer_uint8(buffer, &desplazamiento);
    free(buffer);
    return valor;
}

uint32_t recibir_respuesta_MOV_IN_32b()
{
    if (recibir_operacion(socket_cpu_memoria) != SOLICITUD_MOV_IN) 
    {
        log_error(logger, "Esperaba un numero por SOLICITUD_MOV_IN y vino otra cosa");
        return (uint32_t*) NULL;
    }

    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    uint32_t valor = leer_de_buffer_uint32(buffer, &desplazamiento);
    free(buffer);
    return valor;
}

void solicitar_MOV_OUT(uint32_t marco, uint32_t offset, uint32_t tamanio_registro, int valor)
{
    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_OUT);
    agregar_a_paquete_uint32(paquete, marco);
    agregar_a_paquete_uint32(paquete, offset);
    agregar_a_paquete_uint32(paquete, tamanio_registro);
    if (tamanio_registro == sizeof(uint8_t)) {agregar_a_paquete_uint8(paquete, valor);}
    else /*if (tamanio_registro == sizeof(uint32_t))*/ {agregar_a_paquete_uint32(paquete, valor);}
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}

op_code recibir_respuesta_MOV_OUT()
{
    return recibir_operacion(socket_cpu_memoria);
}