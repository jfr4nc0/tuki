#include "../funcionesCliente.h"

void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

int armar_conexion(t_config* config, char* modulo, t_log* logger) {
    char* ip = extraer_de_modulo_config(config, IP_CONFIG, modulo, logger);
    char* puerto = extraer_de_modulo_config(config, PUERTO_CONFIG, modulo, logger);

    log_debug(logger, cantidad_strings_a_mostrar(2), D__ESTABLECIENDO_CONEXION, ENTER);

    return crear_conexion(ip, puerto, logger);
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigoOperacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

int crear_conexion(char *ip, char* puerto, t_log* logger) {
    struct addrinfo hints, *server_info;
    uint32_t handshake = 1;
    uint32_t result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Vamos a crear el socket.
    int clienteAceptado = socket(server_info->ai_family,
            server_info->ai_socktype,
            server_info->ai_protocol);

    // Ahora que tenemos el socket, vamos a conectarlo
    if (connect(clienteAceptado, server_info->ai_addr, server_info->ai_addrlen) != -1) {
        log_info(logger, cantidad_strings_a_mostrar(2), I__CONEXION_CREATE, ENTER);
    } else {
        log_error(logger, cantidad_strings_a_mostrar(2), E__CONEXION_CONNECT, ENTER);
    }

    freeaddrinfo(server_info);

    send(clienteAceptado, &handshake, sizeof(uint32_t), 0);
    recv(clienteAceptado, &result, sizeof(uint32_t), MSG_WAITALL);

    return clienteAceptado;
}

void enviar_mensaje(char* mensaje, int clienteAceptado, t_log* logger) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigoOperacion = OP_MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2*sizeof(int);

    void* a_enviar = serializar_paquete(paquete, bytes);

    send(clienteAceptado, a_enviar, bytes, 0);

    log_debug(logger, cantidad_strings_a_mostrar(3), "Se envió valor ", mensaje, ENTER);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete) {
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(codigo_operacion codigoOperacion) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigoOperacion = codigoOperacion;
    crear_buffer(paquete);
    return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int clienteAceptado) {
    int bytes = paquete->buffer->size + 2*sizeof(int);
    void* a_enviar = serializar_paquete(paquete, bytes);

    send(clienteAceptado, a_enviar, bytes, 0);

    free(a_enviar);
}
