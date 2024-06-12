#include "../include/CPU-kernel.h"

void recibir_proceso(){
    recibir_CE(socket_cpu_kernel_dispatch, &PID, &contexto_interno);
    interrupcion = INT_NO;
}

void desalojar_proceso(){
    t_paquete* paquete = crear_paquete(RECIBIR_CE_DISPATCH);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_int_code(paquete, interrupcion);
    serializar_CE(paquete, contexto_interno);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
}
