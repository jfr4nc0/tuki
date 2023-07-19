#include "administrarInstrucciones.h"


bool existe_archivo(char *nombreArchivo) {
    bool existeArchivo = dictionary_has_key(listaFcbs, nombreArchivo);

    if (existeArchivo) {
        log_debug(loggerFileSystem, "Archivo %s existe y se encuentra en el fcb", nombreArchivo);
    } else {
        log_warning(loggerFileSystem, ERROR_ABRIR_ARCHIVO, nombreArchivo);
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

    dictionary_put(listaFcbs, nombreArchivo, (void*)nuevoFcb);
    log_info(loggerFileSystem, "Archivo nuevo creado: %s", nombreArchivo);
    return nuevoFcb;
}

t_fcb *crear_fcb_inicial(char *nombreArchivo) {
    t_fcb *fcb = malloc(sizeof(*fcb));
    fcb->nombre_archivo = strdup(nombreArchivo);
    fcb->tamanio_archivo = 0;
    fcb->puntero_indirecto = 0;
    fcb->cantidad_bloques_asignados = 0;

    return fcb;
}

void truncar_archivo(char *nombreArchivo, uint32_t tamanioNuevo) {
    log_info(loggerFileSystem, "Pedido para truncar Archivo: <%s> - Tamaño: <%u>", nombreArchivo, tamanioNuevo);

    t_fcb* fcbArchivo = dictionary_get(listaFcbs, nombreArchivo);

    if (fcbArchivo == NULL) {
        log_error(loggerFileSystem, "No se encontró el fcb en la lista de fcbs.");
        return;
    }

    actualizar_tamanio_bloques(fcbArchivo, tamanioNuevo);

    log_info(loggerFileSystem, "Truncar Archivo: <%s> - Tamaño: <%u> Exitoso", nombreArchivo, tamanioNuevo);
    return;
}
