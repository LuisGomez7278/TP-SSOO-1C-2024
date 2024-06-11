#include "../include/servicios_kernel.h"
uint32_t identificador_PID = 1;
pthread_mutex_t mutex_pid;

__attribute__((constructor)) void init_mutex() {
    pthread_mutex_init(&mutex_pid, NULL);
}

uint32_t asignar_pid(){
    
    uint32_t valor_pid;

    //pthread_mutex_lock(&mutex_pid);
    valor_pid= identificador_PID;
    identificador_PID++;
    //pthread_mutex_unlock(&mutex_pid);

    return valor_pid;

}