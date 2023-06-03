#include "../funcionesCliente.h"

void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

int armar_conexion(t_config* config, char* modulo, t_log* logger) {
    char* ip = extraer_de_modulo_config(config, IP_CONFIG, modulo, logger);
    char* puerto = extraer_de_modulo_config(config, PUERTO_CONFIG, modulo, logger);

    log_debug(logger, D__ESTABLECIENDO_CONEXION);

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
        uint32_t handshake = 1;
        uint32_t result;

        log_info(logger, I__CONEXION_CREATE);

        send(clienteAceptado, &handshake, sizeof(uint32_t), 0);
        recv(clienteAceptado, &result, sizeof(uint32_t), MSG_WAITALL);
    } else {
        log_error(logger, E__CONEXION_CONNECT);
    }

    freeaddrinfo(server_info);

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

    log_debug(logger, cantidad_strings_a_mostrar(2), "Se envió valor ", mensaje);

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

/*
 * Crea un paquete, le agrega el valor pasado como parametro, lo envia, y luego libera el paquete
 */
void enviarOperacion(int conexion, codigo_operacion codOperacion, int tamanio_valor, void* valor) {
    t_paquete* paquete = crear_paquete(codOperacion);
    if (tamanio_valor>0) {
        agregar_a_paquete(paquete, valor, tamanio_valor);
    }
    enviar_paquete(paquete, conexion);
    free(paquete);
}
/*
 * Devuelve el pcb a kernel porque terminó de ejecutar el proceso
 */
void devolver_pcb_kernel(PCB* pcb, int conexion, codigo_operacion codOperacion) {
    // TODO: Testear usando esta función
    // enviarOperacion(conexion, codOperacion, sizeof(PCB), pcb);

    t_paquete* paquete = crear_paquete(codOperacion);
    agregar_a_paquete(paquete, pcb, sizeof(PCB));
    enviar_paquete(paquete, conexion);
    free(paquete);
}

/*
 * Variable auxiliar, si solo me quiero identificar no hace falta que agregue ningun valor al paquete
 */
void identificarse(int conexion, codigo_operacion identificacion) {
	if (conexion > 0) {
		enviarOperacion(conexion, identificacion, 0, 0);
	}
}
