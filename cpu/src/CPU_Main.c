#include "../include/CPU_Main.h"

int main(int argc, char* argv[]) {

    iniciar_CPU();

    // Pruebas con memoria
    // crear conexion
    socket_cpu_memoria = crear_conexion(ip_memoria, puerto_memoria);

    // //Pruebas con kernel
    // //iniciar Server de CPU
    socket_escucha = iniciar_servidor(puerto_escucha_dispatch, logger);

    // //esperar conexion de kernel
    socket_cpu_kernel_dispatch = esperar_cliente(socket_escucha, logger);
    socket_cpu_kernel_interrupt = esperar_cliente(socket_escucha, logger);
    // //enviar mensaje a kernel
    // enviar_mensaje("CPU manda mensaje a Kernel", socket_cpu_kernel_dispatch);
    // log_info(logger, "Se envio el primer mensaje a kernel");

    // //recibir respuesta de kernel
    // op_code codop2 = recibir_operacion(socket_cpu_kernel_dispatch);
    
    // if (codop2 == MENSAJE) {log_info(logger, "LLego un mensaje");}
    // else {log_info(logger, "LLego otra cosa");}
    // recibir_mensaje(socket_cpu_kernel_dispatch, logger);
    // recibir_proceso(socket_cpu_kernel_dispatch, &PID, &contexto_interno);


    // t_instruccion* ins_actual = malloc(sizeof(t_instruccion));
    // ins_actual = fetch(PID, contexto_interno.PC, socket_cpu_memoria);
    // ejecutar_instruccion(PID, &contexto_interno, ins_actual);

    while(true){
        t_instruccion* ins_actual = fetch(PID, contexto_interno.PC, socket_cpu_memoria);
        ejecutar_instruccion(PID, &contexto_interno, ins_actual);
        if (check_interrupt(interrupcion, PID, contexto_interno)) {
            desalojar_proceso();
            // enviar_CE(socket_cpu_kernel_dispatch, PID, contexto_interno);
            recibir_proceso(/*socket_cpu_kernel_dispatch, &PID, &contexto_interno*/);
        };
    }

    if (socket_cpu_kernel_dispatch) {liberar_conexion(socket_cpu_kernel_dispatch);}
    if (socket_cpu_kernel_interrupt) {liberar_conexion(socket_cpu_kernel_interrupt);}
    if (socket_cpu_memoria) {liberar_conexion(socket_cpu_memoria);}
    if (socket_escucha) {liberar_conexion(socket_escucha);}

    end_program(logger, config);

    return 0;
}

void ejecutar_instruccion(uint32_t PID, t_contexto_ejecucion* contexto_interno, t_instruccion* ins_actual){
    cod_ins codigo = ins_actual->ins;
    int tamanio_d;
    int tamanio_o;
    int valor;

    switch (codigo)
    {
    case SET:
        log_info(logger,"PID: %d - Ejecutando: SET - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        void* registro = direccion_registro(contexto_interno, ins_actual->arg1);
        int tamanio = tam_registro(ins_actual->arg1);
        valor = atoi(ins_actual->arg2);

        memcpy(registro, &valor, tamanio);
        contexto_interno->PC++;
        break;
    case SUM:
        log_info(logger,"PID: %d - Ejecutando: SUM - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        tamanio_d = tam_registro(ins_actual->arg1);
        tamanio_o = tam_registro(ins_actual->arg2);
        if (tamanio_d != tamanio_o){
            log_error(logger, "SUM trata de sumar registros de distinto tamaño");
            ins_actual->ins = EXIT; //al tener un error, el proceso sale de CPU
            //falta el break a proposito
        }
        else{   
            if (tamanio_d==8){
            uint8_t* registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
            uint8_t* registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
            uint8_t valor8 = *registro_destino + *registro_origen;
            memcpy(registro_destino, &valor8, sizeof(valor8));
            }
            else{
            uint32_t* registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
            uint32_t* registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
            uint32_t valor32 = *registro_destino + *registro_origen;
            memcpy(registro_destino, &valor32, sizeof(valor32));
            }
            contexto_interno->PC++;
            break;
        }
    case SUB:
        log_info(logger,"PID: %d - Ejecutando: SUB - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        tamanio_d = tam_registro(ins_actual->arg1);
        tamanio_o = tam_registro(ins_actual->arg2);
        if (tamanio_d != tamanio_o){
            log_error(logger, "SUB trata de restar registros de distinto tamaño");
            ins_actual->ins = EXIT; //al tener un error, el proceso sale de CPU
            //falta el break a proposito
        }
        else{   
            if (tamanio_d==8){
            uint8_t* registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
            uint8_t* registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
            uint8_t valor = *registro_destino - *registro_origen;
            memcpy(registro_destino, &valor, sizeof(valor));
            }
            else{
            uint32_t* registro_destino = direccion_registro(contexto_interno, ins_actual->arg1);
            uint32_t* registro_origen = direccion_registro(contexto_interno, ins_actual->arg2);
            uint32_t valor = *registro_destino - *registro_origen;
            memcpy(registro_destino, &valor, sizeof(valor));
            }
            contexto_interno->PC++;
            break;
        }
        break;
    case JNZ:
        break;
    case IO_GEN_SLEEP:
        break;
    case EXIT:
        log_info(logger,"PID: %d - Ejecutando: EXIT", PID);
        // enviar_CE(socket_cpu_kernel_dispatch, PID, contexto_interno);
        break;
    default:
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

int tam_registro(char* registro){
    if (!string_equals_ignore_case(registro, "AX")
    ||(!string_equals_ignore_case(registro, "BX"))
    ||(!string_equals_ignore_case(registro, "CX"))
    ||(!string_equals_ignore_case(registro, "DX"))
    )   {return 8;}
    else {return 32;}
}

bool check_interrupt(int_code interrupcion, uint32_t PID, t_contexto_ejecucion contexto_interno){
    if (interrupcion != INT_NO)
    {
        enviar_CE(socket_cpu_kernel_dispatch, PID, contexto_interno);
        return true;
    }
    return false;
}
