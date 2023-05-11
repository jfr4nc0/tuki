#include "../include/consola.h"
#include "../../shared/src/funciones.c"

int main(int argc, char** argv)
{
    // Comentar este if si se quiere tomar los valores por defecto
//    validarArgumentos(argc, argv);

    // Se setean los parametros que se pasan, con valores por defecto si no encuentra parametros
//    char* pathConfig = argv[1] ? argv[1] : PATH_DEFAULT_CONEXION_KERNEL;
//    char* pathInstrucciones = argv[2] ? argv[2] : DEFAULT_INSTRUCCIONES_PATH;

	char* pathConfig = PATH_DEFAULT_CONEXION_KERNEL;
	char* pathInstrucciones = DEFAULT_INSTRUCCIONES_PATH;


    t_log* logger;
    t_config* config;

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CONSOLA);

    config = iniciar_config(pathConfig, logger);

<<<<<<< HEAD
    log_info(logger, pathConfig);
    log_info(logger, pathInstrucciones);


    // Creamos una conexión hacia kernel
    conexionKernel = armar_conexion(config, KERNEL, logger);
=======
    int conexionKernel = armar_conexion(config, KERNEL, logger);
>>>>>>> refs/heads/PlanificacionFIFO

    enviarInstrucciones(pathInstrucciones, conexionKernel, logger);

    terminar_programa(conexionKernel, logger, config);
}

// TODO: Mover todas las funciones a funciones.c

//TODO, ver si pasarle el parametro log, el problema es que es el tercer parametro
int validarArgumentos(int argc, char** argv) {
    if (argc<3) {
            printf(E__BAD_REQUEST, ENTER);

            printf("Dos parametros son obligatorios (pathConfig y pathInstrucciones), parametros enviados: %d\n", argc);

            for(int i=1; i<argc; i++) {
                    printf("%s\n", argv[i]);
            }

            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void enviarInstrucciones(char* pathInstrucciones, int conexionKernel, t_log* logger){
	t_paquete* instrucciones_p = crear_paquete();

	// Parseo las instrucciones del .txt y las agrego al paquete
	// TODO: Funcion rota, arreglar
	FILE *instrucciones;
	if( (instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO)) == NULL ){
		log_error(logger, E__ARCHIVO_CREATE, ENTER);
	} else {

	    char instruccion[LONGITUD_MAXIMA_CADENA];

	    while (fgets(instruccion, LONGITUD_MAXIMA_CADENA, instrucciones) != NULL) {
	        strtok(instruccion, "$"); // Removemos el salto de linea
	        printf(instruccion);
	        agregar_a_paquete(instrucciones_p, instruccion, strlen(instruccion));
	    }

	    enviar_paquete(instrucciones_p, conexionKernel);
	}
}
