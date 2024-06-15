#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <commons/collections/list.h>
#include<readline/readline.h>
#include <semaphore.h>
#include <pthread.h>






typedef enum instrucciones
{
	SET,
	SUM,
	SUB,
	MOV_IN,
	MOV_OUT,
    RESIZE,
    JNZ,
	COPY_STRING,
	IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
	WAIT,
	SIGNAL,
	EXIT
} cod_ins;

typedef struct instruccion
{
    cod_ins ins;
    char* arg1;
    char* arg2;
    char* arg3;
    char* arg4;
    char* arg5;
} t_instruccion;

typedef enum 
{
    NUEVO,
    LISTO,
    EJECUCION,
    BLOQUEO,
    EXITT,
    ERROR
} t_estado;

typedef struct contexto_de_ejecucion_de_proceso
{
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_contexto_ejecucion;

typedef struct pcb_de_proceso
{
   	uint32_t PID;
    t_estado estado;
    uint32_t quantum;
    t_contexto_ejecucion CE;
} t_pcb;

//Funciones de Utils.
void decir_hola(char* );
void read_console(t_log *);
t_config *start_config(char* path);
t_log *start_logger(char* fileName, char* processName, t_log_level logLevel);
void end_program(t_log* logger, t_config* config);

#endif
