#include "../include/CPU_Main.h"

int main(int argc, char* argv[]) {

    iniciar_CPU();

    // crear conexion
    socket_cpu_memoria = crear_conexion(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a MEMORIA");
    recibir_tamanio_de_pagina();
    inicializar_TLB();

    // //iniciar Server de CPU
    socket_escucha = iniciar_servidor(puerto_escucha_dispatch, logger);

    // //esperar conexion de kernel
    socket_cpu_kernel_dispatch = esperar_cliente(socket_escucha, logger);
    socket_cpu_kernel_interrupt = esperar_cliente(socket_escucha, logger);

    pthread_create(&hilo_conexion_interrupt, NULL, (void*) gestionar_conexion_interrupt, NULL);
    pthread_detach(hilo_conexion_interrupt);
            
    recibir_proceso();

    while(true){
        t_instruccion* ins_actual = fetch(PID, contexto_interno.PC, socket_cpu_memoria);
        ejecutar_instruccion(PID, &contexto_interno, ins_actual);
        if (interrupcion != INT_NO) {
            if (interrupcion == INT_CONSOLA){motivo_desalojo = DESALOJO_POR_CONSOLA;}
            else /*interrupcion==INT_QUANTUM*/ {motivo_desalojo = DESALOJO_POR_QUANTUM;}
            desalojar_proceso(motivo_desalojo);
            recibir_proceso();
        };
        free(ins_actual);
    }

    // pthread_create(hilo_conexion_dispatch, NULL, (void*) gestionar_conexion_memoria, NULL);
    // pthread_join(hilo_conexion_dispatch, NULL);

    if (socket_cpu_kernel_dispatch) {liberar_conexion(socket_cpu_kernel_dispatch);}
    if (socket_cpu_kernel_interrupt) {liberar_conexion(socket_cpu_kernel_interrupt);}
    if (socket_cpu_memoria) {liberar_conexion(socket_cpu_memoria);}
    if (socket_escucha) {liberar_conexion(socket_escucha);}

    end_program(logger, config);

    return 0;
}

void ejecutar_instruccion(uint32_t PID, t_contexto_ejecucion* contexto_interno, t_instruccion* ins_actual){
    cod_ins codigo = ins_actual->ins;
    int* registro_destino;
    int* registro_origen;
    int* registro;
    int valor;

    uint32_t tamanio_registro;
    uint32_t direccion_logica;
    uint32_t nro_pag;
    uint32_t offset;
    uint32_t marco;

    switch (codigo)
    {
    case SET:
        log_info(logger,"PID: %u - Ejecutando: SET - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        valor = atoi(ins_actual->arg2);

        *registro = valor;
        contexto_interno->PC++;
        break;

    case SUM:
        log_info(logger,"PID: %u - Ejecutando: SUM - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
        valor = *registro_destino + *registro_origen;

        *registro_destino = valor;
        break;

    case SUB:
        log_info(logger,"PID: %u - Ejecutando: SUB - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
        valor = *registro_destino - *registro_origen;

        *registro_destino = valor;
        break;
        
    case JNZ:
        log_info(logger,"PID: %u - Ejecutando: JNZ - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        if (*registro != 0) {contexto_interno->PC = atoi(ins_actual->arg2);}
        else {contexto_interno->PC++;}
        break;

    case MOV_IN:
        log_info(logger,"PID: %u - Ejecutando: MOV_IN - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        direccion_logica = atoi(ins_actual->arg2);
        tamanio_registro = registro_chico(ins_actual->arg1) ? sizeof(uint8_t) : sizeof(uint32_t);

        nro_pag = obtener_nro_pagina(direccion_logica);
        offset = obtener_desplazamiento(direccion_logica);
        if(usa_TLB)
        {
            entrada_TLB* entrada_tlb = buscar_en_tlb(PID, nro_pag);
            marco = marco_TLB(entrada_tlb);
        }
        else
        {
            marco = pedir_marco_a_memoria(PID, nro_pag);
        }
        solicitar_MOV_IN(marco, offset, tamanio_registro);
        valor = registro_chico(ins_actual->arg1) ? recibir_respuesta_MOV_IN_8b() : recibir_respuesta_MOV_IN_32b();
        *registro = valor;
        log_info(logger,"PID: %u, Valor leido de MOV_IN: %u", PID, *registro);
        
        break;

    case MOV_OUT:
        log_info(logger,"PID: %u - Ejecutando: MOV_OUT - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro = direccion_registro(contexto_interno, ins_actual->arg2);
        valor = *registro;
        tamanio_registro = registro_chico(ins_actual->arg2) ? sizeof(uint8_t) : sizeof(uint32_t);

        direccion_logica = atoi(ins_actual->arg1);
        nro_pag = obtener_nro_pagina(direccion_logica);
        offset = obtener_desplazamiento(direccion_logica);

        if(usa_TLB)
        {
            entrada_TLB* entrada_tlb = buscar_en_tlb(PID, nro_pag);
            marco = marco_TLB(entrada_tlb);
        }
        else
        {
            marco = pedir_marco_a_memoria(PID, nro_pag);
        }

        solicitar_MOV_OUT(marco, offset, tamanio_registro, valor);
        if (recibir_respuesta_MOV_OUT() != OK)
        {
            log_info(logger,"PID: %u, No pudo realizar el MOV_OUT y va al EXIT (Segmentation fault)", PID);
            motivo_desalojo = DESALOJO_POR_FIN_PROCESO;
            desalojar_proceso(motivo_desalojo);
            recibir_proceso();
        }
        else
        {log_info(logger,"PID: %u, Valor escrito con MOV_OUT: %u", PID, valor);}
        break;

    case RESIZE:
        log_info(logger,"PID: %u - Ejecutando: RESIZE - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        valor = *registro;
        pedir_rezise(PID, valor);
        break;

    case COPY_STRING:
        log_info(logger,"PID: %u - Ejecutando: COPY_STRING - %s", PID, ins_actual->arg1);
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        uint32_t bytes_a_copiar = *registro;
        uint32_t direccion_logica_READ = contexto_interno->SI;
        uint32_t direccion_logica_WRITE = contexto_interno->DI;

        char* string_leida = leer_string_de_memoria(direccion_logica_READ, bytes_a_copiar);
        escribir_en_memoria_string(string_leida, direccion_logica_WRITE, bytes_a_copiar);
        // recibir_respuesta_COPY_STRING();
        break;
        
    case IO_GEN_SLEEP:
        log_info(logger,"PID: %u - Ejecutando: IO_GEN_SLEEP - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_GEN_SLEEP;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_STDIN_READ:
        log_info(logger,"PID: %u - Ejecutando: IO_STDIN_READ - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDIN;
        
        char* direccion_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(direccion_r, "%u", *registro);
        
        char* tamanio_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_r, "%u", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, direccion_r, tamanio_r);
        recibir_proceso();
        break;

    case IO_STDOUT_WRITE:
        log_info(logger,"PID: %u - Ejecutando: IO_STDOUT_WRITE - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDOUT;

        char* direccion_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(direccion_w, "%u", *registro);
        
        char* tamanio_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_w, "%u", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, direccion_w, tamanio_w);
        recibir_proceso();
        break;

    case IO_FS_CREATE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_CREATE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_CREATE;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_FS_DELETE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_DELETE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_DELETE;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_FS_TRUNCATE:
        log_info(logger,"PID: %u - Ejecutando: IO_STDIN_READ - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_TRUNCATE;

        char* tamanio_t = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_t, "%u", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, tamanio_t);
        recibir_proceso();
        break;

    case IO_FS_WRITE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_WRITE - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_WRITE;

        char* direccion_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg3);
        sprintf(direccion_FS_w, "%u", *registro);
        
        char* tamanio_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg4);
        sprintf(tamanio_FS_w, "%u", *registro);

        char* puntero_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg5);
        sprintf(puntero_FS_w, "%u", *registro);

        enviar_CE_con_5_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_FS_w, tamanio_FS_w, puntero_FS_w);
        recibir_proceso();
        break;

    case IO_FS_READ:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_READ - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_READ;

        char* direccion_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg3);
        sprintf(direccion_FS_r, "%u", *registro);
        
        char* tamanio_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg4);
        sprintf(tamanio_FS_r, "%u", *registro);

        char* puntero_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg5);
        sprintf(puntero_FS_r, "%u", *registro);

        enviar_CE_con_5_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_FS_r, tamanio_FS_r, puntero_FS_r);
        recibir_proceso();
        break;

    case WAIT:
        log_info(logger,"PID: %u - Ejecutando: WAIT - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        enviar_CE_con_1_arg(DESALOJO_POR_WAIT, ins_actual->arg1);
        if (esperar_respuesta_recurso())
        {
            log_info(logger,"PID: %u - WAIT de recurso: %s fue exitoso", PID, ins_actual->arg1);
            recibir_proceso();
        }
        else
        {
            log_info(logger,"PID: %u - WAIT de recurso: %s fallo", PID, ins_actual->arg1);
            recibir_proceso();
        }
        break;

    case SIGNAL:
        log_info(logger,"PID: %u - Ejecutando: SIGNAL - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        enviar_CE_con_1_arg(DESALOJO_POR_SIGNAL, ins_actual->arg1);
        if (esperar_respuesta_recurso())
        {
            log_info(logger,"PID: %u - SIGNAL de recurso: %s fue exitoso", PID, ins_actual->arg1);
            recibir_proceso();
        }
        else
        {
            log_info(logger,"PID: %u - SIGNAL de recurso: %s fallo", PID, ins_actual->arg1);
            recibir_proceso();
        }
        break;

    case EXIT:
        log_info(logger,"PID: %u - Ejecutando: EXIT", PID);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_FIN_PROCESO;
        desalojar_proceso(motivo_desalojo);
        recibir_proceso();
        break;
        
    default:
        log_error(logger, "el switch de ejecutar_instruccion() llego al default");
        break;
    }
}

void* direccion_registro(t_contexto_ejecucion* contexto, char* registro){

    if (string_equals_ignore_case(registro, "AX"))  {return &(contexto->AX);}
    else if (string_equals_ignore_case(registro, "BX"))  {return &(contexto->BX);}
    else if (string_equals_ignore_case(registro, "CX"))  {return &(contexto->CX);}
    else if (string_equals_ignore_case(registro, "DX"))  {return &(contexto->DX);}
    else if (string_equals_ignore_case(registro, "EAX"))  {return &(contexto->EAX);}
    else if (string_equals_ignore_case(registro, "EBX"))  {return &(contexto->EBX);}
    else if (string_equals_ignore_case(registro, "ECX"))  {return &(contexto->ECX);}
    else if (string_equals_ignore_case(registro, "EDX"))  {return &(contexto->EDX);}
    else if (string_equals_ignore_case(registro, "SI"))  {return &(contexto->SI);}
    else if (string_equals_ignore_case(registro, "DI"))  {return &(contexto->DI);}
    else {
        log_error(logger, "Error en traduccion de string a registro");
        return NULL;
    }
}

bool registro_chico(char* registro)
{
    return (
        string_equals_ignore_case(registro, "AX") || 
        string_equals_ignore_case(registro, "BX") || 
        string_equals_ignore_case(registro, "CX") || 
        string_equals_ignore_case(registro, "DX")
    );
}
