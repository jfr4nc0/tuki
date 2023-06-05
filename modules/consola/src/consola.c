#include "consola.h"

int main(int argc, char** argv) {

    t_log* logger;
    t_config* config;

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CONSOLA);

    config = iniciar_config(DEFAULT_PATH_CONFIG, logger);

    int conexionKernel = armar_conexion(config, KERNEL, logger);

    if (conexionKernel > 0) {
    	enviarInstrucciones(DEFAULT_INSTRUCCIONES_PATH, conexionKernel, logger);
    }

    terminar_programa(conexionKernel, logger, config);
}

// TODO: Mover todas las funciones a funciones.c
void validarArgumentos(int argc, char** argv) {
    if (argc <= 2) {
            printf(E__BAD_REQUEST);
            printf("\nDos parametros son obligatorios (pathConfig y pathInstrucciones), parametros enviados: %d\n", argc);

            for(int i=1; i<argc; i++) {
                    printf(cantidad_strings_a_mostrar(2), ENTER, argv[i]);
            }
            abort();
    }
}

/*
 * Se extraen las instrucciones del .txt y se envian a Kernel
 *
 */
void enviarInstrucciones(char* pathInstrucciones, int conexion_kernel, t_log* logger){
	t_paquete* paquete = crear_paquete(AUX_NEW_PROCESO);

	FILE *instrucciones;
	if( (instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO)) == NULL ){
		log_error(logger, E__ARCHIVO_CREATE);

	} else {

	    char instruccion[LONGITUD_MAXIMA_CADENA];

	    while (fgets(instruccion, LONGITUD_MAXIMA_CADENA, instrucciones) != NULL) {
	        strtok(instruccion, "$"); // Removemos el salto de linea
	        agregar_a_paquete(paquete, instruccion, strlen(instruccion));
	    }

	    enviar_paquete(paquete, conexion_kernel);
	    log_info(logger, "ENVIO INSTRUCCIONES.");
	}

	eliminar_paquete(paquete);
}