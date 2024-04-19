#include "client.h"

void *serializar_paquete(t_paquete *paquete, int bytes)
{
    void *magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic;
}

int crear_conexion(char *ip, char *puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Ahora vamos a crear el socket.
    int socket_cliente = socket(server_info->ai_family,
                                server_info->ai_socktype,
                                server_info->ai_protocol);

    // Ahora que tenemos el socket, vamos a conectarlo
    if (connect(socket_cliente,
                server_info->ai_addr,
                server_info->ai_addrlen) == -1)
    {
        return -1;
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente, t_log* logger)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);
	// agregado extra (ary) es para saber si el server recibio correctamente el mensaje
	int32_t result;
	recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
	if (result == 0) {
		// Handshake OK
		log_info(logger, "El server recibio el mensaje CORRECTAMENTE");
	} else {
		// Handshake ERROR
		log_info(logger, "El server recibio el mensaje FALLIDO");
	}

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete *paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(void)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente, t_log* logger)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	// agregado extra (ary) es para saber si el server recibio correctamente el paquete
	int32_t result;
	recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);
	if (result == 0) {
		// Handshake OK
		log_info(logger, "El server recibio el paquete CORRECTAMENTE");
	} else {
		// Handshake ERROR
		log_error(logger, "El server recibio el paquete FALLIDO");
	}

	free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}

void paquete(int conexion, t_log* logger)
{
	char* leido;
    
	// Inicializo paquete
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);

	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");
	// Itero hasta que se ingrese un string vacio
	while(strcmp(leido, "") > 0){
		agregar_a_paquete(paquete, leido, strlen(leido) +1);
		leido = readline("> ");
	}
	enviar_paquete(paquete, conexion, logger);

	// Libero memoria
	free(leido);
	eliminar_paquete(paquete);

}