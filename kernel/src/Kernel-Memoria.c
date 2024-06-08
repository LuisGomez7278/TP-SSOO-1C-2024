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


/*

    typedef struct
{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;


typedef enum{
    CONTEXTO,
	SIG_INS,
	PARA_MEMO,
	TAM_PAG,
    CARGAR_PROCESO,
    ELIMINAR_PROCESO,
    PAGE_FAULT,
	RECIBIR_CE_DISPATCH,
	MENSAJE,
    } op_code;


*/



