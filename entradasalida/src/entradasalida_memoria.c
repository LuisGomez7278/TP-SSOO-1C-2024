#include "../include/entradasalida-memoria.h"

void ejecutar_IO_STDIN(uint32_t marco, uint32_t offset, char* string_leida);


void gestionar_conexion_memoria()
{
    op_code operacion;
    bool continuar_iterando = true;

    while (continuar_iterando)
    {
        operacion = recibir_operacion(socket_entradasalida_memoria);

        uint32_t size;
        uint32_t desplazamiento;
        void* buffer;
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_entradasalida_memoria, logger_debug);
            break;

        case DESALOJO_POR_IO_STDOUT:
            buffer = recibir_buffer(&size, socket_entradasalida_memoria);
            string_leida_memoria = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "Entrada-salida recibe una respuesta de memoria por STDOUT, string: %s", string_leida_memoria);
            sem_post(&respuesta_memoria);
            break;

        case DESALOJO_POR_IO_FS_WRITE:
            buffer = recibir_buffer(&size, socket_entradasalida_memoria);
            string_leida_memoria = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "Entrada-salida recibe una respuesta de memoria por FS_WRITE, string: %s", string_leida_memoria);
            sem_post(&respuesta_memoria);
            break;

        case FALLO:
            log_error(logger, "Modulo MEMORIA desconectado, terminando servidor");
            continuar_iterando = false;
            break;

        default:
            log_error(logger, "Llego una operacion desconocida por socket memoria, op_code: %d", operacion);
            break;
        }
    }
}