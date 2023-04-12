#include "../funcionesCliente.h"
#include "../constantes.h"
#include<readline/readline.h>

void paquete(int conexion, t_log* logger)
{
	t_paquete* paquete;
	char* lineaPaquete;

	if(!(paquete = crear_paquete())) {
		log_error(logger, E__PAQUETE_CREATE);
	}

	// Leemos y esta vez agregamos las lineas al paquete
	printf("Los siguientes valores que ingreses se enviaran al servidor, ingrese enter para terminar de ingresar valores", ENTER);

	while(1) {
		lineaPaquete = readline(SIGN_CONSOLA);
		if (strcmp(lineaPaquete, "") == 0) {
			break;
		}
		agregar_a_paquete(paquete, lineaPaquete, strlen(lineaPaquete)+1);
		free(lineaPaquete);
	}

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

int armar_conexion(t_config* config, t_log* logger)
{
	char* ip = config_get_string_value(config, "IP");
	char* puerto = config_get_string_value(config, "PUERTO");

	log_info(logger, I__ESTABLECIENDO_CONEXION, ENTER);
	log_info(logger, "ip %s, puerto %s\n", ip, puerto);

	return crear_conexion(ip, puerto, logger);
}


t_log* iniciar_logger(char* pathLog)
{
	t_log *logger;
	if (( logger = log_create(pathLog, "logs", true, LOG_LEVEL_INFO)) == NULL ) {
		printf(E__LOGGER_CREATE, ENTER);
		exit(1);
	}
	return logger;
}

t_config* iniciar_config(char* pathConfig)
{
	t_config* nuevo_config;
	if ((nuevo_config = config_create(pathConfig)) == NULL) {
		printf(E__LOGGER_CREATE, ENTER);
		exit(1);
	}

	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	printf("Los siguientes valores que ingresen se guardaran en el log, ingrese un enter para terminar de ingresar valores\n");
	char* linea;

	while(1) {
		linea = readline(SIGN_CONSOLA);
		if (strcmp(linea, "") == 0) {
			break;
		}
		log_info(logger, linea, ENTER);
		free(linea);
	}
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if (logger != NULL) {
		log_destroy(logger);
	}

	if (config != NULL) {
		config_destroy(config);
	}

	liberar_conexion(conexion);
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
		log_error(logger, E__CONEXION_CREATE, ENTER);
	}else {
		log_info(logger, I__CONEXION_CREATE, ENTER);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
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

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_super_paquete(void)
{
	//me falta un malloc!
	t_paquete* paquete;

	//TODO: descomentar despues de arreglar
	//paquete->codigo_operacion = PAQUETE;
	//crear_buffer(paquete);
	return paquete;
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

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


