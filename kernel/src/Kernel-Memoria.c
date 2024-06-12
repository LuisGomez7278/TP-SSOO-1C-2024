#include "../include/Kernel-Memoria.h"


    void atender_conexion_MEMORIA_KERNELL(){

//RECIBIR MENSAJE DE MEMORIA
    op_code codop = recibir_operacion(socket_memoria_kernel);
    if (codop == MENSAJE) {printf("LLego un mensaje\n");}
    else {printf("LLego otra cosa");}
    recibir_mensaje(socket_memoria_kernel, logger);

//ENVIAR MENSAJE A MEMORIA
    enviar_mensaje("Kernel manda mensaje a memoria", socket_memoria_kernel);
    log_info(logger, "Se envio el primer mensaje a memoria");
        
        bool continuarIterando=1;
       

        while (continuarIterando) {
            int cod_op = recibir_operacion(socket_memoria_kernel);   ////se queda esperando en recv por ser bloqueante
            switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(socket_memoria_kernel,logger_debug);
                break;
            case CARGA_EXITOSA_PROCESO:
                carga_exitosa_en_memoria();
                break;
            case -1:
                log_error(logger_debug, "el MODULO DE KERNEL SE DESCONECTO. Terminando servidor");
                continuarIterando=0;
                break;
            default:
                log_warning(logger_debug,"Operacion desconocida de KERNEL. No quieras meter la pata");
                break;
            }
        }


    
     
    
    }

void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido){


    char** leido_array = string_split(leido, " ");  //SEPARO EL STRING LEIDO EN UN VECTOR DE STRING: 
    
//LE DOY VALOR A LAS VARIABLES A ENVIAR    

    //estructura: codigo operacion, pid, path_para_memoria
    op_code codigo_de_operacion=CREAR_PROCESO;
    char* path_para_memoria=leido_array[1];
    int tamanio=strlen(path_para_memoria);                      //--------------LO GUARDO EN UN INT DE 32 BITS PORQUE EL STRLEN DEVUELVE 64 BITS 

//PREAPARO EL STREAM DE DATOS, LOS SERIALIZO Y ENVIO

    t_paquete *paquete= crear_paquete (codigo_de_operacion);
    agregar_a_paquete_uint32(paquete,PID);
    agregar_a_paquete_string(paquete,tamanio,path_para_memoria);
    enviar_paquete(paquete,socket_memoria_kernel);              //--------------ESTA FUNCION SERIALIZA EL PAQUETE ANTES DE ENVIARLO --quedaria un void*= cod_op||SIZE TOTAL||size path|| path
    eliminar_paquete(paquete);


}

void carga_exitosa_en_memoria(){  ///PID Y PATH PARCIAL SON LOS QUE SIRVEN PARA CARGAR EL PROCESO

    uint32_t *sizeTotal=malloc(sizeof(uint32_t));
    int *desplazamiento=malloc(sizeof(int));
    *desplazamiento=0;
    void* buffer= recibir_buffer(sizeTotal,socket_memoria_kernel);
    
   // verificar_paquete(buffer);

    if (buffer != NULL) {
        uint32_t tam_buffer=leer_de_buffer_uint32(buffer,desplazamiento);
        uint32_t PID= leer_de_buffer_uint32(buffer,desplazamiento);
        


        log_info(logger_debug,"CARGA EXITOSA DEL PROCESO: PID= %u  tam_buffer= %u  ",PID,tam_buffer);
        
        free(sizeTotal);
        free(desplazamiento);
        free(buffer);
    
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
}




