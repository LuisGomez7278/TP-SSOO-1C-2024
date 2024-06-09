#include "../include/conexiones.h"

/*----------Cliente----------*/

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);;


	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
/*----------Fin Cliente----------*/

/*----------Servidor----------*/

int iniciar_servidor(char* puerto_escucha, t_log* logger)
{
    int err;

    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, puerto_escucha, &hints, &server_info);

    if (err != 0) {
        perror("Error en getaddrinfo");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    int fd_escucha = socket(server_info->ai_family,
                            server_info->ai_socktype,
                            server_info->ai_protocol);
    if (fd_escucha == -1) {
        perror("Error al crear el socket del servidor");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }
    // Asociamos el socket a un puerto
    err = bind(fd_escucha, server_info->ai_addr, server_info->ai_addrlen);
    if (err == -1) {
        perror("Error al enlazar el socket del servidor");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Escuchamos las conexiones entrantes
    err = listen(fd_escucha, SOMAXCONN);
    if (err == -1) {
        perror("Error al escuchar las conexiones entrantes");
        log_error(logger, "Error al escuchar las conexiones entrantes");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);
    log_info(logger, "Listo para escuchar a mi cliente   %d", err);

    return fd_escucha;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{
    // Aceptamos un nuevo cliente
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente == -1) {
        perror("Error en accept");
        log_error(logger, "Error en accept: %s", strerror(errno));
        return -1;
    }
    // Registramos un mensaje informativo indicando que se ha conectado un cliente
    log_info(logger, "Se conectó un cliente. Socket del cliente: %d", socket_cliente);
    return socket_cliente;
}

/*----------Fin Servidor----------*/


/*----------Mensajeria----------*/

 void enviar_mensaje(char* mensaje, int socket)
 {
 	t_paquete* paquete = crear_paquete(MENSAJE);
    uint32_t tamanio = string_length(mensaje)+1;
    agregar_a_paquete_string(paquete, tamanio, mensaje);

    enviar_paquete(paquete, socket);	
	eliminar_paquete(paquete);
 }

t_paquete* crear_paquete(op_code codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(uint32_t);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

op_code recibir_operacion(int socket_cliente){
	op_code cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
    uint32_t size;
    int desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cliente);

    char* mensaje = leer_de_buffer_string(buffer, &desplazamiento);

    log_info(logger, "Me llego el mensaje %s", mensaje);
    free(buffer);
    free(mensaje);
}

void* recibir_buffer(uint32_t* size, int socket_cliente)
{
    void * buffer;

    recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

/*----------Fin Mensajeria----------*/


/*----------Serializacion----------*/

void agregar_a_paquete_uint8(t_paquete* paquete, uint8_t numero)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint8_t));

	paquete->buffer->size += sizeof(uint8_t);
}


void agregar_a_paquete_uint32(t_paquete* paquete, uint32_t numero)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));

	paquete->buffer->size += sizeof(uint32_t);
}

void agregar_a_paquete_string(t_paquete* paquete, uint32_t tamanio, char* string)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(uint32_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(uint32_t));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), string, tamanio);

	paquete->buffer->size += tamanio + sizeof(uint32_t);
}


//-------------------------------------------------------------------------------------------



uint8_t leer_de_buffer_uint8(void* buffer, int* desplazamiento)
{
    uint8_t valor;

    memcpy(&valor,  buffer + (*desplazamiento), sizeof(uint8_t));

    (*desplazamiento) += sizeof(uint8_t);
    
    return valor;
};

uint32_t leer_de_buffer_uint32(void* buffer, int* desplazamiento)
{
    uint32_t valor;

    memcpy(&valor,  buffer + (*desplazamiento), sizeof(uint32_t));

    (*desplazamiento) += sizeof(uint32_t);
    
    return valor;
};

char* leer_de_buffer_string(void* buffer, int* desplazamiento)
{
    uint32_t tamanio = leer_de_buffer_uint32(buffer, desplazamiento);
    char* valor = malloc(tamanio);

    memcpy(valor, buffer + (*desplazamiento), tamanio);
    (*desplazamiento) += tamanio;

    return valor;
};

void serializar_CE(t_paquete* paquete, t_contexto_ejecucion contexto)
{
    agregar_a_paquete_uint32(paquete, contexto.PC);// uint32_t PC
    agregar_a_paquete_uint8(paquete, contexto.AX);// uint8_t AX
    agregar_a_paquete_uint8(paquete, contexto.BX);// uint8_t BX
    agregar_a_paquete_uint8(paquete, contexto.CX);// uint8_t CX
    agregar_a_paquete_uint8(paquete, contexto.DX);// uint8_t DX
    agregar_a_paquete_uint32(paquete, contexto.EAX);// uint32_t EAX
    agregar_a_paquete_uint32(paquete, contexto.EBX);// uint32_t EBX
    agregar_a_paquete_uint32(paquete, contexto.ECX);// uint32_t ECX
    agregar_a_paquete_uint32(paquete, contexto.EDX);// uint32_t EDX
    agregar_a_paquete_uint32(paquete, contexto.SI);// uint32_t SI
    agregar_a_paquete_uint32(paquete, contexto.DI);// uint32_t DI
};

void enviar_CE(int socket, uint32_t PID, t_contexto_ejecucion contexto)
{
    t_paquete* paquete = crear_paquete(CONTEXTO);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
};

void recibir_CE(int socket, uint32_t* PID_contenedor, t_contexto_ejecucion* contexto_contenedor)
{
    uint32_t size = 0;
    int desplazamiento = 0;
    void* buffer;

    buffer = recibir_buffer(&size, socket);

    (*PID_contenedor) = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->PC = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->AX = leer_de_buffer_uint8(buffer, &desplazamiento);
    contexto_contenedor->BX = leer_de_buffer_uint8(buffer, &desplazamiento);
    contexto_contenedor->CX = leer_de_buffer_uint8(buffer, &desplazamiento);
    contexto_contenedor->DX = leer_de_buffer_uint8(buffer, &desplazamiento);
    contexto_contenedor->EAX = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->EBX = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->ECX = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->EDX = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->SI = leer_de_buffer_uint32(buffer, &desplazamiento);
    contexto_contenedor->DI = leer_de_buffer_uint32(buffer, &desplazamiento);

    free(buffer);
};

void agregar_a_paquete_cod_ins(t_paquete* paquete, cod_ins codigo){    
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(cod_ins));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &codigo, sizeof(cod_ins));

	paquete->buffer->size += sizeof(cod_ins);    
}

cod_ins leer_de_buffer_cod_ins(void* buffer, int* desplazamiento)
{
    cod_ins codigo;

    memcpy(&codigo,  buffer + (*desplazamiento), sizeof(cod_ins));

    (*desplazamiento) += sizeof(cod_ins);
    
    return codigo;
};

/*----------Fin Serializacion----------*/
