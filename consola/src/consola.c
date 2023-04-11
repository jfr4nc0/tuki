#include "../include/consola.h"

#include "../../shared/src/funcionesCliente.c"
#include "utils.c"

int main(int argc, char** argv)
{

	// Comentar este if si se quiere tomar los valores por defecto
	if (argc<2) {
				printf(": Enviar dos parametros (pathConfig y pathInstrucciones) \n");
				printf("Parametros enviados: %d\n", argc);
				for(int i=0; i<argc; i++) {
					printf("%s\n", argv[i]);
				}
				return EXIT_FAILURE;
		}
//		return EXIT_SUCCESS;

	/*
	 * TODO: Hacer que esta funcion no tire error de multiple definition
	if (validarArgumentos(argc, argv) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	*/

	// Se setean los parametros que se pasan, con poner valores por defecto
	char* pathConfig = argv[1] ? argv[1] : DEFAULT_CONFIG_PATH;
	char* pathInstrucciones = argv[2] ? argv[2] : DEFAULT_INSTRUCCIONES_PATH;
	char* pathLog = argv[3] ?  argv[3] : DEFAULT_LOG_PATH;

	int conexion;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger(pathLog);
	config = iniciar_config(pathConfig);

	FILE *instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO);
	char bufer[LONGITUD_MAXIMA_CADENA];
	while (fgets(bufer, LONGITUD_MAXIMA_CADENA, instrucciones)) {
		 // Removemos el salto de linea
		 strtok(bufer, "\n");
		 printf("La línea es: '%s'\n", bufer);
	 }

	// Creamos una conexión hacia el servidor
	conexion = armar_conexion(config, logger);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);
}
