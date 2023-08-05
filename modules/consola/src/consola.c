#include "consola.h"

t_log* logger;
t_config* config;

int main(int argc, char** argv) {

    char* pathConfig = argv[1];
    char* pathInstrucciones = argv[2];

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CONSOLA);

    config = iniciar_config(pathConfig, logger);

    int conexionKernel = armar_conexion(config, KERNEL, logger);

    if (conexionKernel > 0) {
    	enviarInstrucciones(pathInstrucciones, conexionKernel, logger);
    }

    //terminar_programa(conexionKernel, logger, config);
}

/*
 * Se extraen las instrucciones del .txt y se envian a Kernel
 *
 */

void enviarInstrucciones(char* pathInstrucciones, int conexion_kernel, t_log* logger){

	t_paquete* paquete = crear_paquete(AUX_NEW_PROCESO);

	FILE *instrucciones;
	if( (instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO)) == NULL ){
		log_error(logger, E__ARCHIVO_CREATE, pathInstrucciones);

	} else {

		char* instruccion;
		size_t length;

		while (getline(&instruccion, &length, instrucciones) != -1) {
			strtok(instruccion, "\n");
			concatenar_strings(instruccion, " ");
			agregar_a_paquete(paquete, instruccion, strlen(instruccion) + 1); // +1 para incluir el '\0'
		}
	    enviar_paquete(paquete, conexion_kernel);
	}

	eliminar_paquete(paquete);
	fclose(instrucciones);
}
