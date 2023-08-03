#include "administrarEstructuras.h"
bool actualizar_tamanio_bloques(t_fcb* fcb, uint32_t tamanioNuevo) {
    uint32_t bloquesAsignados = fcb->cantidad_bloques_asignados;
    uint32_t bloquesNuevos = dividir_por_size_bloque_y_redondear_hacia(tamanioNuevo, ARRIBA);

    // AMPLIAR TAMAÑO
    if (bloquesAsignados < bloquesNuevos) {
        ampliar_archivo(fcb, tamanioNuevo);
    }

    // REDUCIR TAMAÑO
    if (bloquesAsignados > bloquesNuevos) {
        reducir_archivo(fcb, tamanioNuevo);
    }

    fcb->tamanio_archivo = tamanioNuevo;
    log_debug(loggerFileSystem, "Quedando con bloques asignados: %u", fcb->cantidad_bloques_asignados);
    persistir_fcb(fcb);

    return true;
}

void ampliar_archivo(t_fcb *fcb, uint32_t tamanioNuevo)
{
    if(fcb->cantidad_bloques_asignados == 0) {
        asignar_bloques_archivo_vacio(fcb,tamanioNuevo);
    }
    else {
        asignar_bloques_archivo_con_informacion(fcb, tamanioNuevo);
    }
}

// esta implementacion es solo para archivos nuevos o archivos con tamaño 0 y sin punteros
void asignar_bloques_archivo_vacio(t_fcb *fcb,uint32_t tamanioNuevo) {
    // Si el tamanio del bloque alcanza, se le asigna solo el puntero directo
    if (tamanioNuevo <= SIZE_BLOQUE) {
        asignar_puntero_directo(fcb);
        log_debug(loggerFileSystem, "Bloques nuevos: %d", 1);
        fcb->cantidad_bloques_asignados += 1;
    }
    else {
        // cantidad de punteros que deberia haber en el bloque de punteros
        uint32_t temp = tamanioNuevo-SIZE_BLOQUE;
        uint32_t cantidadPunteros = dividir_por_size_bloque_y_redondear_hacia(temp, ARRIBA);
        log_debug(loggerFileSystem, "Bloques nuevos: %d", cantidadPunteros+1);
        fcb->cantidad_bloques_asignados += cantidadPunteros+1;
        asignar_puntero_directo(fcb);
        asignar_puntero_indirecto(fcb);

        log_info(loggerFileSystem, LOG_ACCESO_BLOQUE, fcb->nombre_archivo, fcb->puntero_indirecto);
        sleep(configFileSystem->RETARDO_ACCESO_BLOQUE);

        asignar_bloques(fcb, cantidadPunteros);
    }
}

// Archivos que ya tienen punteros asignados
void asignar_bloques_archivo_con_informacion(t_fcb *fcb, uint32_t tamanioNuevo)
{
    uint32_t temp = tamanioNuevo-SIZE_BLOQUE;
    uint32_t cantidadBloques = dividir_por_size_bloque_y_redondear_hacia(temp, ARRIBA);

    if (fcb->cantidad_bloques_asignados == 1) {
        asignar_puntero_indirecto(fcb);
    }

    log_info(loggerFileSystem, LOG_ACCESO_BLOQUE, fcb->nombre_archivo, fcb->puntero_indirecto);
    sleep(configFileSystem->RETARDO_ACCESO_BLOQUE);

    asignar_bloques(fcb, cantidadBloques);
    return;
}


int32_t bitmap_encontrar_bloque_libre() {
    // false = 0 --> libre
    // true = 1 --> ocupado
    uint32_t i;
    bool bloqueOcupado;
    for (i=0; i < (bitmap->size * 8); i++)
    {
        bloqueOcupado  = bitarray_test_bit(bitmap->bitarray, i);
        log_info(loggerFileSystem, "Acceso a Bitmap - Bloque: <%u> - Estado: <%u>", i, bloqueOcupado);
        // Si encuentra un bloque que esté en 0 devuelve la posición de ese bloque
        if(!bloqueOcupado)
        {
            return i;
            break;
        }
    }
     // Si no encuentra un bloque libre, retorna -1
    return -1;
}

void asignar_puntero_directo(t_fcb* fcb) {
    uint32_t bloque = bitmap_encontrar_bloque_libre();
    fcb->cantidad_bloques_asignados = bloque;
    bitmap_marcar_bloque_ocupado(bloque);
    fcb->cantidad_bloques_asignados += 1;
    fcb->tamanio_archivo += SIZE_BLOQUE;
    log_info(loggerFileSystem, "Se asigna el bloque <%u> como bloque directo.", bloque);
    return;
}

void asignar_puntero_indirecto(t_fcb* fcb) {
    uint32_t bloquePunteros = bitmap_encontrar_bloque_libre();
    fcb->puntero_indirecto = bloquePunteros;
    bitmap_marcar_bloque_ocupado(bloquePunteros);
    fcb->cantidad_bloques_asignados += 1;
    log_info(loggerFileSystem, "Se asigna el bloque <%u> como bloque de punteros.", bloquePunteros);
    return;
}

// 0 --> libre
void bitmap_marcar_bloque_libre(uint32_t numeroBloque) {
    bitarray_clean_bit(bitmap->bitarray, numeroBloque);
    // Sincronizar los cambios en el archivo y verificar que se haga de forma correcta
    if (msync(bitmap->direccion, bitmap->size, MS_SYNC) == -1) {
        log_error(loggerFileSystem,"Error al sincronizar los cambios en el Bitmap");
    }
    log_info(loggerFileSystem, MODIFICADO_BITMAP, numeroBloque, 0);
    return;
}

// 1 --> ocupado
void bitmap_marcar_bloque_ocupado(uint32_t numeroBloque) {
    bitarray_set_bit(bitmap->bitarray, numeroBloque);
    // Sincronizar los cambios en el archivo y verificar que se haga de forma correcta
    if (msync(bitmap->direccion, bitmap->size, MS_SYNC) == -1) {
        log_error(loggerFileSystem,"Error al sincronizar los cambios en el Bitmap");
    }
    log_info(loggerFileSystem, MODIFICADO_BITMAP, numeroBloque, 1);
    return;
}

void reducir_archivo(t_fcb *fcb, uint32_t tamanioNuevo) {
    if (tamanioNuevo == 0)
    {
        vaciar_archivo(fcb);
        return;
    }

    uint32_t cantidadBloquesDesasignar = fcb->cantidad_bloques_asignados - dividir_por_size_bloque_y_redondear_hacia(tamanioNuevo, ARRIBA);
    desasignar_bloques(fcb, cantidadBloquesDesasignar);
    return;
}

void vaciar_archivo(t_fcb *fcb) {
    log_info(loggerFileSystem, "Se vacia el archivo.");
    uint32_t cantidadBloquesDesasignar = fcb->cantidad_bloques_asignados;
    if (cantidadBloquesDesasignar != 1) {
        cantidadBloquesDesasignar--;
        desasignar_bloques(fcb, cantidadBloquesDesasignar);
    }
    // desasignar_puntero_directo
    bitmap_marcar_bloque_libre(fcb->puntero_directo);
    fcb->puntero_directo = 0;
    log_info(loggerFileSystem, LOG_PUNTERO_DIRECTO_DESASIGNADO);

    fcb->cantidad_bloques_asignados = 0;
    fcb->tamanio_archivo = 0;
}

bool persistir_fcb(t_fcb* fcb) {
    char rutaFcb[PATH_MAX];

    // Ruta completa del archivo es el nombre del archivo + el path hacia la ruta de los fcb
    if (snprintf(rutaFcb, sizeof(rutaFcb), "%s/%s", configFileSystem->PATH_FCB, fcb->nombre_archivo) < 0) {
        log_error(loggerFileSystem, "Error al construir la ruta del archivo del FCB %s.", fcb->nombre_archivo);
        return false;
    }

    FILE* archivoFcb = fopen(rutaFcb, "w");
    if (archivoFcb == NULL) {
        log_error(loggerFileSystem, ERROR_ABRIR_ARCHIVO, fcb->nombre_archivo);
        return false;
    }

    fprintf(archivoFcb,"NOMBRE_ARCHIVO=%s\n",fcb->nombre_archivo);
    fprintf(archivoFcb,"TAMANIO_ARCHIVO=%u\n",fcb->tamanio_archivo);
    fprintf(archivoFcb,"PUNTERO_DIRECTO=%u\n", fcb->puntero_directo);
    fprintf(archivoFcb,"PUNTERO_INDIRECTO=%u\n", fcb->puntero_indirecto);
    fprintf(archivoFcb,"CANTIDAD_BLOQUES_ASIGNADOS=%u\n", fcb->cantidad_bloques_asignados);

    fclose(archivoFcb);

    return true;
}

// Asignar punteros dentro del bloque de punteros
bool asignar_bloques(t_fcb *fcb, uint32_t cantidadBloques) {
    uint32_t cantidadPunteros = fcb->cantidad_bloques_asignados - 1; // No tomo en cuenta el primero

    uint32_t desplazamientoArchivoBloque = fcb->puntero_indirecto *SIZE_BLOQUE;
    uint32_t desplazamientoBloque = cantidadPunteros * sizeof(uint32_t);
    uint32_t desplazamiento = desplazamientoArchivoBloque + desplazamientoBloque;

    FILE* archivoDeBloques = fopen(configFileSystem->PATH_BLOQUES, "r+b");

    if (archivoDeBloques == NULL) {
        log_error(loggerFileSystem, ERROR_ABRIR_ARCHIVO, configFileSystem->PATH_BLOQUES);
        return false;
    }

    fseek(archivoDeBloques, desplazamiento, SEEK_SET);

    for (uint32_t i = 0; i < cantidadBloques; i++) {
        // El bloque de punteros siempre va a ser el bloque número 1 del archivo y el
        // bloque al que apunta el puntero indirecto va a ser el 0
        uint32_t bloqueDatos = bitmap_encontrar_bloque_libre();
        bitmap_marcar_bloque_ocupado(bloqueDatos);
        log_info(loggerFileSystem, "Bloque <%u> asignado al Archivo: <%s>", bloqueDatos, fcb->nombre_archivo);
        fwrite(&bloqueDatos, sizeof(uint32_t), 1, archivoDeBloques);
    }
    fclose(archivoDeBloques);
    return true;
}

void desasignar_bloques(t_fcb *fcb, uint32_t cantidadBloquesDesasignar) {
    for (uint32_t i = 0; i<cantidadBloquesDesasignar; i++) {
        desasignar_ultimo_bloque(fcb);
    }

    // Si el archivo quedo solamente con un bloque asignado significa que su bloque de punteros está vacio
    if (fcb->cantidad_bloques_asignados == 1) {
        // desasignar_puntero_indirecto
        bitmap_marcar_bloque_libre(fcb->puntero_indirecto);
        fcb->puntero_indirecto = 0;
        log_info(loggerFileSystem, LOG_PUNTERO_DIRECTO_DESASIGNADO);
    }

    return;
}

// DESASIGNAR
// 1. Buscar último bloque asignado en el bloque de punteros.
// 2. Marcarlo como vacio en el bitmap.
// 3. Disminuir el tamaño del archivo y la cantidad de bloques asignados.
bool desasignar_ultimo_bloque(t_fcb *fcb) {
    // ABRIR EL ARCHIVO DE BLOQUES
    FILE* archivoDeBloques = fopen(configFileSystem->PATH_BLOQUES, "r+b");

    if (archivoDeBloques == NULL) {
        log_error(loggerFileSystem, ERROR_ABRIR_ARCHIVO, configFileSystem->PATH_BLOQUES);
        return false;
    }

    uint32_t ultimoBloque = leer_ultimo_puntero_de_bloque_de_punteros(fcb);

    bitmap_marcar_bloque_libre(ultimoBloque);

    log_info(loggerFileSystem, LOG_BLOQUE_DESASIGNADO, ultimoBloque, fcb->nombre_archivo);

    //ACTUALIZAR FCB  --> El archivo tiene un bloque asignado menos.
    fcb->cantidad_bloques_asignados--;
    fcb->tamanio_archivo = fcb->tamanio_archivo - SIZE_BLOQUE;
    fclose(archivoDeBloques);

    return true;
}


int32_t leer_ultimo_puntero_de_bloque_de_punteros(t_fcb* fcb) {
    // Para saber la cantidad de punteros que hay en el bloque indirecto, tomo la cantidad total de bloques asignados y le
    // resto 1 por el puntero directo (el primer bloque)
    uint32_t cantidadBloquesAsignados = fcb->cantidad_bloques_asignados - 1;
    // Si hay 4 punteros en el bloque de punteros, quiero acceder el puntero número 3
    // Su indice va a ser el 2 --> Bloque X: [Ptr.0 , Ptr. 1, Ptr. 2, Ptr. 3]
    uint32_t punteroAAcceder = cantidadBloquesAsignados - 1;
    int32_t ultimoPuntero = archivo_de_bloques_leer_n_puntero_de_bloque_de_punteros(fcb->puntero_indirecto, punteroAAcceder);
    return ultimoPuntero;
}

// Los indices arrancan en 0, osea para leer el tercer puntero hay que pasar 2 --> [0, 1, 2]
int32_t archivo_de_bloques_leer_n_puntero_de_bloque_de_punteros(uint32_t bloque, uint32_t punteroN) {
    int32_t punteroLeido;
    // Desplazamiento para llegar al bloque de punteros
    uint32_t desplazamientoBloques = bloque * SIZE_BLOQUE;
    // Desplazamiento para llegar al puntero correspondiente en el bloque de punteros
    uint32_t desplazamientoPuntero = punteroN * sizeof(uint32_t);
    // Desplazamiento total
    uint32_t desplazamiento = desplazamientoBloques + desplazamientoPuntero;

    // ABRIR EL ARCHIVO DE BLOQUES
    FILE* archivoDeBloques = fopen(configFileSystem->PATH_BLOQUES, "r+b");

    if (archivoDeBloques == NULL) {
        log_error(loggerFileSystem, ERROR_ABRIR_ARCHIVO, configFileSystem->PATH_BLOQUES);
        return -1;
    }

    fseek(archivoDeBloques, desplazamiento, SEEK_SET);
    fread(&punteroLeido, sizeof(int32_t), 1, archivoDeBloques);
    fclose(archivoDeBloques);
    return punteroLeido;
}
