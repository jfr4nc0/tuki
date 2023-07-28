#include "shared.h"

char* nombres_estados[] = {
    "NEW",
    "READY",
    "EXECUTING",
    "BLOCKED",
    "EXIT"
};

/*************** INSTRUCCIONES ***************/
typedef struct { char *key; int val;} t_symstruct;

static t_symstruct lookUpTable[] = {
		{ "SET", I_SET },
		{ "MOV_IN", I_MOV_IN },
		{ "MOV_OUT", I_MOV_OUT },
		{ "I/O", I_IO },
		{ "F_OPEN", I_F_OPEN },
		{ "F_CLOSE", I_F_CLOSE },
		{ "F_SEEK", I_F_SEEK },
		{ "F_READ", I_F_READ },
		{ "F_WRITE", I_F_WRITE },
		{ "F_TRUNCATE", I_TRUNCATE },
		{ "WAIT", I_WAIT },
		{ "SIGNAL", I_SIGNAL },
		{ "CREATE_SEGMENT", I_CREATE_SEGMENT },
		{ "DELETE_SEGMENT", I_DELETE_SEGMENT },
		{ "YIELD", I_YIELD },
		{ "EXIT", I_EXIT },
};

int keyFromString(char *key) {
    int i;
    for (i=0; i < 16; i++) {
        t_symstruct sym = lookUpTable[i];
        if (strcmp(sym.key, key) == 0)
            return sym.val;
    }
    return -1;
}

/*-------------------- FUNCIONES GENERALES --------------------*/



char** leer_arreglo_string(char* buffer, int* desplazamiento) {

	int longitud = leer_int(buffer, desplazamiento);

	char** arreglo = malloc((longitud + 1) * sizeof(char*));

	for(int i = 0; i < longitud; i++) {
	    arreglo[i] = leer_string(buffer, desplazamiento);
	}
	arreglo[longitud] = NULL;

	return arreglo;
}

/*
* Para que no salgan warning se especifica cuantos strings
* se van a mostrar
*/
char* cantidad_strings_a_mostrar(int cantidad) {
    int tamaño = cantidad * 3 + 1;
    char* mostrarStrings = malloc(tamaño);
    mostrarStrings[0] = '\0'; // Inicializar la cadena vacía

    for (int i = 0; i < cantidad; i++) {
        strcat(mostrarStrings, "%s ");
    }

    return mostrarStrings;
}

char* extraer_string_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            char* valor = config_get_string_value(config, property);
            log_trace(logger, "Se obtuvo el valor -> %s. En el config %s (%s)", valor, config->path, property);
            return valor;
    }
    log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s", config->path, property);

    return NULL;
}

int extraer_int_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            int valor = config_get_int_value(config, property);
            log_trace(logger, "Se obtuvo el valor -> %d. En el config %s (%s)", valor, config->path, property);
            return valor;
    }
    log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s", config->path, property);

    return -1;
}

char* extraer_de_modulo_config(t_config* config, char* valorIncompleto, char* modulo, t_log* logger) {
    char* property = concatenar_strings(valorIncompleto, modulo);
    return extraer_string_de_config(config, property, logger);
}

// TODO: volverla funcion que acepte infinitos parametros
char* concatenar_strings(char *p1, char *p2 ) {
    char *concatenacion = malloc( sizeof( char ) * ( strlen( p1 ) + strlen( p2 ) ) + 1 );

    // strcat( ) NECESITA un 0 al final de la cadena destino.
    *concatenacion = 0;

    // Ahora la llamamos 2 veces, 1 para cada cadena a añadir.
    strcat( concatenacion, p1 );
    strcat( concatenacion, p2 );

    return concatenacion;
}

bool obtener_valores_para_logger(int moduloPos, bool *mostrarConsola, t_log_level *log_level, char* *modulo) {
    switch(moduloPos) {
        case ENUM_KERNEL:
            *modulo = KERNEL;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL);
            *log_level = LOG_LEVEL_KERNEL;
            break;
        case ENUM_CPU:
            *modulo = CPU;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CPU);
            *log_level = LOG_LEVEL_CPU;
            break;
        case ENUM_MEMORIA:
            *modulo = MEMORIA;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA);
            *log_level = LOG_LEVEL_MEMORIA;
            break;
        case ENUM_FILE_SYSTEM:
            *modulo = FILE_SYSTEM;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM);
            *log_level = LOG_LEVEL_FILE_SYSTEM;
            break;
        case ENUM_CONSOLA:
            *modulo = CONSOLA;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA);
            *log_level = LOG_LEVEL_CONSOLA;
            break;
        default:
            *modulo = "LOG";
            *mostrarConsola = true;
            *log_level = LOG_LEVEL_DEFAULT;
            return true;
    }
    return false;
}

t_log* iniciar_logger(char* pathLog, int moduloPos) {
        bool mostrarConsola = true;
        t_log_level log_level;
        char* modulo;
        bool valoresPorDefecto = obtener_valores_para_logger(moduloPos, &mostrarConsola, &log_level, &modulo);

    t_log *logger;
    char *directorioActual = get_current_dir_name();
    printf("Por crear logger desde %s \n", directorioActual);
    free(directorioActual);
    if (( logger = log_create(pathLog, modulo, mostrarConsola, log_level)) == NULL ) {
        printf(cantidad_strings_a_mostrar(2), E__LOGGER_CREATE, ENTER);
        exit(1);
    }

    if (valoresPorDefecto) {
    	log_warning(logger, cantidad_strings_a_mostrar(4), D__LOG_CREADO, "-> ", pathLog, " con valores por defecto");
    }else {
        log_debug(logger, cantidad_strings_a_mostrar(3), D__LOG_CREADO, "-> ", pathLog);
    }

    return logger;
}

t_config* iniciar_config(char* pathConfig, t_log* logger) {
    t_config* nuevo_config;
    char *directorioActual = get_current_dir_name();
    printf("Por crear config desde %s \n", directorioActual);
    free(directorioActual);
    if ((nuevo_config = config_create(pathConfig)) == NULL) {
        log_error(logger, E__CONFIG_CREATE);
        exit(1);
    }

    log_debug(logger, cantidad_strings_a_mostrar(3), D__CONFIG_INICIAL_CREADO, "-> ", pathConfig);
    return nuevo_config;
}


void terminar_programa(int conexion, t_log* logger, t_config* config) {
    if (logger != NULL) {
        log_destroy(logger);
    }

    if (config != NULL) {
        config_destroy(config);
    }

    liberar_conexion(conexion);
}

void liberar_conexion(int conexion) {
    if (conexion > 0) {
        close(conexion);
    }
}

/*
 * Produce perdida de memoria si no se libera la respuesta
 */
void* leer_de_buffer(char* buffer, int* desp, size_t tamanio) {
	void* respuesta = malloc(tamanio);
	memcpy(&respuesta, buffer + (*desp), tamanio);
	(*desp) += tamanio;

	return respuesta;
}

double leer_double(char* buffer, int* desp) {
	double respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(double));
	(*desp) += sizeof(double);

	return respuesta;
}

long leer_long(char* buffer, int* desp) {
	long respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(long));
	(*desp)+=sizeof(long);

	return respuesta;
}

long long leer_long_long(char* buffer, int* desp) {
	long long respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(long long));
	(*desp)+=sizeof(long long);

	return respuesta;
}

float leer_float(char* buffer, int* desp) {
	float respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(float));
	(*desp)+=sizeof(float);

	return respuesta;
}

int leer_int(char* buffer, int* desp) {
	int respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(int));
	(*desp)+=sizeof(int);

	return respuesta;
}

uint32_t leer_uint32(char* buffer, int* desp) {
	uint32_t respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(uint32_t));
	(*desp)+=sizeof(uint32_t);

	return respuesta;
}

char* leer_texto(char* buffer, int* desp, int size) {
	char* respuesta = malloc(size);
	memcpy(respuesta, buffer+(*desp), size);
	(*desp)+=size;

	return respuesta;
}

char* leer_registro_4_bytes(char* buffer, int* desp){
    return leer_texto(buffer, desp, 4);
}

char* leer_registro_8_bytes(char* buffer, int* desp){
	return leer_texto(buffer, desp, 8);
}

char* leer_registro_16_bytes(char* buffer, int* desp){
	return leer_texto(buffer, desp, 16);
}

char* leer_string(char* buffer, int* desp) {
	int size = leer_int(buffer, desp);

	char* respuesta = malloc(size);
	memcpy(respuesta, buffer+(*desp), size);
	(*desp)+=size;

	return respuesta;
}

t_list* leer_string_array(char* buffer, int* desp) {
    int cantidadElementos = leer_int(buffer, desp);
    t_list* lista_instrucciones = list_create();

    for(int i = 0; i < cantidadElementos; i++)
    {
    	char* palabra = leer_string(buffer, desp);
    	int length = strlen(palabra);
    	if (length > 0 && palabra[length - 1] == '\n') {
			palabra[length - 1] = '\0'; // Se elimina el \n al final de la cadena y se reemplaza por \0 para el t_list
		}
    	list_add(lista_instrucciones, (void*) palabra);
    }

    return lista_instrucciones;
}


/*------------------- FUNCIONES CLIENTE ---------------------*/
void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

char** decode_instruccion(char* linea_a_parsear, t_log* logger) {
	char** instruccion = string_split(linea_a_parsear, " ");
	if(instruccion[0] == NULL) {
	    log_info(logger, "Se ignora linea vacía.");
	}
	return instruccion;
}

int armar_conexion(t_config* config, char* modulo, t_log* logger) {
    char* ip = extraer_de_modulo_config(config, IP_CONFIG, modulo, logger);
    char* puerto = extraer_de_modulo_config(config, PUERTO_CONFIG, modulo, logger);

    log_debug(logger, D__ESTABLECIENDO_CONEXION, modulo);

    return crear_conexion(ip, puerto, modulo, logger);
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

int crear_conexion(char *ip, char* puerto, char* modulo, t_log* logger) {
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

        log_info(logger, I__CONEXION_CREATE, modulo);

        send(clienteAceptado, &handshake, sizeof(uint32_t), 0);
        recv(clienteAceptado, &result, sizeof(uint32_t), MSG_WAITALL);
    } else {
        log_error(logger, E__CONEXION_CONNECT, modulo);
        clienteAceptado = -1;
    }

    freeaddrinfo(server_info);

    return clienteAceptado;
}

void enviar_mensaje(char* mensaje, int clienteAceptado, t_log* logger) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigoOperacion = AUX_MENSAJE;
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

void agregar_a_paquete(t_paquete* paquete, void* valor, size_t tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void buffer_pack_string(t_buffer *self, char *stringToAdd)
{
    uint32_t length = strlen(stringToAdd) + 1;

    // Empaqueto el tamanio del string
    buffer_pack(self, &length, sizeof(length));
    self->stream = realloc(self->stream, self->size + length);
    // Empaqueto el string
    memcpy(self->stream + self->size, stringToAdd, length);
    self->size += length;

    return;
}

char *buffer_unpack_string(t_buffer *self)
{
    char *str;
    uint32_t length;

    // Desempaqueto el tamanio del string y luego el string
    buffer_unpack(self, &length, sizeof(length));
    str = malloc(length);
    buffer_unpack(self, str, length);

    return str;
}

void buffer_pack(t_buffer* self, void* streamToAdd, int size) {
    // Reservo memoria para alojar el nuevo stream, copio  y actualizo la informacion necesaria
    self->stream = realloc (self->stream, self->size + size);
    memcpy(self->stream + self->size, streamToAdd, size);
    self->size += size;

    return;
}


t_buffer *buffer_create(void) {
    // Creo y seteo las variables del buffer
    t_buffer *self = malloc(sizeof(*self));
    self->size = 0;
    self->stream = NULL;

    return self;
}

t_buffer *buffer_unpack(t_buffer *self, void *dest, int size) {
    // Chequeo que me hayan pasado un buffer correctamente
    if (self->stream == NULL || self->size == 0) {
        puts("\e[0;31mbuffer_unpack: Error en el desempaquetado del buffer\e[0m");
        exit(-1);
    }

    // Desempaqueto la informacion y actualizo el tamano del buffer
    memcpy(dest, self->stream, size);
    self->size -= size;

    // Copio la proxima informacion a desempaquetar al puntero stream
    // Y reduzco la nueva memoria reservada al nuevo tamanio
    memmove(self->stream, self->stream + size, self->size);
    self->stream = realloc(self->stream, self->size);

    return self;
}

static void *__stream_create(uint8_t header, t_buffer *buffer)
{
    void *streamToSend = malloc(sizeof(header) + sizeof(buffer->size) + buffer->size);

    // Creamos el stream
    int offset = 0;
    memcpy(streamToSend + offset, &header, sizeof(header));
    offset += sizeof(header);
    memcpy(streamToSend + offset, &(buffer->size), sizeof(buffer->size));
    offset += sizeof(buffer->size);
    memcpy(streamToSend + offset, buffer->stream, buffer->size);

    return streamToSend;
}

void stream_send_buffer(int toSocket, uint8_t header, t_buffer *buffer) {
    void *stream = __stream_create(header, buffer);
    __stream_send(toSocket, stream, buffer->size);
    free(stream);

    return;
}

static void __stream_send(int toSocket, void *streamToSend, uint32_t bufferSize) {
    // Variables creadas para el sizeof
    uint8_t header = 0;
    uint32_t size = 0;

    ssize_t bytesSent = send(toSocket, streamToSend, sizeof(header) + sizeof(size) + bufferSize, 0);

    // Chequeo que se haya enviado bien el stream
    if (bytesSent == -1) {
        printf("\e[0;31m__stream_send: Error en el envío del buffer [%s]\e[0m\n", strerror(errno));
    }

    return;
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
void enviar_operacion(int conexion, codigo_operacion codOperacion, size_t tamanio_valor, void* valor) {
	if (conexion > 0) {
		t_paquete* paquete = crear_paquete(codOperacion);
		if (tamanio_valor>0) {
			agregar_a_paquete(paquete, valor, tamanio_valor);
		}
		enviar_paquete(paquete, conexion);
		free(paquete);
	}
}

/*
 * Variable auxiliar, si solo me quiero identificar no hace falta que agregue ningun valor al paquete
 */
void enviar_codigo_operacion(int conexion, codigo_operacion codigoOperacion) {
	if (conexion > 0) {
		enviar_operacion(conexion, codigoOperacion, 0, 0);
	}
}

/*----------------------- FUNCIONES SERVIDOR -------------------*/
int iniciar_servidor(t_config* config, t_log* logger) {
    int socket_servidor;
    char* puerto = extraer_string_de_config(config, PUERTO_LOCAL, logger);

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
    log_info(logger, I__SERVER_READY);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger) {
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;

    log_info(logger, I_ESPERANDO_CONEXION);

    // Aceptamos un nuevo cliente
    int clienteAceptado = accept(socket_servidor, NULL, NULL);
    if (clienteAceptado == -1) {
        log_error(logger, E__CONEXION_ACEPTAR);
        return -1;
    }

    log_info(logger, I__CONEXION_ACCEPT);

    log_debug(logger, "Se realiza un handshake de parte del servidor");
    recv(clienteAceptado, &handshake, sizeof(uint32_t), MSG_WAITALL);

    if(handshake == 1) {
        send(clienteAceptado, &resultOk, sizeof(uint32_t), 0);
        log_info(logger, HANDSHAKE, OK);
    } else {
        log_error(logger, HANDSHAKE, ERROR);
        send(clienteAceptado, &resultError, sizeof(uint32_t), 0);
    }

    return clienteAceptado;
}

// TODO: en vez de int debería devolver el tipo de dato de codigo_operacion
int recibir_operacion(int clienteAceptado) {
    codigo_operacion cod_op;
    if(recv(clienteAceptado, &cod_op, sizeof(codigo_operacion), MSG_WAITALL) > 0) {
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


/*----------------------- MANDAR A DORMIR -------------------*/

void intervalo_de_pausa(int duracionEnMilisegundos) {
    const uint32_t SECS_MILISECS = 1000;
    const uint32_t MILISECS_NANOSECS = 1000000;
    struct timespec timeSpec;

    // Time in seconds and nano seconds calculation
    timeSpec.tv_sec = duracionEnMilisegundos / SECS_MILISECS;
    timeSpec.tv_nsec = (duracionEnMilisegundos % SECS_MILISECS) * MILISECS_NANOSECS;

    nanosleep(&timeSpec, &timeSpec);

    return;
}
