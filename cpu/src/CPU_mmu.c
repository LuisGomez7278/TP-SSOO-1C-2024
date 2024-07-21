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
        for (int i = 0; i < cant_entradas_TLB; i++)
        {
            
            aux->libre = true;
            aux->PID = 0;
            aux->nro_pag = 0;
            aux->marco = 0;
            aux->t_ingreso = temporal_create();
            aux->t_ultimo_uso = temporal_create();
            list_add(tabla_TLB, aux);
        }
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

uint32_t get_marco(uint32_t PID_pedida, uint32_t nro_pag)
{
    char* tlb = usa_TLB ? "Usa TLB" : "No usa TLB";
    log_debug(logger_debug, "PID: %u - Necesita la pagina nro: %u. %s", PID_pedida, nro_pag, tlb);
    uint32_t marco;
    if (usa_TLB)
    {
        entrada_TLB* entrada = buscar_en_tlb(PID_pedida, nro_pag);
        marco = marco_TLB(entrada);
    }
    else
    {
        marco = pedir_marco_a_memoria(PID_pedida, nro_pag);
    }
    return marco;
}

entrada_TLB* buscar_en_tlb(uint32_t PID_pedida, uint32_t nro_pag)
{
    entrada_TLB* entrada;
    for (int i = 0; i < cant_entradas_TLB; i++)
    {
        entrada = list_get(tabla_TLB, i);
        if (entrada->PID == PID_pedida && entrada->nro_pag == nro_pag) 
        {
            log_info(logger, "ID: %u - TLB HIT - Pagina: %u", PID_pedida, nro_pag);
            return entrada;
        }
    }

    log_info(logger, "ID: %u - TLB MISS - Pagina: %u", PID_pedida, nro_pag);
    entrada = TLB_miss(PID_pedida, nro_pag);
    return entrada;
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

    log_debug(logger_debug, "Esperando marco de memoria");
    sem_wait(&respuesta_marco);    
    uint32_t marco = marco_pedido;

    log_info(logger, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", PID, nro_pag, marco);
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
        entrada_actual = list_get(tabla_TLB, i);
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

void solicitar_lectura_string(uint32_t direccion_logica_READ, uint32_t bytes_a_copiar)
{
    uint32_t bytes_restantes = bytes_a_copiar;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_READ);
    uint32_t offset = obtener_desplazamiento(direccion_logica_READ);
    uint32_t marco;
    
    uint32_t cant_accesos = ceil((bytes_a_copiar + offset) / tamanio_de_pagina);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_READ);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    
    marco = get_marco(PID, nro_pag);

    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);

    uint32_t tam_acceso = cant_accesos==1 ? bytes_a_copiar : (tamanio_de_pagina-offset);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    bytes_restantes-=tam_acceso;

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag);
        
        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
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
    uint32_t bytes_restantes = bytes_a_copiar;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_WRITE);
    uint32_t offset = obtener_desplazamiento(direccion_logica_WRITE);
    uint32_t marco;

    uint32_t cant_accesos = ceil((bytes_a_copiar + offset) / tamanio_de_pagina);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_WRITE);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    
    marco = get_marco(PID, nro_pag);

    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    uint32_t tam_acceso = cant_accesos==1 ? bytes_a_copiar : (tamanio_de_pagina-offset);

    agregar_a_paquete_string(paquete, tam_acceso, string_leida);
    bytes_restantes-=tam_acceso;

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag);
        
        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_string(paquete, tamanio_de_pagina, string_leida + (tamanio_de_pagina * i));
            bytes_restantes -= tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_string(paquete, bytes_restantes, string_leida + (tamanio_de_pagina * i));
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        i++;
    }
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete); 
}


uint32_t solicitar_MOV_IN(uint32_t direccion_logica, uint32_t tamanio_registro)
{
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t bytes_restantes = tamanio_registro;
    uint32_t cant_accesos = (offset+tamanio_registro > tamanio_de_pagina) ? 2 : 1;

    uint32_t marco;
    uint32_t dir_fisica;
    uint32_t dir_fisica_inicial;

    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_IN);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    agregar_a_paquete_uint32(paquete, tamanio_registro);


    marco = get_marco(PID, nro_pag);

    dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);
    agregar_a_paquete_uint32(paquete, (tamanio_de_pagina-offset));//n bytes, los faltantes hasta el fin del marco/pagina
    
    dir_fisica_inicial = dir_fisica;

    if (cant_accesos>1)
    {
        marco = get_marco(PID, nro_pag);

        dir_fisica = (marco*tamanio_de_pagina);
        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_uint32(paquete, bytes_restantes);
    }

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);

    return dir_fisica_inicial;
}

uint32_t solicitar_MOV_OUT(uint32_t direccion_logica, uint32_t tamanio_registro, int valor)
{
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t bytes_restantes = tamanio_registro;
    uint32_t cant_accesos = (offset+tamanio_registro > tamanio_de_pagina) ? 2 : 1;
    void* puntero_valor = &valor;

    uint32_t marco;
    uint32_t dir_fisica;
    uint32_t dir_fisica_inicial;

    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_OUT);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    marco = get_marco(PID, nro_pag);

    dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);

    uint32_t tam_acceso = cant_accesos==1 ? tamanio_registro : (tamanio_de_pagina-offset);
    agregar_a_paquete_string(paquete, tam_acceso, puntero_valor);
    dir_fisica_inicial = dir_fisica;
    
    bytes_restantes -= (tamanio_de_pagina-offset);
    if (cant_accesos>1)
    {
        marco = get_marco(PID, nro_pag);

        dir_fisica = (marco*tamanio_de_pagina);
        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_string(paquete, bytes_restantes, puntero_valor + (tamanio_de_pagina-offset));
    }

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
    return dir_fisica_inicial;
}

op_code recibir_respuesta_MOV_OUT()
{
    return recibir_operacion(socket_cpu_memoria);
}
