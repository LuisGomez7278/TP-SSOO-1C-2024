#include "../include/entradasalida_Main.h"

int main(int argc, char* argv[]) {
    iniciar_entradasalida();

    // socket_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria);

    socket_entradasalida_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);

    atender_conexion_entradasalida_KERNEL();

    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}

    return 0;
}
