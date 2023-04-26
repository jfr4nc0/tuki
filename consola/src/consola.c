#include "../include/consola.h"
#include "../../shared/src/funciones.c"

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

    int conexionKernel = armar_conexion(config, KERNEL, logger);

    enviarInstrucciones(pathInstrucciones, conexionKernel, logger);

    terminar_programa(conexionKernel, logger, config);
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

void enviarInstrucciones(char* pathInstrucciones, int conexionKernel, t_log* logger) {
    FILE *instrucciones = fopen(pathInstrucciones, MODO_LECTURA_ARCHIVO);
    char instruccion[LONGITUD_MAXIMA_CADENA];
    while (fgets(instruccion, LONGITUD_MAXIMA_CADENA, instrucciones)) {
        strtok(instruccion, "\n"); // Removemos el salto de linea
        enviar_mensaje(instruccion, conexionKernel, logger);
    }
}
