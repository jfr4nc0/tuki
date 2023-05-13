#include "../funciones.h"

#include "funcionesCliente.c"
#include "funcionesServidor.c"

t_log* logger;

char* extraer_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            char* valor = config_get_string_value(config, property);
            log_trace(logger, "Se obtuvo el valor -> %s. En el config %s (%s)\n", valor, config->path, property);
            return valor;
        }
        log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s\n", config->path, property);

        return EMPTY_STRING;
}

char* extraer_de_modulo_config(t_config* config, char* valorIncompleto, char* modulo, t_log* logger) {
    char* property = concatenar_strings(valorIncompleto, modulo);
    return extraer_de_config(config, property, logger);
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

bool obtener_valores_para_logger(int moduloPos, bool *mostrarConsola, t_log_level *log_level, char* *modulo) {
    switch(moduloPos) {
            case ENUM_KERNEL:
                    (*modulo) = KERNEL;
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL);
                    (*log_level) = LOG_LEVEL_KERNEL;
                    break;
            case ENUM_CPU:
                    (*modulo) = CPU;
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CPU);
                    (*log_level) = LOG_LEVEL_CPU;
                    break;
            case ENUM_MEMORIA:
                    (*modulo) = MEMORIA;
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA);
                    (*log_level) = LOG_LEVEL_MEMORIA;
                    break;
            case ENUM_FILE_SYSTEM:
                    (*modulo) = FILE_SYSTEM;
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM);
                    (*log_level) = LOG_LEVEL_FILE_SYSTEM;
                    break;
            case ENUM_CONSOLA:
                    (*modulo) = CONSOLA;
                    (*mostrarConsola) = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA);
                    (*log_level) = LOG_LEVEL_CONSOLA;
              break;
            default:
                    (*modulo) = "LOG";
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
        char* modulo;
        bool valoresPorDefecto = obtener_valores_para_logger(moduloPos, &mostrarConsola, &log_level, &modulo);

    t_log *logger;
    if (( logger = log_create(pathLog, modulo, mostrarConsola, log_level)) == NULL ) {
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
