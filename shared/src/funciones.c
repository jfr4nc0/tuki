#include "../funciones.h"

#include "funcionesCliente.c"
#include "funcionesServidor.c"

t_log* logger;

char* extrar_de_config(t_config* config, char* valor, char* modulo, t_log* logger) {
    char* valorModuloConfig = concatenar_strings(valor, modulo);

    if(config_has_property(config, valorModuloConfig)) {
        char* valor = config_get_string_value(config, valorModuloConfig);
        log_trace(logger, "Se obtuvo el valor -> %s. En el archivo de configuracion %s (%s)\n", valor, config->path, valorModuloConfig);
        return valor;
    }

    log_warning(logger, "No se pudo encontrar en el archivo de configuracion (%s), el valor -> %s\n", config->path, valorModuloConfig);

    return EMPTY_STRING;
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

bool obtener_valores_para_logger(int moduloPos, bool *mostrarConsola, t_log_level *log_level) {
    switch(moduloPos) {
            case ENUM_KERNEL:
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL);
                    (*log_level) = LOG_LEVEL_KERNEL;
                    break;
            case ENUM_CPU:
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CPU);
                    (*log_level) = LOG_LEVEL_CPU;
                    break;
            case ENUM_MEMORIA:
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA);
                    (*log_level) = LOG_LEVEL_MEMORIA;
                    break;
            case ENUM_FILE_SYSTEM:
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM);
                    (*log_level) = LOG_LEVEL_FILE_SYSTEM;
                    break;
            case ENUM_CONSOLA:
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA);
                    (*log_level) = LOG_LEVEL_CONSOLA;
              break;
            default:
              (*mostrarConsola) = true;
              (*log_level) = LOG_LEVEL_DEFAULT;
                    return true;
    }
    return false;
}

t_log* iniciar_logger(char* pathLog, int moduloPos)
{
        bool mostrarConsola = true;
        t_log_level log_level;
        bool valoresPorDefecto = obtener_valores_para_logger(moduloPos, &mostrarConsola, &log_level);

    t_log *logger;
    if (( logger = log_create(pathLog, "logs", mostrarConsola, log_level)) == NULL ) {
        printf(E__LOGGER_CREATE, ENTER);
        exit(1);
    }

    if (valoresPorDefecto) {
        log_warning(logger, D__LOG_CREADO, "-> ", pathLog, " con valores por defecto", ENTER);
    }else {
        log_debug(logger, D__LOG_CREADO, "-> ", pathLog, ENTER);
    }

    return logger;
}

t_config* iniciar_config(char* pathConfig, t_log* logger)
{
    t_config* nuevo_config;
    if ((nuevo_config = config_create(pathConfig)) == NULL) {
        log_error(logger, E__CONFIG_CREATE, ENTER);
        exit(1);
    }

    log_debug(logger, D__CONFIG_CREADO, "-> ", pathConfig, ENTER);
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
