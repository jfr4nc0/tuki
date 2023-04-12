#include "../include/consola.h"
#include "../../shared/src/funcionesCliente.c"

int main(int argc, char** argv)
{
	// Comentar este if si se quiere tomar los valores por defecto
    validarArgumentos(argc, argv);

	// Se setean los parametros que se pasan, con poner valores por defecto
	char* pathConfig = argv[1] ? argv[1] : DEFAULT_CONFIG_PATH;
	char* pathInstrucciones = argv[2] ? argv[2] : DEFAULT_INSTRUCCIONES_PATH;
	char* pathLog = argv[3] ?  argv[3] : DEFAULT_LOG_PATH;

	int conexion;
	t_log* logger;
	t_config* config;

	logger = iniciar_logger(pathLog);
	config = iniciar_config(pathConfig);

	// Creamos una conexión hacia kernel
	conexion = armar_conexion(config, logger);

	ejecutarInstrucciones(pathInstrucciones, logger);


	// Armamos y enviamos el paquete
	paquete(conexion, logger);

	terminar_programa(conexion, logger, config);
}


// TODO: Mover todas las funciones a funciones.c

//TODO, ver si pasarle el parametro log, el problema es que es el tercer parametro
int validarArgumentos(int argc, char** argv) {
	if (argc<2) {
			printf(E__BAD_REQUEST, ENTER);

			printf("Dos parametros son obligatorios (pathConfig y pathInstrucciones), parametros enviados: %d\n", argc);

			for(int i=0; i<argc; i++) {
					printf("%s\n", argv[i]);
			}

			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void ejecutarInstrucciones(char* pathInstrucciones, t_log* logger) {
	FILE *instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO);
	char bufer[LONGITUD_MAXIMA_CADENA];
	while (fgets(bufer, LONGITUD_MAXIMA_CADENA, instrucciones)) {
		strtok(bufer, "\n"); // Removemos el salto de linea
		printf("La línea es: '%s'\n", bufer);
	}
}
