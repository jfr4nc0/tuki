#include "../funciones.h"

#include "funcionesCliente.c"
#include "funcionesServidor.c"

t_log* logger;

char* extraerDeConfig(t_config* config, char* valor, char* modulo) {
	char* valorModuloConfig = concatenarStrings(valor, modulo);

	if(config_has_property(config, valorModuloConfig)) {
		return config_get_string_value(config, valorModuloConfig);
	}

	printf("No se pudo encontrar en el archivo de configuracion -> %s El valor -> %s/n", config, valorModuloConfig);

	return EMPTY_STRING;
}


// TODO: volverla funcion que acepte infinitos parametros
char* concatenarStrings(char *p1, char *p2 ) {
  char *concatenacion = malloc( sizeof( char ) * ( strlen( p1 ) + strlen( p2 ) ) + 1 );

  // strcat( ) NECESITA un 0 al final de la cadena destino.
  *concatenacion = 0;

  // Ahora la llamamos 2 veces, 1 para cada cadena a añadir.
  strcat( concatenacion, p1 );
  strcat( concatenacion, p2 );

  return concatenacion;
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

// Por ahora no se usa la siguiente linea, Si se va a usar fijarse si es necesario agregar el include #include <readline/readline.h>
	/*
	while(1) {
		// Por ahora no se usa la siguiente linea, Si se va a usar agregar el include #include <readline/readline.h>
		//linea = readline(SIGN_CONSOLA);
		if (strcmp(linea, "") == 0) {
			break;
		}
		log_info(logger, linea, ENTER);
		free(linea);
	}
	*/
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

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
