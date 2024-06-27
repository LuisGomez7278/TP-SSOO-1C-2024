#include "../include/CPU_Main.h"

int main(int argc, char* argv[]) {

        iniciar_CPU();

    // //INICIAR SERVIDOR CPU
        socket_escucha = iniciar_servidor(puerto_escucha_dispatch, logger);
    
    //CREAR CONEXION CON MEMORIA 
        socket_cpu_memoria = crear_conexion(ip_memoria, puerto_memoria);
        log_info(logger, "Conectado a MEMORIA");
        recibir_tamanio_de_pagina();


    //// ESPERAR CONEXION CON KERNEL
        socket_cpu_kernel_dispatch = esperar_cliente(socket_escucha, logger);
        socket_cpu_kernel_interrupt = esperar_cliente(socket_escucha, logger);
        log_info(logger,"CPU conectado a Kernel");
    
    t_instruccion* ins1;
    ins1 = malloc(sizeof(t_instruccion));
    ins1->ins = SET;
    ins1->arg1 = "AX";
    ins1->arg2 = "8";

    t_instruccion* ins2;   
    ins2 = malloc(sizeof(t_instruccion));
    ins2->ins = IO_GEN_SLEEP;
    ins2->arg1 = "Int1";
    ins2->arg2 = "5";
    
    // t_instruccion* ins3;
    // ins3 = malloc(sizeof(t_instruccion));
    // ins3->ins = SUM;
    // ins3->arg1 = "AX";
    // ins3->arg2 = "BX";

    log_info(logger, "I: AX: %d", contexto_interno.AX);
    ejecutar_instruccion(PID, &contexto_interno, ins1);
    log_info(logger, "II: AX: %d", contexto_interno.AX);
    ejecutar_instruccion(PID, &contexto_interno, ins2);
    log_info(logger, "III: BX: %d", contexto_interno.BX);
    // ejecutar_instruccion(PID, &contexto_interno, ins3);    
    // log_info(logger, "IV: AX: %d BX: %d", contexto_interno.AX, contexto_interno.BX);
        
    // recibir_proceso();

    // while(true){
    //     t_instruccion* ins_actual = fetch(PID, contexto_interno.PC, socket_cpu_memoria);
    //     ejecutar_instruccion(PID, &contexto_interno, ins_actual);
    //     if (check_interrupt(interrupcion)) {
    //         motivo_desalojo = DESALOJO_POR_INTERRUPCION;
    //         desalojar_proceso(motivo_desalojo);
    //         recibir_proceso();
    //     };
    //     free(ins_actual);
    // }

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

    switch (codigo)
    {
    case SET:
        log_info(logger,"PID: %d - Ejecutando: SET - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        valor = atoi(ins_actual->arg2);

        *registro = valor;
        contexto_interno->PC++;
        break;

    case SUM:
        log_info(logger,"PID: %d - Ejecutando: SUM - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
        valor = *registro_destino+*registro_origen;

        *registro_destino = valor;
        break;

    case SUB:
        log_info(logger,"PID: %d - Ejecutando: SUB - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
        valor = *registro_destino-*registro_origen;

        *registro_destino = valor;
        break;
        
    case JNZ:
        log_info(logger,"PID: %d - Ejecutando: JNZ - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        int* reg = direccion_registro(contexto_interno, ins_actual->arg1);
        if (*reg != 0) {contexto_interno->PC = atoi(ins_actual->arg2);}
        else {contexto_interno->PC++;}
        break;

    case MOV_IN:
        log_info(logger,"PID: %d - Ejecutando: MOV_IN - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        break;

    case MOV_OUT:
        log_info(logger,"PID: %d - Ejecutando: MOV_OUT - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        break;

    case RESIZE:
        log_info(logger,"PID: %d - Ejecutando: RESIZE - %s", PID, ins_actual->arg1);
        break;

    case COPY_STRING:
        log_info(logger,"PID: %d - Ejecutando: COPY_STRING - %s", PID, ins_actual->arg1);
        break;
        
    case IO_GEN_SLEEP:
        log_info(logger,"PID: %d - Ejecutando: IO_GEN_SLEEP - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_GEN_SLEEP;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_STDIN_READ:
        log_info(logger,"PID: %d - Ejecutando: IO_STDIN_READ - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDIN;
        
        char* direccion_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(direccion_r, "%d", *registro);
        
        char* tamanio_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_r, "%d", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, direccion_r, tamanio_r);
        recibir_proceso();
        break;

    case IO_STDOUT_WRITE:
        log_info(logger,"PID: %d - Ejecutando: IO_STDOUT_WRITE - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDOUT;

        char* direccion_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(direccion_w, "%d", *registro);
        
        char* tamanio_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_w, "%d", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, direccion_w, tamanio_w);
        recibir_proceso();
        break;

    case IO_FS_CREATE:
        log_info(logger,"PID: %d - Ejecutando: IO_FS_CREATE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_CREATE;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_FS_DELETE:
        log_info(logger,"PID: %d - Ejecutando: IO_FS_DELETE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_DELETE;
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        recibir_proceso();
        break;

    case IO_FS_TRUNCATE:
        log_info(logger,"PID: %d - Ejecutando: IO_STDIN_READ - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_TRUNCATE;

        char* tamanio_t = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg1);
        sprintf(tamanio_t, "%d", *registro);

        enviar_CE_con_3_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, tamanio_t);
        recibir_proceso();
        break;

    case IO_FS_WRITE:
        log_info(logger,"PID: %d - Ejecutando: IO_FS_WRITE - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_WRITE;

        char* direccion_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg3);
        sprintf(direccion_FS_w, "%d", *registro);
        
        char* tamanio_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg4);
        sprintf(tamanio_FS_w, "%d", *registro);

        char* puntero_FS_w = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg5);
        sprintf(puntero_FS_w, "%d", *registro);

        enviar_CE_con_5_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_FS_w, tamanio_FS_w, puntero_FS_w);
        recibir_proceso();
        break;

    case IO_FS_READ:
        log_info(logger,"PID: %d - Ejecutando: IO_FS_READ - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_READ;

        char* direccion_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg3);
        sprintf(direccion_FS_r, "%d", *registro);
        
        char* tamanio_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg4);
        sprintf(tamanio_FS_r, "%d", *registro);

        char* puntero_FS_r = string_new();
        registro = direccion_registro(contexto_interno, ins_actual->arg5);
        sprintf(puntero_FS_r, "%d", *registro);

        enviar_CE_con_5_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_FS_r, tamanio_FS_r, puntero_FS_r);
        recibir_proceso();
        break;

    case WAIT:
        log_info(logger,"PID: %d - Ejecutando: WAIT - %s", PID, ins_actual->arg1);
        enviar_CE_con_1_arg(DESALOJO_POR_WAIT, ins_actual->arg1);
        if (esperar_respuesta_recurso())
        {
            log_info(logger,"PID: %d - WAIT de recurso: %s fue exitoso", PID, ins_actual->arg1);
            recibir_proceso();
            contexto_interno->PC++;
        }
        else
        {
            log_info(logger,"PID: %d - WAIT de recurso: %s fallo", PID, ins_actual->arg1);
            recibir_proceso();
        }
        break;

    case SIGNAL:
        log_info(logger,"PID: %d - Ejecutando: SIGNAL - %s", PID, ins_actual->arg1);
        enviar_CE_con_1_arg(DESALOJO_POR_SIGNAL, ins_actual->arg1);
        if (esperar_respuesta_recurso())
        {
            log_info(logger,"PID: %d - SIGNAL de recurso: %s fue exitoso", PID, ins_actual->arg1);
            recibir_proceso();
            contexto_interno->PC++;
        }
        else
        {
            log_info(logger,"PID: %d - SIGNAL de recurso: %s fallo", PID, ins_actual->arg1);
            recibir_proceso();
        }
        break;

    case EXIT:
        log_info(logger,"PID: %d - Ejecutando: EXIT", PID);
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

bool check_interrupt(int_code interrupcion){
    return (interrupcion != INT_NO);
}
