#include "../funcionesCliente.h"

void paquete(int conexion, t_log* logger)
{
    t_paquete* paquete;

    if(!(paquete = crear_paquete())) {
        log_error(logger, E__PAQUETE_CREATE);
    }

    // Leemos y esta vez agregamos las lineas al paquete
    printf("Los siguientes valores que ingreses se enviaran al servidor, ingrese enter para terminar de ingresar valores", ENTER);

    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void eliminar_paquete(t_paquete* paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

int armar_conexion(t_config* config, char* modulo, t_log* logger)
{
    char* ip = extraer_de_modulo_config(config, IP_CONFIG, modulo, logger);
    char* puerto = extraer_de_modulo_config(config, PUERTO_CONFIG, modulo, logger);

    log_debug(logger, D__ESTABLECIENDO_CONEXION, ENTER);

    return crear_conexion(ip, puerto, logger);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

int crear_conexion(char *ip, char* puerto, t_log* logger)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Vamos a crear el socket.
    int socket_cliente = socket(server_info->ai_family,
            server_info->ai_socktype,
            server_info->ai_protocol);

    // Ahora que tenemos el socket, vamos a conectarlo
    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        log_error(logger, E__CONEXION_CONNECT, ENTER);
    }else {
        log_info(logger, I__CONEXION_CREATE, ENTER);
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

    log_debug(logger, "Se envió valor %s\n", mensaje);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2*sizeof(int);
    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}

