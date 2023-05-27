#include "../funciones.h"

/*
* Para que no salgan warning se especifica cuantos strings
* se van a mostrar
*/
char* cantidad_strings_a_mostrar(int cantidad) {
    int tamaño = cantidad * 3 + 1;
    char* mostrarStrings = malloc(tamaño);
    mostrarStrings[0] = '\0'; // Inicializar la cadena vacía

    for (int i = 0; i < cantidad; i++) {
        strcat(mostrarStrings, "%s ");
    }

    return mostrarStrings;
}

char* extraer_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            char* valor = config_get_string_value(config, property);
            log_trace(logger, "Se obtuvo el valor -> %s. En el config %s (%s)", valor, config->path, property);
            return valor;
    }
    log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s", config->path, property);

    return NULL;
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
            *modulo = KERNEL;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL);
            *log_level = LOG_LEVEL_KERNEL;
            break;
        case ENUM_CPU:
            *modulo = CPU;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CPU);
            *log_level = LOG_LEVEL_CPU;
            break;
        case ENUM_MEMORIA:
            *modulo = MEMORIA;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA);
            *log_level = LOG_LEVEL_MEMORIA;
            break;
        case ENUM_FILE_SYSTEM:
            *modulo = FILE_SYSTEM;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM);
            *log_level = LOG_LEVEL_FILE_SYSTEM;
            break;
        case ENUM_CONSOLA:
            *modulo = CONSOLA;
            *mostrarConsola = !!(MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA);
            *log_level = LOG_LEVEL_CONSOLA;
            break;
        default:
            *modulo = "LOG";
            *mostrarConsola = true;
            *log_level = LOG_LEVEL_DEFAULT;
            return true;
    }
    return false;
}

t_log* iniciar_logger(char* pathLog, int moduloPos) {
        bool mostrarConsola = true;
        t_log_level log_level;
        char* modulo;
        bool valoresPorDefecto = obtener_valores_para_logger(moduloPos, &mostrarConsola, &log_level, &modulo);

    t_log *logger;
    if (( logger = log_create(pathLog, modulo, mostrarConsola, log_level)) == NULL ) {
        printf(cantidad_strings_a_mostrar(2), E__LOGGER_CREATE, ENTER);
        exit(1);
    }

    if (valoresPorDefecto) {
    	log_warning(logger, cantidad_strings_a_mostrar(4), D__LOG_CREADO, "-> ", pathLog, " con valores por defecto");
    }else {
        log_debug(logger, cantidad_strings_a_mostrar(3), D__LOG_CREADO, "-> ", pathLog);
    }

    return logger;
}

t_config* iniciar_config(char* pathConfig, t_log* logger) {
    t_config* nuevo_config;
    if ((nuevo_config = config_create(pathConfig)) == NULL) {
        log_error(logger, E__CONFIG_CREATE);
        exit(1);
    }

    log_debug(logger, cantidad_strings_a_mostrar(3), D__CONFIG_CREADO, "-> ", pathConfig);
    return nuevo_config;
}


void terminar_programa(int conexion, t_log* logger, t_config* config) {
    if (logger != NULL) {
        log_destroy(logger);
    }

    if (config != NULL) {
        config_destroy(config);
    }

    liberar_conexion(conexion);
}

void liberar_conexion(int clienteAceptado) {
    close(clienteAceptado);
}

long leer_long(char* buffer, int* desp) {
	long respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(long));
	(*desp)+=sizeof(long);

	return respuesta;
}

long long leer_long_long(char* buffer, int* desp) {
	long long respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(long long));
	(*desp)+=sizeof(long long);

	return respuesta;
}

float leer_float(char* buffer, int* desp) {
	float respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(float));
	(*desp)+=sizeof(float);

	return respuesta;
}

int leer_int(char* buffer, int* desp) {
	int respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(int));
	(*desp)+=sizeof(int);

	return respuesta;
}

char* leer_string(char* buffer, int* desp){
	int size = leer_int(buffer, desp); // TODO: ¿No modifica acá también desplazamiento? Probar

	char* respuesta = malloc(size);
	memcpy(respuesta, buffer+(*desp), size);
	(*desp)+=size;

	return respuesta;
}

char** leer_string_array(char* buffer, int* desp) {
    int length = leer_int(buffer, desp);
    char** arr = malloc((length + 1) * sizeof(char*));

    for(int i = 0; i < length; i++)
    {
        arr[i] = leer_string(buffer, desp);
    }
    arr[length] = NULL;

    return arr;
}

const char* obtener_nombre_estado(pcb_estado estado){
	if (estado >= ENUM_NEW) {
		return nombres_estados[estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}
