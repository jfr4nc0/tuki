#include "../funcionesServidor.h"

/*
void recibir_mensaje(int clienteAceptado, ) {
    int size;
    char* buffer = recibir_buffer(&size, clienteAceptado);
    log_info(logger, "Me llego el mensaje %s", buffer);
    free(buffer);
}
*/

int iniciar_servidor(t_config* config, t_log* logger) {
    int socket_servidor;
    char* puerto = extraer_de_config(config, PUERTO_LOCAL, logger);

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(LOCALHOST, puerto, &hints, &servinfo);

    // Creamos el socket de escucha del servidor
    socket_servidor = socket(servinfo->ai_family,
           servinfo->ai_socktype,
           servinfo->ai_protocol);

    // Asociamos el socket a un puerto
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    // Escuchamos las conexiones entrantes
    listen(socket_servidor, SOMAXCONN);
    log_info(logger, cantidad_strings_a_mostrar(2), I__SERVER_READY, ENTER);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger) {
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;
    // Aceptamos un nuevo cliente
    int clienteAceptado = accept(socket_servidor, NULL, NULL);
    if (clienteAceptado == -1) {
        log_error(logger, "Error al esperar cliente");
        return -1;
    }

    log_info(logger, "¡Se conecto un cliente!\n");

    log_debug(logger, "Se realiza un handshake de parte del servidor\n");
    recv(clienteAceptado, &handshake, sizeof(uint32_t), MSG_WAITALL);

    if(handshake == 1) {
        send(clienteAceptado, &resultOk, sizeof(uint32_t), 0);
        log_info(logger, "Handshake OK\n");
    } else {
        log_error(logger, "Handshake ERROR\n");
        send(clienteAceptado, &resultError, sizeof(uint32_t), 0);
    }

    return clienteAceptado;
}

int recibir_operacion(int clienteAceptado) {
    int cod_op;
    if(recv(clienteAceptado, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
        return cod_op;
    }else {
        close(clienteAceptado);
        return -1;
    }
}

void* recibir_buffer(int* size, int clienteAceptado) {
    void * buffer;

    recv(clienteAceptado, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(clienteAceptado, buffer, *size, MSG_WAITALL);

    return buffer;
}

t_list* recibir_paquete(int clienteAceptado) {
    int size;
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, clienteAceptado);
    while(desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento+=sizeof(int);
        char* valor = malloc(tamanio);
        memcpy(valor, buffer+desplazamiento, tamanio);
        desplazamiento+=tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}
