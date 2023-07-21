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
bool leer_archivo(char* nombreArchivo, uint32_t punteroProceso, uint32_t direccionFisica, uint32_t cantidadBytes, uint32_t pidProceso) {
    log_info(loggerFileSystem, LEER_ARCHIVO, nombreArchivo, punteroProceso, direccionFisica, cantidadBytes);
    char* informacion = malloc(cantidadBytes);
    int puntero = punteroProceso; // TODO: VER si hace falta
    uint32_t bytesQueFaltanPorLeer = cantidadBytes; // TODO: Ver si hace falta

    t_fcb* fcb = dictionary_get(dictionaryFcbs, nombreArchivo);
    if (fcb == NULL) {
        log_error(loggerFileSystem, FCB_NOT_FOUND, nombreArchivo);
        return false;
    }

    /* Obtengo cuales son las estructuras que voy a usar para leer, haciendo calculos con los parametros dados */
    uint32_t bloque = redondear_hacia(punteroProceso, ABAJO);
    bloque = (bloque == 0) ? fcb->puntero_directo:
        archivo_de_bloques_leer_n_puntero_de_bloque_de_punteros(fcb->puntero_indirecto, bloque-1);
    uint32_t espacioDisponible = espacio_disponible_en_bloque_desde_posicion(punteroProceso);
    uint32_t numeroBloqueArchivo = obtener_bloque_relativo(fcb, punteroProceso);

    /* Leo del archivo de bloques */
    log_info(loggerFileSystem, ACCESO_BLOQUE, nombreArchivo, bloque, numeroBloqueArchivo);
    char* buffer = malloc(SIZE_BLOQUE);
    FILE* archivoDeBloques = abrir_archivo_de_bloques(configFileSystem->PATH_BLOQUES);
    fseek(archivoDeBloques, bloque, SEEK_SET);
    size_t rtaLectura = fread(buffer, sizeof(char), cantidadBytes, archivoDeBloques);
    sleep(configFileSystem->RETARDO_ACCESO_BLOQUE);

    if (rtaLectura!=cantidadBytes || ferror(archivoDeBloques))
    {
        log_error(loggerFileSystem, "El archivo de bloques no se leyo correctamente.");
        return false;
    }
    fclose(archivoDeBloques);

    // Actualizo punteros
    puntero = puntero + espacioDisponible;
    uint32_t bytesLeidos = (cantidadBytes < espacioDisponible) ? cantidadBytes : espacioDisponible;
    bytesQueFaltanPorLeer = bytesQueFaltanPorLeer - bytesLeidos;

    memcpy(informacion, buffer, bytesLeidos);

    // TODO: CONTINUAR DESDE ACÁ

}


/*
Se deberá solicitar a la Memoria la información que se encuentra a partir de la dirección física y
escribirlo en los bloques correspondientes del archivo a partir del puntero recibido.
El tamaño de la información a leer de la memoria y a escribir en los bloques también deberá recibirse desde el Kernel.
*/


/////////// Auxiliares //////////
uint32_t obtener_posicion_en_bloque(uint32_t punteroFseek) {
    uint32_t posicion, bloqueRelativo;
    bloqueRelativo = redondear_hacia(punteroFseek, ABAJO);
    posicion = punteroFseek - SIZE_BLOQUE * bloqueRelativo;
    return posicion;
}

uint32_t espacio_disponible_en_bloque_desde_posicion(uint32_t punteroFseek) {
    uint32_t posicion, espacioDisponible;
    posicion = obtener_posicion_en_bloque(punteroFseek);
    espacioDisponible = SIZE_BLOQUE - posicion;
    return espacioDisponible;
}

uint32_t obtener_bloque_relativo(t_fcb* fcbArchivo, uint32_t punteroFseek) {
    if ( ( punteroFseek % SIZE_BLOQUE == 0) && punteroFseek != 0) {
        return (punteroFseek / SIZE_BLOQUE)+1;
    }
    return redondear_hacia(punteroFseek, ARRIBA);
}
