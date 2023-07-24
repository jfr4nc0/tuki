#include "administrarInstrucciones.h"


bool existe_archivo(char *nombreArchivo) {
    bool existeArchivo = dictionary_has_key(dictionaryFcbs, nombreArchivo);

    if (existeArchivo) {
        log_debug(loggerFileSystem, "Archivo %s existe y se encuentra en el fcb", nombreArchivo);
    } else {
        log_warning(loggerFileSystem, FCB_NOT_FOUND, nombreArchivo);
    }

    return existeArchivo;
}

bool crear_archivo(char *nombreArchivo) {

    char rutaFcb[PATH_MAX];
    // Crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y sin bloques asociados.
    // Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
    t_fcb* nuevoFcb = crear_fcb_inicial(nombreArchivo);

    // Ruta completa del archivo es el nombre del archivo + el path hacia la ruta de los fcb
    if (snprintf(rutaFcb, sizeof(rutaFcb), "%s/%s", configFileSystem->PATH_FCB, nombreArchivo) < 0) {
        log_error(loggerFileSystem, "Error al construir la ruta del archivo del FCB %s.", nombreArchivo);
        return false;
    }

    FILE* archivo = fopen(rutaFcb,"w");
    if (archivo == NULL) {
        log_error(loggerFileSystem, "Error al crear el archivo del FCB %s.", nombreArchivo);
        return false;
    }

    dictionary_put(dictionaryFcbs, nombreArchivo, (void*)nuevoFcb);
    persistir_fcb(nuevoFcb);
    log_info(loggerFileSystem, "Archivo nuevo creado: %s", nombreArchivo);
    return nuevoFcb;
}

t_fcb *crear_fcb_inicial(char *nombreArchivo) {
    t_fcb* fcb = malloc(sizeof(*fcb));
    fcb->nombre_archivo = strdup(nombreArchivo);
    fcb->tamanio_archivo = 0;
    fcb->puntero_indirecto = 0;
    fcb->cantidad_bloques_asignados = 0;

    return fcb;
}

bool truncar_archivo(char* nombreArchivo, uint32_t tamanioNuevo) {
    log_info(loggerFileSystem, TRUNCATE_ARCHIVO, nombreArchivo, tamanioNuevo);

    t_fcb* fcb = dictionary_get(dictionaryFcbs, nombreArchivo);

    if (fcb == NULL) {
        log_error(loggerFileSystem, FCB_NOT_FOUND, nombreArchivo);
        return false;
    }

    actualizar_tamanio_bloques(fcb, tamanioNuevo);

    log_info(loggerFileSystem, "Completado Truncado de Archivo: <%s>", nombreArchivo);
    return true;
}

/*
Esta operación deberá leer la información correspondiente de los bloques a partir del puntero y el tamaño recibidos.
Esta información se deberá enviar a la Memoria para ser escrita a partir de la dirección física recibida por parámetro
y esperar su finalización para poder confirmar el éxito de la operación al Kernel.
*/
bool leer_archivo(char* nombreArchivo, uint32_t puntero, uint32_t direccionFisica, uint32_t bytesQueFaltanPorLeer, uint32_t pidProceso) {
    log_info(loggerFileSystem, LEER_ARCHIVO, nombreArchivo, puntero, direccionFisica, bytesQueFaltanPorLeer);

    t_fcb* fcb = dictionary_get(dictionaryFcbs, nombreArchivo);
    if (fcb == NULL) {
        log_error(loggerFileSystem, FCB_NOT_FOUND, nombreArchivo);
        return false;
    }
    FILE* archivoDeBloques = abrir_archivo_de_bloques(configFileSystem->PATH_BLOQUES);

    char* informacion = malloc(bytesQueFaltanPorLeer);
    char* buffer;

    uint32_t posicionAbsoluta = 0;
    uint32_t espacioDisponible = 0;
    uint32_t bytesLeidos = 0;
    uint32_t bytesParaLeer = 0;
    size_t rtaLectura;


    // Obtengo la posicion desde la cual voy a empezar a leer informacion.
    espacioDisponible = SIZE_BLOQUE - obtener_posicion_en_bloque(puntero);

    while(bytesQueFaltanPorLeer != 0) {
        log_info(loggerFileSystem,"Faltan leer <%u> bytes de los <%u> totales", bytesQueFaltanPorLeer, bytesQueFaltanPorLeer);
        buffer = malloc(SIZE_BLOQUE);

        // Se busca la posicion del siguiente bloque
        posicionAbsoluta = obtener_posicion_absoluta(fcb, puntero);
        log_info(loggerFileSystem, "Se posiciona en la posicion absoluta: <%u>.", posicionAbsoluta); // LOG A SACAR

        sleep(configFileSystem->RETARDO_ACCESO_BLOQUE);
        fseek(archivoDeBloques, posicionAbsoluta, SEEK_SET);

        // Si bytesParaLeer es 0 es primera iteracion
        if (bytesParaLeer == 0) {
            puntero = puntero + espacioDisponible;
            bytesParaLeer = (bytesQueFaltanPorLeer < espacioDisponible) ? bytesQueFaltanPorLeer : espacioDisponible;
        } else {
            puntero = puntero + SIZE_BLOQUE;
            bytesParaLeer = (bytesQueFaltanPorLeer < SIZE_BLOQUE) ? bytesQueFaltanPorLeer : SIZE_BLOQUE;
        }

        rtaLectura = fread(buffer, sizeof(char), bytesParaLeer, archivoDeBloques);

        if (rtaLectura!=bytesParaLeer || ferror(archivoDeBloques)) {
            log_error(loggerFileSystem, "El archivo de bloques no se leyo correctamente.");
        }
        fclose(archivoDeBloques);

        // Pasar la data leida del buffer al char *informacion.
        memcpy(informacion+bytesLeidos, buffer, bytesParaLeer);

        if (bytesQueFaltanPorLeer < SIZE_BLOQUE) {
            bytesLeidos += bytesQueFaltanPorLeer;
            bytesQueFaltanPorLeer = 0;
        } else {
            bytesLeidos += SIZE_BLOQUE;
            bytesQueFaltanPorLeer -= SIZE_BLOQUE;
        }

        log_info(loggerFileSystem,"Hasta ahora se leyeron <%u> bytes.", bytesLeidos);
        log_info(loggerFileSystem,"Faltan leer <%u> bytes.", bytesQueFaltanPorLeer);
    }

    // TODO: Enviar información a memoria para ser escrita a partir de la dirección física
    free(informacion);
    free(buffer);

    return true;
}


/*
Se deberá solicitar a la Memoria la información que se encuentra a partir de la dirección física y
escribirlo en los bloques correspondientes del archivo a partir del puntero recibido.
El tamaño de la información a leer de la memoria y a escribir en los bloques también deberá recibirse desde el Kernel.
*/


/////////// Auxiliares //////////

uint32_t obtener_bloque_relativo(t_fcb* fcbArchivo, uint32_t punteroFseek) {
    if ( ( punteroFseek % SIZE_BLOQUE == 0) && punteroFseek != 0) {
        return (punteroFseek / SIZE_BLOQUE)+1;
    }
    return redondear_hacia(punteroFseek, ARRIBA);
}

uint32_t obtener_bloque_absoluto(t_fcb* fcb, uint32_t punteroFseek) {
    uint32_t bloque = redondear_hacia(punteroFseek, ABAJO);
    return (bloque == 0) ? fcb->puntero_directo:
            archivo_de_bloques_leer_n_puntero_de_bloque_de_punteros(fcb->puntero_indirecto, bloque-1);
}

uint32_t obtener_posicion_en_bloque(uint32_t punteroFseek) {
    return punteroFseek - SIZE_BLOQUE * redondear_hacia(punteroFseek, ABAJO);
}

// Funcion que sirve para saber desde donde empezar a leer/escribir.
uint32_t obtener_posicion_absoluta(t_fcb* fcb, uint32_t punteroFseek) {
    return (obtener_bloque_absoluto(fcb, punteroFseek) * SIZE_BLOQUE) + obtener_posicion_en_bloque(punteroFseek);
}