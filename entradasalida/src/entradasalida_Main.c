#include "../include/entradasalida_Main.h"

int32_t main(int32_t argc, char* argv[]) {


        if (argc < 3) {
        fprintf(stderr, "Uso: %s <nombre_interfaz> <config_interfaz>\n", argv[0]);
        return 1;
        }

        printf("Argumento 1: %s\n", argv[1]);
        printf("Argumento 2: %s\n", argv[2]);
    
  
    //VALIDO ARGUMENTOS
    validar_argumentos(argv[1],argv[2]);
    

    nombre_interfaz = argv[1];
    config_interfaz = argv[2];

    //nombre_interfaz="INTFZ3";
    //config_interfaz = "IO-GEN1";


    iniciar_entradasalida(nombre_interfaz, config_interfaz);


    socket_kernel_entradasalida = crear_conexion(IP_KERNEL,PUERTO_KERNEL);
    log_info(logger, "Se creo la conexion entre IO y Kernel");

    enviar_mensaje("CONEXION CON INTERFAZ OK", socket_kernel_entradasalida);
    log_info(logger_debug, "Handshake enviado: KERNEL")    ;

    
    recibir_operacion(socket_kernel_entradasalida);
    recibir_mensaje(socket_kernel_entradasalida, logger);

    //Envio nombre y tipo a kernel:::  op_code (nueva IO) ||  cod_interfaz tipo interfaz || string nombre interfaz
    t_paquete* paquete= crear_paquete(NUEVA_IO);
    cod_interfaz interfaz = get_tipo_interfaz(TIPO_INTERFAZ);
    agregar_a_paquete_cod_interfaz(paquete,interfaz);
    
    uint32_t tamanio=string_length(nombre_interfaz)+1;
    agregar_a_paquete_string(paquete,tamanio,nombre_interfaz);
    enviar_paquete(paquete,socket_kernel_entradasalida);
    eliminar_paquete(paquete);
    log_debug(logger_debug,"Se confirma a kernel la creacion de la IO");

    if (interfaz!=GENERICA)
    {
        socket_memoria_entradasalida = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
        log_info(logger, "Se creo la conexion entre IO y MEMORIA");

        enviar_mensaje("CONEXION CON INTERFAZ OK", socket_memoria_entradasalida);
        log_info(logger_debug, "Handshake enviado: KERNEL")    ;
        
    }
    

    if (string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS"))
    {
        inicializar_FS();
    }
    else {bloques = NULL;}

    bool continuarIterando = true;
    op_code cod_op;
    op_code verificacion;
    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer;

    uint32_t PID;
    char* nombre_archivo;
    uint32_t tamanio_total;
    uint32_t cant_accesos;
    uint32_t dir_fisica;
    uint32_t tamanio_a_leer;

    uint32_t puntero;
    char* path_archivo_metadata;
    t_config* metadata;
    uint32_t tamanio_archivo;
    uint32_t bloque_inicial;
    uint32_t acumulador;

    op_code exito_io = SOLICITUD_EXITOSA_IO;
    op_code error_io = ERROR_SOLICITUD_IO;

    
    while (continuarIterando) {
        verificacion = recibir_operacion(socket_kernel_entradasalida);
        log_debug(logger_debug, "Cod verificar: %d", verificacion);
        
        cod_op = recibir_operacion(socket_kernel_entradasalida);
        log_debug(logger_debug, "Cod operacion: %d", cod_op);

        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_kernel_entradasalida, logger);
            break;
        case DESALOJO_POR_IO_GEN_SLEEP:
            uint32_t unidades_trabajo;
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            unidades_trabajo = atoi(leer_de_buffer_string(buffer, &desplazamiento));
            log_info(logger,"PID: %u - Operacion: IO_GEN_SLEEP", PID);
            
            free(buffer);
            sleep(unidades_trabajo);
            // notificar_kernel(PID);
            send(socket_kernel_entradasalida, &exito_io, sizeof(op_code), 0);
            log_trace(logger, "PID: %u - Finaliza GEN_SLEEP", PID);
            break;
        case DESALOJO_POR_IO_STDIN:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDIN_READ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            char* string_leida = leer_de_teclado(tamanio_total);
            
            paquete = crear_paquete(SOLICITUD_IO_STDIN_READ);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);

            acumulador = 0;
            for (int i = 0; i < cant_accesos; i++)
            {
                uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                uint32_t tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_string(paquete, tamanio_a_leer, string_leida+acumulador);
                acumulador+=tamanio_a_leer;
            }
            free(buffer);
            free(string_leida);

            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);
            // notificar_kernel(PID);
            break;

        case DESALOJO_POR_IO_STDOUT:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDOUT_WRITE ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            // Pedir lectura de string a memoria
            paquete = crear_paquete(SOLICITUD_IO_STDOUT_WRITE);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            
            for (int i = 0; i < cant_accesos; i++)
            {
                dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
            }
            free(buffer);

            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);
            
            sem_wait(&respuesta_memoria);
            // // Enviar string a kernel para que lo imprima
            // paquete = crear_paquete(DESALOJO_POR_IO_STDOUT);
            // agregar_a_paquete_string(paquete, tamanio_total, string_leida_memoria);
            // enviar_paquete(paquete, socket_kernel_entradasalida);
            // eliminar_paquete(paquete);
            // log_info(logger, "Se envio la string \'%s\', a kernel para que sea imprimida en pantalla", string_leida_memoria);
            // Imprimir por pantalla
            printf("%s", string_leida_memoria);
            free(string_leida_memoria);
            break;

        case DESALOJO_POR_IO_FS_CREATE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_CREATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Crear Archivo: %s", PID, nombre_archivo);
            crear_archivo(nombre_archivo);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_DELETE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_DELETE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Eliminar Archivo: %s", PID, nombre_archivo);
            eliminar_archivo(nombre_archivo);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_TRUNCATE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_TRUNCATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            int32_t nuevo_tamanio = leer_de_buffer_uint32(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Truncar Archivo: %s", PID, nombre_archivo);
            truncar_archivo(PID, nombre_archivo, nuevo_tamanio);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_WRITE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            puntero = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            // Pedir lectura de string a memoria
            paquete = crear_paquete(DESALOJO_POR_IO_FS_WRITE);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            
            for (int i = 0; i < cant_accesos; i++)
            {
                dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
            }
            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);

            // Esperar respuesta y grabarla en disco
            sem_wait(&respuesta_memoria);

            path_archivo_metadata = string_duplicate(path_metadata);
            string_append(&path_archivo_metadata, nombre_archivo);
            metadata = config_create(path_archivo_metadata);
            tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
            bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

            log_info(logger, "PID: %u - Escribir Archivo: %s - Tamaño a Escribir: %u - Puntero Archivo: %u", PID, nombre_archivo, tamanio_total, puntero);
            if (puntero+tamanio_total >= tamanio_archivo)
            {
                log_error(logger, "PID: %u trato de escribir en disco al archivo: %s mas alla de su tamaño asignado", PID, nombre_archivo);
            }
            else
            {
                FS_WRITE(bloques, bloque_inicial, puntero, tamanio_total, string_leida_memoria);
            }
            
            config_destroy(metadata);
            free(nombre_archivo);
            free(string_leida_memoria);
            break;

        case DESALOJO_POR_IO_FS_READ:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            puntero = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);

            path_archivo_metadata = string_duplicate(path_metadata);
            string_append(&path_archivo_metadata, nombre_archivo);
            metadata = config_create(path_archivo_metadata);
            tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
            bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

            log_info(logger, "PID: %u - Escribir Archivo: %s - Tamaño a Escribir: %u - Puntero Archivo: %u", PID, nombre_archivo, tamanio_total, puntero);
            void* datos_leidos;
            if (puntero+tamanio_total >= tamanio_archivo)
            {
                log_error(logger, "PID: %u trato de leer de disco al archivo: %s mas alla de su tamaño asignado", PID, nombre_archivo);
            }
            else
            {
                datos_leidos = malloc(tamanio_total);
                FS_READ(bloques, bloque_inicial, puntero, tamanio_total, datos_leidos);
            }
            
            config_destroy(metadata);

            t_paquete* paq = crear_paquete(DESALOJO_POR_IO_FS_READ);
            agregar_a_paquete_uint32(paq, tamanio_total);
            agregar_a_paquete_uint32(paq, cant_accesos);

            acumulador = 0;
            for (int i = 0; i < cant_accesos; i++)
            {
                dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_string(paquete, tamanio_a_leer, datos_leidos+acumulador);
                acumulador+=tamanio_a_leer;
            }
            enviar_paquete(paq, socket_memoria_entradasalida);
            eliminar_paquete(paq);
            free(datos_leidos);
            free(nombre_archivo);
            break;

        case FALLO:
            log_error(logger, "La ENTRADASALIDA SE DESCONECTO. Terminando servidor");
            continuarIterando=0;
            break;

        case VERIFICAR_CONEXION:
            log_info(logger, "Kernel pide verificar la conexion");
            break;

        default:
            log_warning(logger,"Operacion desconocida de ENTRADA Y SALIDA. Codigo: %d", cod_op);
            break;
        
        }

    free(buffer);

    }

    //if (bloques) {munmap(bloques, BLOCK_SIZE*BLOCK_COUNT);}
    //if (array_bitmap) {munmap(array_bitmap, BLOCK_COUNT/8);}
    if (socket_memoria_entradasalida) {liberar_conexion(socket_memoria_entradasalida);}
    if (socket_kernel_entradasalida) {liberar_conexion(socket_kernel_entradasalida);}

    return 0;
}






void validar_argumentos(char* nombre_interfaz, char* config_interfaz)
{
    if(nombre_interfaz == NULL || config_interfaz == NULL){
        printf("Agregar argumentos 'nombre_interfaz' y 'config_interfaz'");
        exit(EXIT_FAILURE);
    }
    //Verificar si puede venir otro nombre en las pruebas
  //  if(strcmp(nombre_interfaz,"GENERICA") != 0 && strcmp(nombre_interfaz,"STDOUT") != 0 && strcmp(nombre_interfaz,"STDIN") !=0){
  //       printf("Utilizar 'GENERICA', 'STDOUT' o 'STDIN' como nombre de interfaz");
  //      exit(EXIT_FAILURE);
  //  }
}

void notificar_kernel(uint32_t PID)
{
    t_paquete* paquete = crear_paquete(SOLICITUD_EXITOSA_IO);
    agregar_a_paquete_uint32(paquete, PID);
    enviar_paquete(paquete, socket_kernel_entradasalida);
    eliminar_paquete(paquete);
}

char* leer_de_teclado(uint32_t tamanio_a_leer)
{
    char* string_a_memoria = string_new();
    char* leido = malloc(tamanio_a_leer);
    uint32_t restante = tamanio_a_leer;

    //Primera lectura de teclado
    char* mensaje_mostrado = malloc(25);
    sprintf(mensaje_mostrado, "Ingresar %d caracteres", tamanio_a_leer);
    leido = readline(mensaje_mostrado);
    string_append(&string_a_memoria, leido);
    restante-=string_length(leido);
    free(mensaje_mostrado);
    free(leido);

    //Si lo leido no ocupa todo el tamanio_a_leer sigue pidiendo datos hasta completar
    while (string_length(string_a_memoria)<restante)
    {
        mensaje_mostrado = malloc(25);
        sprintf(mensaje_mostrado, "Ingresar %d caracteres", restante);
        leido = readline(mensaje_mostrado);
        string_append(&string_a_memoria, leido);
        restante-=string_length(leido);

        free(mensaje_mostrado);
        free(leido);
    }
    
    //Verifica que no se pase del tamaño pedido
    string_substring_until(string_a_memoria, tamanio_a_leer+1);//Probar si hace falta el +1

    return string_a_memoria;
}
