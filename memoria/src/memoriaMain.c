#include "../include/memoriaMain.h"

int main(int argc, char* argv[]) {
    
// INICIALIZO LOGs
    inciarlogs();

// OBTENGO LOS VALORES DEL CONFIG
    cargarConfig();

// INICIALIZO EL ESPACIO DE USUARIO DE MEMORIA (PAGINACION)
    inicializarEspacioMem();                                                           
    
// INICIO SERVIDOR MEMORIA
    socket_escucha = iniciar_servidor(puerto_escucha, logger);

 // PRUEBAS SOBRE FUNCIONAMIENTO 
 /*
    uint32_t n = 2; int i=0; char* leido = string_new(); 
    bool crear = crear_procesoM(path_base, 1);
    bool a = resize(1,69); bool escrito = true;

    while((i<n) && (escrito)){
        if(i==0)escrito = escribir_memoria(62, 2, "20", 1);

        if(i==1)escrito = escribir_memoria(64, 2, "24", 1);
        ++i;
    }
   
    i=0;
    while(i<n){
        char* leido2;
        if(i==0) {
        leido2 = leer_memoria(62, 2, 1); log_info(logger_debug, "Leido2: %s", leido2);
        }
        if(i==1) {
        leido2 = leer_memoria(64, 2, 1); log_info(logger_debug, "Leido2: %s", leido2);
        }
        string_append(&leido, leido2);
        free(leido2);
        ++i;
    } 
    char* leer = leer_memoria(62, 2, 1);
    log_info(logger_debug, "leer: %s", leer);
    uint32_t num = (uint32_t)strtoul(leido, NULL, 10); //convierte el char* leido a un uint32_t
    log_info(logger_debug, "Uint: %u", num);

    bool crear2 = crear_procesoM(path_base, 10);
    eliminar_procesoM(10);


    resize(1, 67);

    tabla_pag_proceso* tpg = obtener_tabla_pag_proceso(1);

    if(tpg == NULL){
        perror("AAA");    
    }

    char* buffer = "Hola planeta tierra, hoy es lunes";

    bool escribir_bien = escribir_memoria(50, 14, buffer, 1);
    bool escribir_bien2 = escribir_memoria(64, 20, "Hola", 1);

    //bool escribir_bien2 = escribir_memoria(10, 3, "TP", 1);
    char *leido2 = string_new();
    int i=0; int n=2; 

    leido2 = leer_memoria(50, 14, 1);
    log_info(logger_debug, "%s", leido2);
    char* leido = leer_memoria(64, 20, 1);
    log_info(logger_debug, "%s", leido);

    strcat(leido2, leido);

    log_info(logger_debug, "%s", leido2);

    while(i<n){
    char* leido;
    if(i==0){leido = leer_memoria(50, 14, 1);}
    if(i==1){leido = leer_memoria(64, 20, 1);}
    
    //strcat(leido2, leido);
    string_append(&leido2, leido);
    //log_info(logger_debug, "%s", leido2);
    free(leido);
    ++i;
    }
    log_info(logger_debug, "%s", leido2);

    // if(escribir_bien){log_info(logger_debug, "Perfecto");}

    char str[12];
    uint32_t numero = 145;

    sprintf(str, "%u", numero); //convierte uint32_t a char*

    bool escrito = escribir_memoria(10, sizeof(numero), str, 1);

    char* leido = leer_memoria(10, 10, 1);
    
    log_info(logger_debug, "%s", leido);
    
    //log_info(logger_debug, "Int Leido: %d", leido_int);
    if(leido==NULL){perror("Rompio");}

    free(leido);
*/

// ESPERO QUE SE CONECTE CPU
    log_trace(logger_debug, "Esperando que se conecte CPU");
    socket_cpu_memoria = esperar_cliente(socket_escucha,logger_debug);
    enviar_tam_pag();
    
// ESPERO QUE SE CONECTE EL KERNEL
    log_trace(logger_debug, "Esperando que se conecte KERNEL");
    socket_kernel_memoria = esperar_cliente(socket_escucha,logger_debug);

// ESPERO QUE SE CONECTE E/S
    log_trace(logger_debug, "Esperando que se conecte E/S");
    socket_entradasalida_memoria = esperar_cliente(socket_escucha,logger_debug);

// CREO HILO ESCUCHA CPU
    pthread_t hilo_cpu_memoria;
    pthread_create(&hilo_cpu_memoria,NULL,(void*)conexion_con_cpu,NULL);
    pthread_detach(hilo_cpu_memoria);

// CREO HILO ESCUCHA KERNEL 
    pthread_t hilo_kernel_memoria;
    pthread_create(&hilo_kernel_memoria,NULL,(void*)conexion_con_kernel,NULL);
    pthread_join(hilo_kernel_memoria, NULL); 
    
// CREO HILO ESCUCHA ENTRADA-SALIDA
    // pthread_t hilo_entradaSalida_memoria;
    // pthread_create(&hilo_entradaSalida_memoria,NULL,(void*)conexion_con_es,NULL);
    // pthread_detach(hilo_entradaSalida_memoria);

    //--------------------------------------------


    if (socket_cpu_memoria) {liberar_conexion(socket_cpu_memoria);}
    if (socket_kernel_memoria) {liberar_conexion(socket_kernel_memoria);}
    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_escucha) {liberar_conexion(socket_escucha);}

    return 0;
}

//-------------------------------         FUNCIONES     -----------------------------------------

t_list* leer_pseudocodigo(char* path){
    FILE* archivo =  fopen(path, "r");
    t_list* lista_instrucciones = list_create();
    t_instruccion* instr;

    if (archivo == NULL) {
        log_error(logger_debug, "No se pudo abrir archivo de pseudocodigo");
        return (t_list*) NULL;    
    }

    char linea[50];
    memset(linea, 0, 50);
    while (fgets(linea, 50, archivo) != NULL)
    {
        instr = parsear_instruccion(linea);
        if (!instr) 
        {
            log_error(logger_debug, "El archivo de pseudocodigo tiene errores/instrucciones invalidas");
            return (t_list* ) NULL;
            break;
        }
        list_add(lista_instrucciones, instr);
    }

    fclose(archivo);
    log_info(logger_debug, "Archivo pseudocodigo leido, cantidad lineas leidas: [%d]", list_size(lista_instrucciones));

    return lista_instrucciones;
}

t_instruccion* parsear_instruccion(char* linea){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    char* ins;
    char* a1;
    char* a2;
    char* a3;
    char* a4;
    char* a5;

    char** tokens = string_split(linea, " ");

    ins = tokens[0];    
    switch (hash_ins(ins))
    {
    // 5 argumentos
    case IO_FS_WRITE:
    case IO_FS_READ:
        if (string_array_size(tokens)!=6)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            return (t_instruccion* ) NULL;
            break;
        }
        a1 = tokens[1];
        a2 = tokens[2];
        a3 = tokens[3];
        a4 = tokens[4];
        a5 = tokens[5];

        string_trim(&a1);
        string_trim(&a2);
        string_trim(&a3);
        string_trim(&a4);
        string_trim(&a5);

        instruccion->ins = hash_ins(ins);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;

    // 3 argumentos
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
    case IO_FS_TRUNCATE:
        if (string_array_size(tokens)!=4)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            return (t_instruccion* ) NULL;
            break;
        }
        a1 = tokens[1];
        a2 = tokens[2];
        a3 = tokens[3];
        a4 = string_new();
        a5 = string_new();

        string_trim(&a1);
        string_trim(&a2);
        string_trim(&a3);

        instruccion->ins = hash_ins(ins);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;

    // 2 argumentos
    case SET: 
    case SUM:
    case SUB:
    case MOV_IN:
    case MOV_OUT:
    case JNZ:
    case IO_GEN_SLEEP:
    case IO_FS_CREATE:
    case IO_FS_DELETE:
        if (string_array_size(tokens)!=3)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            return (t_instruccion* ) NULL;
            break;
        }
        a1 = tokens[1];
        a2 = tokens[2];
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();

        string_trim(&a1);
        string_trim(&a2);

        instruccion->ins = hash_ins(ins);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;    

    // 1 argumento
    case RESIZE:
    case COPY_STRING:
    case WAIT:
    case SIGNAL:
        if (string_array_size(tokens)!=2)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            return (t_instruccion* ) NULL;
            break;
        }    
        a1 = tokens[1];
        a2 = string_new();
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();
        string_trim(&a1);
        
        instruccion->ins = hash_ins(ins);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;
    case EXIT: // 0 argumentos, solo EXIT
        if (string_array_size(tokens)!=1)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            return (t_instruccion* ) NULL;
            break;
        }
        a1 = string_new();
        a2 = string_new();
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();

        instruccion->ins = hash_ins(ins);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;   
    default:
        log_error(logger_debug, "Instruccion no reconocida");
        return (t_instruccion* ) NULL;
        break;
    }
    return instruccion;
}

cod_ins hash_ins(char* ins){
    if (string_equals_ignore_case(ins, "SET")){return SET;}
    else if (string_equals_ignore_case(ins, "SUM")){return SUM;}
    else if (string_equals_ignore_case(ins, "SUB")){return SUB;}
    else if (string_equals_ignore_case(ins, "MOV_IN")){return MOV_IN;}
    else if (string_equals_ignore_case(ins, "MOV_OUT")){return MOV_OUT;}
    else if (string_equals_ignore_case(ins, "RESIZE")){return RESIZE;}
    else if (string_equals_ignore_case(ins, "JNZ")){return JNZ;}
    else if (string_equals_ignore_case(ins, "COPY_STRING")){return COPY_STRING;}
    else if (string_equals_ignore_case(ins, "IO_GEN_SLEEP")){return IO_GEN_SLEEP;}
    else if (string_equals_ignore_case(ins, "IO_STDIN_READ")){return IO_STDIN_READ;}
    else if (string_equals_ignore_case(ins, "IO_STDOUT_WRITE")){return IO_STDOUT_WRITE;}
    else if (string_equals_ignore_case(ins, "IO_FS_CREATE")){return IO_FS_CREATE;}
    else if (string_equals_ignore_case(ins, "IO_FS_DELETE")){return IO_FS_DELETE;}
    else if (string_equals_ignore_case(ins, "IO_FS_TRUNCATE")){return IO_FS_TRUNCATE;}
    else if (string_equals_ignore_case(ins, "IO_FS_WRITE")){return IO_FS_WRITE;}
    else if (string_equals_ignore_case(ins, "IO_FS_READ")){return IO_FS_READ;}
    else if (string_equals_ignore_case(ins, "WAIT")){return WAIT;}
    else if (string_equals_ignore_case(ins, "SIGNAL")){return SIGNAL;}
    else if (string_equals_ignore_case(ins, "EXIT")){return EXIT;}
    else return -1;
}

char* path_completo(char* path_base, char* path_parcial){
    char* path = string_new();
    string_append(&path, path_base);
    string_append(&path, path_parcial);

    return path;
}

t_instruccion* get_ins(t_list* lista_instrucciones, uint32_t PC){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion*));
    instruccion =  list_get(lista_instrucciones, PC);
    return instruccion;
}

void enviar_tam_pag(){
    uint32_t tam_pag = tam_pagina;
    t_paquete* paquete = crear_paquete(TAM_PAG);
    agregar_a_paquete_uint32(paquete, tam_pag);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);            
    log_info(logger_debug, "Se envia el tamaño de pagina a CPU");
}