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

    
     
    
    }

void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido){

//LE DOY VALOR A LAS VARIABLES A ENVIAR

    char** leido_array = string_split(leido, " ");  //SEPARO EL STRING LEIDO EN UN VECTOR DE STRING: 
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






