#include "../include/entradasalida-kernel.h"
/*

void atender_conexion_entradasalida_KERNEL(){
    log_info(logger, "Inicio conexion entre IO y Kernel");
    op_code codigo;

    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer;
    uint32_t PID;
    char* nombre_interfaz;
    char* tiempo_unidad_trabajo;

    while (true)
    {
        codigo = recibir_operacion(socket_kernel_entradasalida);

        switch (codigo)
        {
        case DESALOJO_POR_IO_GEN_SLEEP:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);            
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            tiempo_unidad_trabajo = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "Datos recibidos: PID: %d, nombre: %s, tiempo: %d", PID, nombre_interfaz, atoi(tiempo_unidad_trabajo));//Solo para pruebas

            log_info(logger, "PID: %d - Operacion: IO_GEN_SLEEP", PID);
            // crear_interfaz_generica(PID, nombre_interfaz, tiempo_unidad_trabajo);
            break;
        
        default:
            break;
        }


    }
    

}
*/