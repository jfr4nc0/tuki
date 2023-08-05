#include "shared.h"

char* nombres_estados[] = {
    "NEW",
    "READY",
    "EXECUTING",
    "BLOCKED",
    "EXIT"
};

/*************** INSTRUCCIONES ***************/
typedef struct { char *key; int val;} t_symstruct;

static t_symstruct lookUpTable[] = {
		{ "SET", I_SET },
		{ "MOV_IN", I_MOV_IN },
		{ "MOV_OUT", I_MOV_OUT },
		{ "I/O", I_IO },
		{ "F_OPEN", I_F_OPEN },
		{ "F_CLOSE", I_F_CLOSE },
		{ "F_SEEK", I_F_SEEK },
		{ "F_READ", I_F_READ },
		{ "F_WRITE", I_F_WRITE },
		{ "F_TRUNCATE", I_TRUNCATE },
		{ "WAIT", I_WAIT },
		{ "SIGNAL", I_SIGNAL },
		{ "CREATE_SEGMENT", I_CREATE_SEGMENT },
		{ "DELETE_SEGMENT", I_DELETE_SEGMENT },
		{ "YIELD", I_YIELD },
		{ "EXIT", I_EXIT },
};

int keyFromString(char *key) {
    int i;
    //int cantidad_de_instrucciones = sizeof(lookUpTable) / sizeof(lookUpTable[0]);
    for (i=0; i < 16; i++) {
        t_symstruct sym = lookUpTable[i];
        if (strcmp(sym.key, key) == 0)
            return sym.val;
    }
    return -1;
}

int keyFromString_prueba(char *key) {
	int respuesta;

	if(string_starts_with(key, "SET") == 0){
		respuesta = I_SET;
	}else if(string_starts_with(key, "MOV_IN") == 0){
		respuesta = I_MOV_IN;
	}else if(string_starts_with(key, "MOV_OUT") == 0){
		respuesta = I_MOV_OUT;
	}else if(string_starts_with(key, "I/O") == 0){
		respuesta = I_IO;
	}if(string_starts_with(key, "F_OPEN") == 0){
		respuesta = I_F_OPEN;
	}else if(string_starts_with(key, "F_CLOSE") == 0){
		respuesta = I_F_CLOSE;
	}else if(string_starts_with(key, "F_SEEK") == 0){
		respuesta = I_F_SEEK;
	}else if(string_starts_with(key, "F_READ") == 0){
		respuesta = I_F_READ;
	}if(string_starts_with(key, "F_WRITE") == 0){
		respuesta = I_F_WRITE;
	}else if(string_starts_with(key, "F_TRUNCATE") == 0){
		respuesta = I_TRUNCATE;
	}else if(string_starts_with(key, "WAIT") == 0){
		respuesta = I_WAIT;
	}else if(string_starts_with(key, "SIGNAL") == 0){
		respuesta = I_SIGNAL;
	}if(string_starts_with(key, "CREATE_SEGMENT") == 0){
		respuesta = I_CREATE_SEGMENT;
	}else if(string_starts_with(key, "DELETE_SEGMENT") == 0){
		respuesta = I_DELETE_SEGMENT;
	}else if(string_starts_with(key, "YIELD") == 0){
		respuesta = I_YIELD;
	}else if(string_starts_with(key, "EXIT") == 0){
		respuesta = I_EXIT;
	}
	return respuesta;
}

/*-------------------- FUNCIONES GENERALES --------------------*/

char* truncar_string(char* str,int size){
    char* truncado = malloc(size+1);
    if(truncado==NULL){
        printf("Error al asignar memoria.\n");
        return NULL;
    }

    strncpy(truncado, str, size);
    truncado[size] = '\0';

    return truncado;
}

void* calcular_direccion(void* posicionBase, size_t desplazamiento) {
    return (void*)((uintptr_t)posicionBase + desplazamiento);
}

char** leer_arreglo_string(char* buffer, int* desplazamiento) {

	int longitud = leer_int(buffer, desplazamiento);

	char** arreglo = malloc((longitud + 1) * sizeof(char*));

	for(int i = 0; i < longitud; i++) {
	    arreglo[i] = leer_string(buffer, desplazamiento);
	}
	arreglo[longitud] = NULL;

	return arreglo;
}

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

char* extraer_string_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            char* valor = config_get_string_value(config, property);
            //log_trace(logger, "Se obtuvo el valor -> %s. En el config %s (%s)", valor, config->path, property);
            return valor;
    }
    //log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s", config->path, property);

    return NULL;
}

int extraer_int_de_config(t_config* config, char* property, t_log* logger) {
    if(config_has_property(config, property)) {
            int valor = config_get_int_value(config, property);
            //log_trace(logger, "Se obtuvo el valor -> %d. En el config %s (%s)", valor, config->path, property);
            return valor;
    }
    //log_warning(logger, "No se pudo encontrar en el config (%s), la propiedad -> %s", config->path, property);

    return -1;
}

char* extraer_de_modulo_config(t_config* config, char* valorIncompleto, char* modulo, t_log* logger) {
    char* property = concatenar_strings(valorIncompleto, modulo);
    return extraer_string_de_config(config, property, logger);
}

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
    char *directorioActual = get_current_dir_name();

    if (( logger = log_create(pathLog, modulo, mostrarConsola, log_level)) == NULL ) {
        printf(cantidad_strings_a_mostrar(3), E__LOGGER_CREATE, directorioActual, ENTER);
        free(directorioActual);
        exit(1);
    }
    free(directorioActual);
    /*
    if (valoresPorDefecto) {
    	log_warning(logger, cantidad_strings_a_mostrar(4), D__LOG_CREADO, "-> ", pathLog, " con valores por defecto");
    }else {
        log_debug(logger, cantidad_strings_a_mostrar(3), D__LOG_CREADO, "-> ", pathLog);
    }
	*/
    return logger;
}

t_config* iniciar_config(char* pathConfig, t_log* logger) {
    t_config* nuevo_config;
    char *directorioActual = get_current_dir_name();

    free(directorioActual);
    if ((nuevo_config = config_create(pathConfig)) == NULL) {
        log_error(logger, E__CONFIG_CREATE);
        exit(1);
    }

    //log_debug(logger, cantidad_strings_a_mostrar(3), D__CONFIG_INICIAL_CREADO, "-> ", pathConfig);
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

void liberar_conexion(int conexion) {
    if (conexion > 0) {
        close(conexion);
    }
}

/*
 * Produce perdida de memoria si no se libera la respuesta
 */
void* leer_de_buffer(char* buffer, int* desp, size_t tamanio) {
	void* respuesta = malloc(tamanio);
	memcpy(&respuesta, buffer + (*desp), tamanio);
	(*desp) += tamanio;

	return respuesta;
}

double leer_double(char* buffer, int* desp) {
	double respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(double));
	(*desp) += sizeof(double);

	return respuesta;
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

void* leer_algo(char* buffer, int* desp, size_t size) {
	void* respuesta;
	memcpy(respuesta, buffer + (*desp), size);
	(*desp)+=sizeof(size);

	return respuesta;
}

size_t leer_size(char* buffer, int* desp) {
	size_t respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(size_t));
	(*desp)+=sizeof(size_t);

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

uint32_t leer_uint32(char* buffer, int* desp) {
	uint32_t respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(uint32_t));
	(*desp)+=sizeof(uint32_t);

	return respuesta;
}

char* leer_texto(char* buffer, int* desp, int size) {
	char* respuesta = malloc(size);
	memcpy(respuesta, buffer+(*desp), size);
	(*desp)+=size;

	return respuesta;
}

char* leer_registro_4_bytes(char* buffer, int* desp){
    return leer_texto(buffer, desp, 4);
}

char* leer_registro_8_bytes(char* buffer, int* desp){
	return leer_texto(buffer, desp, 8);
}

char* leer_registro_16_bytes(char* buffer, int* desp){
	return leer_texto(buffer, desp, 16);
}

t_list* leer_string_array(char* buffer, int* desp) {
    int cantidadElementos = leer_int(buffer, desp);
    t_list* lista_instrucciones = list_create();

    for(int i = 0; i < cantidadElementos; i++)
    {
    	char* palabra = leer_string(buffer, desp);
    	int length = strlen(palabra);
    	if (length > 0 && palabra[length - 1] == '\n') {
			palabra[length - 1] = '\0'; // Se elimina el \n al final de la cadena y se reemplaza por \0 para el t_list
		}
    	list_add(lista_instrucciones, (void*) palabra);
    }

    return lista_instrucciones;
}


/*------------------- FUNCIONES CLIENTE ---------------------*/
void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

char** decode_instruccion(char* linea_a_parsear, t_log* logger) {

	char** instruccion = string_split(linea_a_parsear, " ");

	if(instruccion[0] == NULL) {
	    log_info(logger, "Se ignora linea vacía.");
	}

	return instruccion;
}

char** decode_instruccion_prueba(char* linea_a_parsear, t_log* logger) {

	//linea_a_parsear[strcspn(linea_a_parsear, "\n")]='\0';
	char** instruccion = string_split(linea_a_parsear, " ");

	int instruccion_;
	log_error(logger, "LA LINEA A PARSEAR ES: %s", linea_a_parsear);
	if(string_starts_with(linea_a_parsear, "SET") == 0){
		instruccion_ = I_SET;
	}else if(string_starts_with(linea_a_parsear, "YIELD") == 0){
		instruccion_ = I_YIELD;
	}else if(string_starts_with(linea_a_parsear, "EXIT") == 0){
		instruccion_ = I_EXIT;
	}

	switch(instruccion_){
		case I_SET:{
			strtok(instruccion[2], "\n");
			if (instruccion[0] == NULL) {
				log_info(logger, "Se ignora linea vacía.");
				return instruccion;
			}
			break;
		}
		case I_YIELD:{
			char* instruccion_yield = "YIELD";
			instruccion = &instruccion_yield;
			break;
		}
		case I_EXIT:{
			char* instruccion_exit = "EXIT";
			instruccion = &instruccion_exit;
			break;
		}
	}
	return instruccion;
}

char* encode_instruccion(char** strings) {
    const char* separator = " ";
    // Primero, calculamos el tamaño total de la cadena resultante
    size_t total_length = 0;
    for (int i = 0; strings[i] != NULL; i++) {
        total_length += strlen(strings[i]);
    }
    // Sumamos la longitud de los separadores entre cadenas
    total_length += (strlen(separator) * (int)(strlen(strings) - 1));

    // Reservamos memoria para la cadena resultante
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        return NULL;
    }

    // Copiamos cada cadena al resultado y añadimos el separador apropiado
    int offset = 0;
    for (int i = 0; strings[i] != NULL; i++) {
        if (i > 0) { // && strings[i+1] != NULL
            strcpy(result + offset, separator);
            offset += strlen(separator);
        }
        strcpy(result + offset, strings[i]);
        offset += strlen(strings[i]);
    }

    return result;
}


int armar_conexion(t_config* config, char* modulo, t_log* logger) {
	char* ip = extraer_de_modulo_config(config, IP_CONFIG, modulo, logger);
    char* puerto = extraer_de_modulo_config(config, PUERTO_CONFIG, modulo, logger);

    //log_debug(logger, D__ESTABLECIENDO_CONEXION, modulo);

    return crear_conexion(ip, puerto, modulo, logger);
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigoOperacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista, t_log* logger) {
	int tamanio = list_size(lista);
	agregar_int_a_paquete(paquete, tamanio);

	for(int i = 0; i < tamanio; i++) {
		void* elemento = list_get(lista, i);
		char* palabra = (char*)elemento;
		strtok(palabra, "\n"); // Removemos el salto de linea
		agregar_a_paquete(paquete, palabra, strlen(palabra));
	}

}

void agregar_registros_a_paquete(t_paquete* paquete, registros_cpu* registrosCpu) {
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->AX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->BX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->CX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->DX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EAX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EBX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->ECX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EDX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RAX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RBX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RCX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RDX);
}

void agregar_registro_a_paquete(t_paquete* paquete, char* registro, int tamanio_registro) {
	agregar_int_a_paquete(paquete, tamanio_registro);
	if (tamanio_registro == 16) {
		agregar_registro16bytes_a_paquete(paquete, registro);
	}
	if (tamanio_registro == 8) {
		agregar_registro8bytes_a_paquete(paquete, registro);
	}
	if (tamanio_registro == 4) {
		agregar_registro4bytes_a_paquete(paquete, registro);
	}
	return;
}


char* leer_registro_de_buffer(char* buffer, int desplazamiento) {
    int tamanioRegistro = leer_int(buffer, &desplazamiento);
    if (tamanioRegistro == 16) {
        return leer_registro_16_bytes(buffer, &desplazamiento);
    }
    if (tamanioRegistro == 8) {
        return leer_registro_8_bytes(buffer, &desplazamiento);
    }
    if (tamanioRegistro == 4) {
        return leer_registro_4_bytes(buffer, &desplazamiento);
    }
}

void agregar_registro4bytes_a_paquete(t_paquete* paquete, char valor[4]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}
void agregar_registro8bytes_a_paquete(t_paquete* paquete, char valor[8]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}
void agregar_registro16bytes_a_paquete(t_paquete* paquete, char valor[16]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long)*2);
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long)*2);
    paquete->buffer->size += sizeof(long)*2;
}

void agregar_size_a_paquete(t_paquete* paquete, size_t valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(size_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(size_t));
    paquete->buffer->size += sizeof(size_t);
}

void agregar_int_a_paquete(t_paquete* paquete, int valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}

void agregar_long_a_paquete(t_paquete* paquete, long valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}

void agregar_puntero_a_paquete(t_paquete* paquete, void* valor) {
    uintptr_t puntero = (uintptr_t)valor;
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uintptr_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &puntero, sizeof(uintptr_t));
    paquete->buffer->size += sizeof(uintptr_t);
}

uint32_t agregar_uint32_a_paquete(t_paquete* paquete, uint32_t valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(uint32_t));
    paquete->buffer->size += sizeof(uint32_t);
}

void agregar_lista_segmentos_a_paquete(t_paquete* paquete, int cliente, t_list* segmentosTabla, t_log* logger) {
	// Envio aparte las direcciones
//    enviar_operacion(cliente, AUX_OK, sizeof(segmentosTabla), (void*)segmentosTabla);
    /*
    t_paquete* paqueteDirecciones = crear_paquete(AUX_OK);

    for (int i = 0; i < list_size(segmentosTabla); i++) {
        t_segmento* segmento = list_get(segmentosTabla, i);
        agregar_a_paquete(paqueteDirecciones, &segmento, sizeof(t_segmento));
    }
    enviar_paquete(paqueteDirecciones, cliente);
    eliminar_paquete(paqueteDirecciones);
    */

    // envio el resto
	agregar_int_a_paquete(paquete, list_size(segmentosTabla));
    for (int i = 0; i < list_size(segmentosTabla); i++) {
        t_segmento* segmento = list_get(segmentosTabla, i);
        //log_trace(logger, D__LOG_SEGMENTO, segmento->id, segmento->direccionBase, segmento->size);
        agregar_int_a_paquete(paquete, segmento->id);
        agregar_size_a_paquete(paquete, segmento->size);
        agregar_puntero_a_paquete(paquete, segmento->direccionBase);
    }
    return;
}

void enviar_segmento_por_pid(int cliente, codigo_operacion codOp,t_segmento_tabla* tabla_segmento){
	t_paquete* paquete = crear_paquete(codOp);
	agregar_int_a_paquete(paquete, tabla_segmento->idProceso);
	agregar_int_a_paquete(paquete, tabla_segmento->segmento->id);
	agregar_size_a_paquete(paquete, tabla_segmento->segmento->size);
	enviar_paquete(paquete, cliente);
	eliminar_paquete(paquete);
	return;
}

void enviar_lista_segmentos_del_proceso(int cliente, t_list* segmentosLista, t_log* logger) {
	t_paquete* paquete = crear_paquete(AUX_OK);
    agregar_lista_segmentos_a_paquete(paquete, cliente, segmentosLista, logger);
    enviar_paquete(paquete, cliente);
    eliminar_paquete(paquete);
    return;
}

t_list* recibir_resto_lista_segmentos(void* buffer, int* desp) {
    t_list* listaSegmentos = list_create();
    int cantidadSegmentos = leer_int(buffer, desp);

    for (int i = 0; i < cantidadSegmentos; i++) {
    	t_segmento* segmento = malloc(sizeof(t_segmento));
    	segmento->id = leer_int(buffer, desp);
        segmento->size = leer_size(buffer, desp);
        segmento->direccionBase = leer_puntero(buffer, desp);

        list_add(listaSegmentos, segmento);
    }
    return listaSegmentos;
}

t_segmento_tabla* recibir_segmento_por_pid(int cliente){
	t_segmento_tabla* tabla_segmento = malloc(sizeof(tabla_segmento));
    t_segmento* segmento = malloc(sizeof(segmento));

    void* buffer;
	int tamanio = 0;
	int desp = 0;

    buffer = recibir_buffer(&tamanio, cliente);
    tabla_segmento->idProceso = leer_int(buffer,&desp);
    segmento->id = leer_int(buffer,&desp);
    segmento->size = leer_size(buffer,&desp);
    tabla_segmento->segmento = segmento;

    free(buffer);
    return tabla_segmento;
}

// va de la mano con agregar_lista_segmentos_a_paquete funcion
t_list* recibir_lista_segmentos(int clienteAceptado) {
    void* buffer;
	int tamanio = 0;
	int desp = 0;

    buffer = recibir_buffer(&tamanio, clienteAceptado);

    t_list* listaSegmentos = recibir_resto_lista_segmentos(buffer, &desp);

    free(buffer);
    return listaSegmentos;
}

void* leer_puntero(void* buffer, int* desp) {
	uintptr_t respuesta;
	memcpy(&respuesta, buffer + (*desp), sizeof(uintptr_t));
	(*desp)+=sizeof(uintptr_t);

	return (void*)respuesta;
}

void iteratorSinLog(char* value) {
    printf("%s \n", value);
}

void iteratorStrtok(char* instruccion) {
	strtok(instruccion, "\n");
}

void serializar_todas_las_tablas_segmentos(t_list* tablas_segmentos, t_paquete* paquete){
    agregar_int_a_paquete(paquete, tablas_segmentos->elements_count); // TODO: TENER CUIDADO
	for(int i = 0; i < tablas_segmentos->elements_count; i++){
		t_tabla_segmentos* tabla_segmentos = list_get(tablas_segmentos, i);
        agregar_int_a_paquete(paquete, tabla_segmentos->PID);
		serializar_tabla_segmentos(tabla_segmentos->segmentos, paquete);
	}
}

t_list* deserealizar_todas_las_tablas_segmentos(void* buffer, int* desplazamiento){
	t_list* tablas_segmentos = list_create();
	int cantidad_tablas_segmentos = leer_int(buffer, desplazamiento);
	for(int i = 0; i < cantidad_tablas_segmentos; i++){
		t_tabla_segmentos* elemento_tabla_segmentos = malloc(sizeof(t_tabla_segmentos));
        elemento_tabla_segmentos->PID = leer_int(buffer, desplazamiento);
		elemento_tabla_segmentos->segmentos = deserializar_tabla_segmentos(buffer, desplazamiento);
		list_add(tablas_segmentos, elemento_tabla_segmentos);
	}
	return tablas_segmentos;
}


t_list* deserializar_tabla_segmentos(void* buffer, int* desplazamiento){
	t_list* tabla_segmentos = list_create();

	int cantsegmento_ts = leer_int(buffer, desplazamiento);

	for (int i = 0; i < cantsegmento_ts; i++) {
        t_segmento* segmento = malloc(sizeof(t_segmento));

	    segmento->id = leer_int(buffer, desplazamiento);
	    segmento->size = leer_int(buffer, desplazamiento);
	    segmento->direccionBase = leer_puntero(buffer, desplazamiento);

	    list_add(tabla_segmentos, segmento);
    }
    return tabla_segmentos;
}

void serializar_tabla_segmentos(t_list *tabla_segmentos, t_paquete *paquete){
	agregar_int_a_paquete(paquete, tabla_segmentos->elements_count);
//    agregar_a_paquete_dato_serializado(paquete, &(tabla_segmentos->elements_count), sizeof(int));
    for (int i = 0; i < tabla_segmentos->elements_count; i++)
    {
        t_segmento *segmento = list_get(tabla_segmentos, i);
		// agregar_a_paquete
        agregar_int_a_paquete(paquete, segmento->id);
        agregar_int_a_paquete(paquete, segmento->size);
        agregar_puntero_a_paquete(paquete, segmento->direccionBase);
    }
}

void mostrarListaSegmentos(t_list* segmentos) {
	for (int indice = 0; indice < list_size(segmentos); indice++) {
		t_segmento* segmento = list_get(segmentos, indice);
		printf(D__LOG_SEGMENTO, segmento->id, segmento->direccionBase, segmento->size);
	}
}

void mostrar_pcb(PCB* pcb, t_log* logger){
    log_trace(logger, "PID: %d", pcb->id_proceso);
    char* estado = nombres_estados[pcb->estado];
    log_trace(logger, "ESTADO: %s", estado);
    log_trace(logger, "INSTRUCCIONES A EJECUTAR: ");
    list_iterate(pcb->lista_instrucciones, (void*) iteratorSinLog);
    log_trace(logger, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
    log_trace(logger, "Registro AX: %s", pcb->registrosCpu->AX);
    log_trace(logger, "Registro BX: %s", pcb->registrosCpu->BX);
    log_trace(logger, "Registro CX: %s", pcb->registrosCpu->CX);
    log_trace(logger, "Registro DX: %s", pcb->registrosCpu->DX);
    log_trace(logger, "Registro EAX: %s", pcb->registrosCpu->EAX);
    log_trace(logger, "Registro EBX: %s", pcb->registrosCpu->EBX);
    log_trace(logger, "Registro ECX: %s", pcb->registrosCpu->ECX);
    log_trace(logger, "Registro EDX: %s", pcb->registrosCpu->EDX);
    log_trace(logger, "Registro RAX: %s", pcb->registrosCpu->RAX);
    log_trace(logger, "Registro RBX: %s", pcb->registrosCpu->RBX);
    log_trace(logger, "Registro RCX: %s", pcb->registrosCpu->RCX);
    log_trace(logger, "Registro RDX: %s", pcb->registrosCpu->RDX);
    log_trace(logger, "LISTA SEGMENTOS: ");
    mostrarListaSegmentos(pcb->lista_segmentos);
    log_trace(logger, "LISTA ARCHIVOS ABIERTOS: ");
    // list_iterate(pcb->lista_archivos_abiertos, (void*) iteratorSinLog);
    log_trace(logger, "ESTIMACION HHRN: %f", pcb->estimacion_rafaga);
    log_trace(logger, "TIMESTAMP EN EL QUE EL PROCESO LLEGO A READY POR ULTIMA VEZ: %f", pcb->ready_timestamp);
}

void enviar_pcb(int conexion, PCB* pcb_a_enviar, codigo_operacion codigo, t_log* log) {
    t_paquete* paquete = crear_paquete(codigo);
    agregar_pcb_a_paquete(paquete, pcb_a_enviar, log);
    agregar_lista_segmentos_a_paquete(paquete, conexion, pcb_a_enviar->lista_segmentos, log);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}


void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb, t_log* log) {
    agregar_registros_a_paquete(paquete, pcb->registrosCpu);
    agregar_int_a_paquete(paquete, pcb->id_proceso);
    agregar_int_a_paquete(paquete, pcb->estado);
    agregar_lista_a_paquete(paquete, pcb->lista_instrucciones, log);
    agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
    // agregar_lista_archivos_a_paquete(paquete, pcb->lista_archivos_abiertos);
    agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
    agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
}

void agregar_lista_archivos_a_paquete(t_paquete* paquete, t_list* lista, t_log* logger) {
    int tamanio = list_size(lista);
    agregar_int_a_paquete(paquete, tamanio);

    for(int i = 0; i < tamanio; i++) {
        t_archivo_abierto* archivo = list_get(lista, i);

        char* nombreArchivo = (char*)archivo->nombreArchivo;
        strtok(nombreArchivo, "\n"); // Removemos el salto de linea
        log_debug(logger, "Agregando nombreArchivo: %s, tamanio %zu", nombreArchivo, strlen(nombreArchivo));
        agregar_a_paquete(paquete, nombreArchivo, strlen(nombreArchivo));

        log_debug(logger, "Agregando puntero: %d, tamanio %zu", archivo->puntero, sizeof(uint32_t));
        agregar_a_paquete(paquete, (void*)(intptr_t)(int)archivo->puntero, sizeof(int));
    }
}

void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}

char* leer_string(char* buffer, int* desp) {
	int size = leer_int(buffer, desp);

	char* respuesta = malloc(size);
	memcpy(respuesta, buffer+(*desp), size);
	(*desp)+=size;

	return respuesta;
}

PCB* recibir_pcb(int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));

	void* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, clienteAceptado);

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	strcpy(pcb->registrosCpu->AX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->BX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->CX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->DX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EAX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EBX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->ECX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EDX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RAX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RBX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RCX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RDX,  leer_registro_16_bytes(buffer, &desplazamiento));

	pcb->id_proceso = leer_int(buffer, &desplazamiento);
	pcb->estado = leer_int(buffer, &desplazamiento);
	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento);
	list_iterate(pcb->lista_instrucciones, (void*)iteratorStrtok);
	pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);
	pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);

	pcb->lista_archivos_abiertos = list_create();

	pcb->lista_segmentos = recibir_resto_lista_segmentos(buffer, &desplazamiento);
    free(buffer);

	return pcb;
	/*
	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));

		    archivo_abierto->nombreArchivo = leer_string(buffer, &desplazamiento);
		    archivo_abierto->puntero = leer_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		    free(archivo_abierto);
	}

*/

}

int crear_conexion(char *ip, char* puerto, char* modulo, t_log* logger) {
    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Vamos a crear el socket.
    int clienteAceptado = socket(server_info->ai_family,
            server_info->ai_socktype,
            server_info->ai_protocol);

    int val = 1;
    setsockopt(clienteAceptado, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // Ahora que tenemos el socket, vamos a conectarlo
    if (connect(clienteAceptado, server_info->ai_addr, server_info->ai_addrlen) != -1) {
        uint32_t handshake = 1;
        uint32_t result;

        log_info(logger, I__CONEXION_CREATE, modulo);

        send(clienteAceptado, &handshake, sizeof(uint32_t), 0);
        recv(clienteAceptado, &result, sizeof(uint32_t), MSG_WAITALL);

    } else {
        log_error(logger, E__CONEXION_CONNECT, modulo);
        clienteAceptado = -1;
    }

    freeaddrinfo(server_info);

    return clienteAceptado;
}

void enviar_mensaje(char* mensaje, int clienteAceptado, t_log* logger) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigoOperacion = AUX_MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2*sizeof(int);

    void* a_enviar = serializar_paquete(paquete, bytes);

    send(clienteAceptado, a_enviar, bytes, 0);

    log_debug(logger, cantidad_strings_a_mostrar(2), "Se envió valor ", mensaje);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete) {
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(codigo_operacion codigoOperacion) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigoOperacion = codigoOperacion;
    crear_buffer(paquete);
    return paquete;
}

void stream_recv_buffer(int fromSocket, t_buffer *destBuffer)
{
    // Recibo el size del buffer
    ssize_t msgBytes = recv(fromSocket, &(destBuffer->size), sizeof(destBuffer->size), 0);

    // Chequeo que el size del buffer se haya recibido correctamente
    if (msgBytes == -1) {
        printf("\e[0;31mstream_recv_buffer: Error en la recepción del buffer [%s]\e[0m\n", strerror(errno));
    }
    else if (destBuffer->size > 0) {
        // Recibo el stream del buffer
        destBuffer->stream = malloc(destBuffer->size);
        recv(fromSocket, destBuffer->stream, destBuffer->size, 0);
    }

    return;
}

// void enviar_tabla_segmentos(int conexion, codigo_operacion codOperacion, t_list* tabla_segmento) {
// 	if (conexion > 0) {
//         t_buffer* buffer = empaquetar_tabla_segmentos(tabla_segmento,(uint32_t)list_size(tabla_segmento));
//         stream_send_buffer(conexion,codOperacion,buffer);
// 		free(buffer);
// 	}
// }

// t_list* recibir_tabla_segmentos(int cliente, int tamanio){
//     t_buffer* buffer = buffer_create();
//     stream_recv_buffer(cliente,buffer);

//     uint32_t tamanio_tabla_segmento;
//     buffer_unpack(buffer,&tamanio_tabla_segmento,sizeof(tamanio_tabla_segmento));

//     t_list* tabla_segmento = desempaquetar_tabla_segmentos(buffer,tamanio_tabla_segmento);

//     free(buffer);
//     return tabla_segmento;
// }

t_list* desempaquetar_tabla_segmentos(t_buffer *bufferTablaSegmentos, uint32_t tamanioTablaSegmentos)
{
    t_list* tablaSegmentos = list_create();

    for (int i = 0; i < tamanioTablaSegmentos; i++) {
        t_segmento_tabla* tabla_segmento = malloc(sizeof(t_segmento_tabla));

        uint32_t idProceso;
        buffer_unpack(bufferTablaSegmentos, &idProceso, sizeof(idProceso));
        tabla_segmento->idProceso = (int)idProceso;

        uint32_t idSegmento;
        buffer_unpack(bufferTablaSegmentos, &idSegmento, sizeof(idSegmento));
        tabla_segmento->segmento->id = (int)idSegmento;

        uint32_t direccionBase;
        buffer_unpack(bufferTablaSegmentos, &direccionBase, sizeof(direccionBase));
        tabla_segmento->segmento->direccionBase = (void*)direccionBase;

        uint32_t tamanio;
        buffer_unpack(bufferTablaSegmentos, &tamanio, sizeof(tamanio));
        tabla_segmento->segmento->size = (size_t)tamanio;

        list_add(tablaSegmentos,tabla_segmento);
    }

    return tablaSegmentos;
}

t_buffer* empaquetar_tabla_segmentos(t_list* tablaSegmentos, uint32_t tamanioTablaSegmentos)
{
    t_buffer *bufferTablaSegmentos = buffer_create();

    buffer_pack(bufferTablaSegmentos, &tamanioTablaSegmentos, sizeof(tamanioTablaSegmentos));

    for (int i = 0; i < tamanioTablaSegmentos; i++) {
        t_segmento_tabla* tabla_segmento = (t_segmento_tabla*)list_get(tablaSegmentos,i);

        uint32_t idProceso = (uint32_t)tabla_segmento->idProceso;
        buffer_pack(bufferTablaSegmentos, &idProceso, sizeof(idProceso));

        uint32_t idSegmento = (uint32_t)tabla_segmento->segmento->id;
        buffer_pack(bufferTablaSegmentos, &idSegmento, sizeof(idSegmento));

        uint32_t direccionBase = (uint32_t)tabla_segmento->segmento->direccionBase;
        buffer_pack(bufferTablaSegmentos, &direccionBase, sizeof(direccionBase));

        uint32_t tamanio = (uint32_t)tabla_segmento->segmento->size;
        buffer_pack(bufferTablaSegmentos, &tamanio, sizeof(tamanio));
    }

    return bufferTablaSegmentos;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, size_t tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void buffer_pack_string(t_buffer *self, char *stringToAdd)
{
    uint32_t length = strlen(stringToAdd) + 1;

    // Empaqueto el tamanio del string
    buffer_pack(self, &length, sizeof(length));
    self->stream = realloc(self->stream, self->size + length);
    // Empaqueto el string
    memcpy(self->stream + self->size, stringToAdd, length);
    self->size += length;

    return;
}

char *buffer_unpack_string(t_buffer *self)
{
    char *str;
    uint32_t length;

    // Desempaqueto el tamanio del string y luego el string
    buffer_unpack(self, &length, sizeof(length));
    str = malloc(length);
    buffer_unpack(self, str, length);

    return str;
}

void buffer_pack(t_buffer* self, void* streamToAdd, int size) {
    // Reservo memoria para alojar el nuevo stream, copio  y actualizo la informacion necesaria
    self->stream = realloc (self->stream, self->size + size);
    memcpy(self->stream + self->size, streamToAdd, size);
    self->size += size;

    return;
}


t_buffer *buffer_create(void) {
    // Creo y seteo las variables del buffer
    t_buffer *self = malloc(sizeof(*self));
    self->size = 0;
    self->stream = NULL;

    return self;
}

t_buffer *buffer_unpack(t_buffer *self, void *dest, int size) {
    // Chequeo que me hayan pasado un buffer correctamente
    if (self->stream == NULL || self->size == 0) {
        puts("\e[0;31mbuffer_unpack: Error en el desempaquetado del buffer\e[0m");
        exit(-1);
    }

    // Desempaqueto la informacion y actualizo el tamano del buffer
    memcpy(dest, self->stream, size);
    self->size -= size;

    // Copio la proxima informacion a desempaquetar al puntero stream
    // Y reduzco la nueva memoria reservada al nuevo tamanio
    memmove(self->stream, self->stream + size, self->size);
    self->stream = realloc(self->stream, self->size);

    return self;
}

void *__stream_create(uint8_t header, t_buffer *buffer)
{
    void *streamToSend = malloc(sizeof(header) + sizeof(buffer->size) + buffer->size);

    // Creamos el stream
    int offset = 0;
    memcpy(streamToSend + offset, &header, sizeof(header));
    offset += sizeof(header);
    memcpy(streamToSend + offset, &(buffer->size), sizeof(buffer->size));
    offset += sizeof(buffer->size);
    memcpy(streamToSend + offset, buffer->stream, buffer->size);

    return streamToSend;
}

void stream_send_buffer(int toSocket, uint8_t header, t_buffer *buffer) {
    void *stream = __stream_create(header, buffer);
    __stream_send(toSocket, stream, buffer->size);
    free(stream);

    return;
}

void __stream_send(int toSocket, void *streamToSend, uint32_t bufferSize) {
    // Variables creadas para el sizeof
    uint8_t header = 0;
    uint32_t size = 0;

    ssize_t bytesSent = send(toSocket, streamToSend, sizeof(header) + sizeof(size) + bufferSize, 0);

    // Chequeo que se haya enviado bien el stream
    if (bytesSent == -1) {
        printf("\e[0;31m__stream_send: Error en el envío del buffer [%s]\e[0m\n", strerror(errno));
    }

    return;
}

void enviar_paquete(t_paquete* paquete, int clienteAceptado) {
    int bytes = paquete->buffer->size + 2*sizeof(int);
    void* a_enviar = serializar_paquete(paquete, bytes);

    send(clienteAceptado, a_enviar, bytes, 0);

    free(a_enviar);
}

/*
 * Crea un paquete, le agrega el valor pasado como parametro, lo envia, y luego libera el paquete
 */
void enviar_operacion(int conexion, codigo_operacion codOperacion, size_t tamanio_valor, void* valor) {
	if (conexion > 0) {
		t_paquete* paquete = crear_paquete(codOperacion);
		if (tamanio_valor>0) {
			agregar_a_paquete(paquete, valor, tamanio_valor);
		}
		enviar_paquete(paquete, conexion);
		free(paquete);
	}
}

/*
 * Variable auxiliar, si solo me quiero identificar no hace falta que agregue ningun valor al paquete
 */

void enviar_codigo_operacion(int conexion, codigo_operacion codigoOperacion) {
	if (conexion > 0) {
		enviar_operacion(conexion, codigoOperacion, 0, 0);
	}
}
/*----------------------- FUNCIONES SERVIDOR -------------------*/
int iniciar_servidor(t_config* config, t_log* logger) {
    int socket_servidor;
    char* puerto = extraer_string_de_config(config, PUERTO_LOCAL, logger);

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &servinfo);

    // Creamos el socket de escucha del servidor
    socket_servidor = socket(servinfo->ai_family,
           servinfo->ai_socktype,
           servinfo->ai_protocol);

    int my_true = 1; //Defino un true para poder pasarle el puntero al true
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &my_true, sizeof(int)); //Para cerrar el socket en cuanto se termine el proceso

    // Asociamos el socket a un puerto
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    // Escuchamos las conexiones entrantes
    listen(socket_servidor, SOMAXCONN);
    //log_warning(logger, I__SERVER_READY);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger) {
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;

    //log_info(logger, I_ESPERANDO_CONEXION);

    // Aceptamos un nuevo cliente
    int clienteAceptado = accept(socket_servidor, NULL, NULL);
    if (clienteAceptado == -1) {
        log_error(logger, E__CONEXION_ACEPTAR);
        return -1;
    }

    //log_info(logger, I__CONEXION_ACCEPT);

    log_debug(logger, "Se realiza un handshake de parte del servidor");

    recv(clienteAceptado, &handshake, sizeof(uint32_t), MSG_WAITALL);

    if(handshake == 1) {
        send(clienteAceptado, &resultOk, sizeof(uint32_t), 0);
        log_info(logger, HANDSHAKE, OK);
    } else {
        log_error(logger, HANDSHAKE, ERROR);
        send(clienteAceptado, &resultError, sizeof(uint32_t), 0);
    }

    return clienteAceptado;
}

void enviar_msj_con_parametros(int socket, int op_code, char** parametros) {
	int size_payload = 0;
	int size_total = sizeof(op_code) + sizeof(size_payload);

	for(int i = 0; i < string_array_size(parametros); i++) {
		size_payload += sizeof(int) + strlen(parametros[i]) + 1; //Tamanio de parametro + longitud de parametro
	}
	size_total += size_payload;

	void* stream = malloc(size_total);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &(op_code), sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(stream + desplazamiento, &size_payload, sizeof(size_payload));
	desplazamiento += sizeof(size_payload);

	int size_parametro_de_instruccion;
	for(int i = 0; i < string_array_size(parametros); i++) {
		size_parametro_de_instruccion = strlen(parametros[i]) + 1;
		memcpy(stream + desplazamiento, &(size_parametro_de_instruccion), sizeof(size_parametro_de_instruccion));
		desplazamiento += sizeof(size_parametro_de_instruccion);

		memcpy(stream + desplazamiento, parametros[i], size_parametro_de_instruccion);
		desplazamiento += size_parametro_de_instruccion;
	}

	send(socket, stream, size_total, 0);

	free(stream);
}

// TODO: en vez de int debería devolver el tipo de dato de codigo_operacion
int recibir_operacion(int clienteAceptado) {
    codigo_operacion cod_op;
    if(recv(clienteAceptado, &cod_op, sizeof(codigo_operacion), MSG_WAITALL) > 0) {
        return cod_op;
    }else {
    	close(clienteAceptado);
        return -1;
    }
}

void* recibir_buffer(int* size, int clienteAceptado) {
    void* buffer;

    recv(clienteAceptado, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(clienteAceptado, buffer, *size, MSG_WAITALL);

    return buffer;
}

void* recibir_puntero(int clienteAceptado) {
    int size;
    int desplazamiento = 0;
    int tamanio;
    void * buffer = recibir_buffer(&size, clienteAceptado);

    return leer_puntero(buffer, &desplazamiento);
}

t_list* recibir_paquete(int clienteAceptado) {
    int size;
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, clienteAceptado);
    while(desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento+=sizeof(int);
        char* valor = malloc(tamanio);
        memcpy(valor, buffer+desplazamiento, tamanio);
        desplazamiento+=tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}


/*----------------------- MANDAR A DORMIR -------------------*/

void intervalo_de_pausa(int duracionEnMilisegundos) {
    const uint32_t SECS_MILISECS = 1000;
    const uint32_t MILISECS_NANOSECS = 1000000;
    struct timespec timeSpec;

    // Time in seconds and nano seconds calculation
    timeSpec.tv_sec = duracionEnMilisegundos / SECS_MILISECS;
    timeSpec.tv_nsec = (duracionEnMilisegundos % SECS_MILISECS) * MILISECS_NANOSECS;

    nanosleep(&timeSpec, &timeSpec);

    return;
}


/*----------------- MANEJO DE SEGMENTOS -------------------*/
int get_dir_fisica(t_segmento* segmento ,char* dir_logica, int segment_max){
	/*	Esquema de memoria: Segmentacion
	 * 	Direccion Logica: [ Nro Segmento | direccionBase ]
	 *	@return: La direccion fisica
	 */

	segmento->id = floor(atoi(dir_logica)/segment_max);
	segmento->direccionBase = atoi(dir_logica)%segment_max;
	segmento->size = segmento->id + segmento->direccionBase;

	if(segmento->size > segment_max){
		return -1;
	} else { return segmento->size; }
}
