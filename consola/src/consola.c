#include "../include/consola.h"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesCliente.c"

int main(int argc, char** argv)
{
    // Comentar este if si se quiere tomar los valores por defecto
    validarArgumentos(argc, argv);

    // Se setean los parametros que se pasan, con valores por defecto si no encuentra parametros
    char* pathConfig = argv[1] ? argv[1] : PATH_DEFAULT_CONEXION_KERNEL;
    char* pathInstrucciones = argv[2] ? argv[2] : DEFAULT_INSTRUCCIONES_PATH;

    t_log* logger;
    t_config* config;

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CONSOLA);

    config = iniciar_config(pathConfig, logger);

    int conexion_kernel = armar_conexion(config, KERNEL, logger);

    enviarInstrucciones(pathInstrucciones, conexion_kernel, logger);

    terminar_programa(conexion_kernel, logger, config);
}

// TODO: Mover todas las funciones a funciones.c
void validarArgumentos(int argc, char** argv) {
    if (argc <= 2) {
            printf(E__BAD_REQUEST, ENTER);
            printf("Dos parametros son obligatorios (pathConfig y pathInstrucciones), parametros enviados: %d\n", argc);

            for(int i=1; i<argc; i++) {
                    printf("%s\n", argv[i]);
            }
            abort();
    }
}

void enviarInstrucciones(char* pathInstrucciones, int conexion_kernel, t_log* logger){
	t_paquete* paquete = crear_paquete();

	// Se extraen las instrucciones del .txt y se envian a Kernel
	// TODO: Funcion rota, arreglar
	FILE *instrucciones;
	if( (instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO)) == NULL ){
		log_error(logger, E__ARCHIVO_CREATE, ENTER);
	} else {

	    char instruccion[LONGITUD_MAXIMA_CADENA];

	    while (fgets(instruccion, LONGITUD_MAXIMA_CADENA, instrucciones) != NULL) {
	        strtok(instruccion, "$"); // Removemos el salto de linea
	        printf(instruccion);
	        agregar_a_paquete(paquete, instruccion, strlen(instruccion));
	    }

	    enviar_paquete(paquete, conexion_kernel);
	}

	eliminar_paquete(paquete);
}
