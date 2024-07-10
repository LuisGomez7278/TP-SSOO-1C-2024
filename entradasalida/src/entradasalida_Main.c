#include "../include/entradasalida_Main.h"

int32_t main(int32_t argc, char* argv[]) {

    //VALIDO ARGUMENTOS
    //validar_argumentos(argv[1],argv[2]);
    
    //nombre_interfaz = argv[1];
    //config_interfaz = argv[2];

    nombre_interfaz = "Generica";
    config_interfaz = "entradasalida.config";

    iniciar_entradasalida(nombre_interfaz, config_interfaz);

    socket_entradasalida_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(logger, "Se creo la conexion entre IO y Memoria");
    socket_entradasalida_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
    log_info(logger, "Se creo la conexion entre IO y Kernel");

    if (string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS"))
    {
        inicializar_FS();
    }
    

    bool continuarIterando = true;
    while (continuarIterando) {
        uint32_t PID;

        uint32_t size;
        void* buffer;
        uint32_t desplazamiento = 0;
        t_paquete* paquete;
        op_code cod_op = recibir_operacion(socket_entradasalida_kernel);

        uint32_t tamanio_total;
        uint32_t cant_accesos;

        char* nombre_archivo;

        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_entradasalida_kernel, logger);
            break;
        case DESALOJO_POR_IO_GEN_SLEEP:
            uint32_t unidades_trabajo;
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            unidades_trabajo = atoi(leer_de_buffer_string(buffer, &desplazamiento));
            log_info(logger,"PID: %u - Operacion: IO_GEN_SLEEP", PID);
            
            free(buffer);
            usleep(unidades_trabajo);
            notificar_kernel(PID);
            break;
        case DESALOJO_POR_IO_STDIN:
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDIN_READ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            char* string_leida = leer_de_teclado(tamanio_total);
            
            paquete = crear_paquete(SOLICITUD_IO_STDIN_READ);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);

            uint32_t acumulador = 0;
            for (int i = 0; i < cant_accesos; i++)
            {
                uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                uint32_t tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
                agregar_a_paquete_string(paquete, tamanio_a_leer, string_leida+acumulador);
                acumulador+=tamanio_a_leer;
            }
            free(buffer);

            enviar_paquete(paquete, socket_entradasalida_memoria);
            eliminar_paquete(paquete);
            // notificar_kernel(PID);
            break;

        case DESALOJO_POR_IO_STDOUT:
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDOUT_WRITE ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            paquete = crear_paquete(SOLICITUD_IO_STDOUT_WRITE);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            
            for (int i = 0; i < cant_accesos; i++)
            {
                uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                uint32_t tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
            }
            free(buffer);

            enviar_paquete(paquete, socket_entradasalida_memoria);
            eliminar_paquete(paquete);
            // notificar_kernel(PID);
            break;

        case DESALOJO_POR_IO_FS_CREATE:
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_CREATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            free(buffer);
            log_info(logger, "PID: %u - Crear Archivo: %s", PID, nombre_archivo);
            crear_archivo(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_DELETE:
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_DELETE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            free(buffer);
            log_info(logger, "PID: %u - Eliminar Archivo: %s", PID, nombre_archivo);
            eliminar_archivo(nombre_archivo);
            break;
        case DESALOJO_POR_IO_FS_TRUNCATE:
            buffer = recibir_buffer(&size, socket_entradasalida_kernel);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_TRUNCATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            int32_t nuevo_tamanio = leer_de_buffer_uint32(buffer, &desplazamiento);
            free(buffer);
            log_info(logger, "PID: %u - Truncar Archivo: %s", PID, nombre_archivo);
            truncar_archivo(nombre_archivo, nuevo_tamanio);
            break;
        case DESALOJO_POR_IO_FS_WRITE:
        case DESALOJO_POR_IO_FS_READ:
            
        case FALLO:
            log_error(logger, "La ENTRADASALIDA SE DESCONECTO. Terminando servidor");
            continuarIterando=0;
            break;
        default:
            log_warning(logger,"Operacion desconocida de ENTRADA Y SALIDA. No quieras meter la pata");
            break;
        }
    }

    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}

    return 0;
}

void validar_argumentos(char* nombre_interfaz, char* config_interfaz)
{
    if(nombre_interfaz == NULL || config_interfaz == NULL){
        log_error(logger,"Agregar argumentos 'nombre_interfaz' y 'config_interfaz'");
        exit(EXIT_FAILURE);
    }
    //Verificar si puede venir otro nombre en las pruebas
    if(strcmp(nombre_interfaz,"GENERICA") != 0 && strcmp(nombre_interfaz,"STDOUT") != 0 && strcmp(nombre_interfaz,"STDIN") !=0){
        log_error(logger,"Utilizar 'GENERICA', 'STDOUT' o 'STDIN' como nombre de interfaz");
        exit(EXIT_FAILURE);
    }
}

void notificar_kernel(uint32_t PID)
{
    t_paquete* paquete = crear_paquete(FINALIZA_IO);
    agregar_a_paquete_uint32(paquete, PID);
    enviar_paquete(paquete, socket_entradasalida_kernel);
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
