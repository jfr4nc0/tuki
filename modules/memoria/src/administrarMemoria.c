#include "administrarMemoria.h"

int idSegmentoGlobal = 0;
t_memoria* memoria;

void inicializar_memoria(size_t sizeMemoriaTotal, size_t sizeSegmento0) {

    memoria = malloc(sizeof(t_memoria));
    memoria->espacioUsuario = malloc(sizeMemoriaTotal);
    memoria->segmentos = malloc(sizeof(t_list));
    memoria->segmentos = list_create();
    memoria->tablaDeSegmentos = malloc(sizeof(t_list));
    memoria->tablaDeSegmentos = list_create();
    memoria->huecosLibres = malloc(sizeof(t_list));
    memoria->huecosLibres = list_create();
    memoria->sizeEspacioUsuario = sizeMemoriaTotal;
    t_segmento* segmento = malloc(sizeof(t_segmento));

    if (añadir_segmento(0, sizeSegmento0, memoria->espacioUsuario)) {
        log_info(loggerMemoria, I__SEGMENTO_0_CREADO, sizeSegmento0);
    } else {
        log_error(loggerMemoria, E__MALLOC_ERROR, sizeSegmento0);
    }

    free(segmento);
    return;
}

void iteratorTabla(t_segmento_tabla* elemento) {
    log_debug(loggerMemoria, "Id proceso %d, segmento id %d, posicion %p, size %zu",
    		elemento->idProceso, elemento->segmento->id, elemento->segmento->direccionBase, elemento->segmento->size);
}

void iteratorSegmento(t_segmento* elemento) {
	log_debug(loggerMemoria, "Segmento id %d, posicion %p, size %zu", elemento->id, elemento->direccionBase, elemento->size);
}

/*
** Maneja el flujo de respuesta de la funcion crear_segmento
*/
codigo_operacion adapter_respuesta_segmento(int pid, t_segmento* segmento, void* respuesta){
    if (respuesta == (void*)-1) {
		log_error(loggerMemoria, "Segmentation Fault (no hay memoria), para proceso %d, por peticion de tamaño: %zu", pid, segmento->size);
        return AUX_ERROR;
	}else if(respuesta == NULL) {
		log_warning(loggerMemoria, "Peticion de proceso %d, para tamaño %zu, solo se puede haciendo compactacion y usando los huecos libres", pid, segmento->size);
        return AUX_SOLO_CON_COMPACTACION;
	}else{
        segmento->direccionBase = respuesta;
		log_info(loggerMemoria, "Memoria usada: %p", respuesta);
        return AUX_OK;
    }
}

codigo_operacion inicializar_proceso(int pid, size_t pcbSize) {
    t_segmento* segmento;
    segmento->size = pcbSize;
    void* respuesta = crear_segmento(pid, segmento->size);

    return adapter_respuesta_segmento(pid,pcbSize,respuesta);
}

void finalizar_proceso(int idProceso) {
    int cantidadSegmentos = list_size(memoria->tablaDeSegmentos);
    bool seEliminoSegmento = false;

    for (int i = 0; i < cantidadSegmentos; i++) {
        t_segmento_tabla* segmentoTabla = list_get(memoria->tablaDeSegmentos, i);

        if (segmentoTabla->idProceso == idProceso) {
            t_segmento* segmento = segmentoTabla->segmento;
            list_remove_and_destroy_element(memoria->tablaDeSegmentos, i, free);
            list_remove_and_destroy_element(memoria->segmentos, i, free);
            seEliminoSegmento = true;
            i--;
            cantidadSegmentos--;
            log_info(loggerMemoria, "Se eliminó el segmento id %d, perteneciente al id de proceso %d.", segmento->id, idProceso);
        }
    }

    if (!seEliminoSegmento) {
    	log_warning(loggerMemoria, "No hay segmentos para eliminar para el proceso %d.", idProceso);
    }
    return;
}

void liberar_memoria() {
    free(memoria->espacioUsuario);

    // Liberar los elementos de las listas antes de destruirlas
    list_destroy_and_destroy_elements(memoria->segmentos, free);
    list_destroy_and_destroy_elements(memoria->huecosLibres, free);
    list_destroy_and_destroy_elements(memoria->tablaDeSegmentos, free);

    free(memoria);

    return;
}

//////////////////////// Instrucciones ////////////////

void compactar_memoria() {
    // Ordenar la lista de segmentos por dirección base
    list_sort(memoria->tablaDeSegmentos, (void*) comparar_segmentos_por_direccion_base);

    int cantidadSegmentos = list_size(memoria->tablaDeSegmentos);
    void* direccionBaseActual = memoria->espacioUsuario;

    for (int i = 0; i < cantidadSegmentos; i++) {
        t_segmento_tabla* segmentoTabla = list_get(memoria->tablaDeSegmentos, i);
        t_segmento* segmento = segmentoTabla->segmento;

        // Actualizar la dirección base del segmento
        segmento->direccionBase = direccionBaseActual;

        // Calcular la dirección base para el siguiente segmento
        direccionBaseActual = calcular_direccion(direccionBaseActual, segmento->size);
    }
    list_clean(memoria->huecosLibres);
}

codigo_operacion crear_segmento_por_pid(int pid, t_segmento* segmento){
    void* respuesta = crear_segmento(pid, segmento->size);

    return adapter_respuesta_segmento(pid,segmento,respuesta);
}

void* crear_segmento(int idProceso, size_t size) {
    // Verificar disponibilidad de espacio contiguo
    void* direccion_base = buscar_espacio_contiguo(size);
    if (direccion_base != NULL) {
        añadir_segmento(idProceso, size, direccion_base);
        recalcular_huecos_libres();

        return direccion_base;
    } else if (suma_size_huecos_disponibles(memoria->huecosLibres) >= size) {
        return (void*)NULL; // Utilizar puntero nulo para indicar que se utilizarán los huecos libres
    }

    return (void*)-1; // Retornar un puntero a -1 para indicar segmentation fault
}


bool añadir_segmento(int idProceso, size_t size, void* direccion_base) {
    // Espacio contiguo disponible, crear segmento
    t_segmento* segmento = malloc(sizeof(t_segmento));
    segmento->id = idSegmentoGlobal;
    idSegmentoGlobal++;

    segmento->direccionBase = direccion_base;
    segmento->size = size;

    if (list_add(memoria->segmentos, segmento) >=0 &&
        guardarSegmentoEnTabla(segmento, idProceso) >=0) {
    	// log_info(loggerMemoria, "Segmento id %d, de tamaño %zu, creado por proceso %d", segmento->id, size, idProceso);
        log_info(loggerMemoria,CREACION_DE_SEGMENTO,idProceso,segmento->id,segmento->direccionBase,segmento->size);
        log_info(loggerMemoria, "Logueo tabla segmentos: ");
        list_iterate(memoria->tablaDeSegmentos, (void*) iteratorTabla);
        log_info(loggerMemoria, "Logueo lista segmentos: ");
        list_iterate(memoria->segmentos, (void*) iteratorSegmento);
    	return true;
    }

    return false;
}

int guardarSegmentoEnTabla(t_segmento* segmento, int idProceso) {
    // Guardar segmento en la tabla de segmentos del proceso
    t_segmento_tabla* segmentoTabla = malloc(sizeof(t_segmento_tabla));
    segmentoTabla->segmento = segmento;
    segmentoTabla->idProceso = idProceso;
    return list_add(memoria->tablaDeSegmentos, segmentoTabla);
}

t_list* eliminar_segmento(int idProceso, int idSegmento) {
    list_sort(memoria->tablaDeSegmentos, (void*) comparar_tabla_segmentos_por_segmento_id);
    t_segmento_tabla* segmentoTabla = list_get(memoria->tablaDeSegmentos, idSegmento);
    if (segmentoTabla != NULL) {
        if (segmentoTabla->idProceso == idProceso) {
            list_sort(memoria->tablaDeSegmentos, (void*) comparar_segmentos_por_segmento_id);
            list_remove(memoria->segmentos, idSegmento);
            list_remove(memoria->tablaDeSegmentos, idSegmento);
            log_info(loggerMemoria, ELIMINACION_DE_SEGMENTO,
                idProceso, idSegmento, segmentoTabla->segmento->direccionBase, segmentoTabla->segmento->size);

            return memoria->tablaDeSegmentos;
        }
        log_error(loggerMemoria, "El proceso: No cuenta con los permisos suficientes para eliminar el segmento numero: %d", idProceso);
        return NULL;
    }
    log_warning(loggerMemoria, "Se pidió eliminar un segmento que no existe, segmento numero: %d", idSegmento);
    return NULL;
}


t_list* recalcular_huecos_libres() {
    list_clean(memoria->huecosLibres);

    // Ordenar la lista de segmentos por dirección base
    list_sort(memoria->tablaDeSegmentos, (void*) comparar_segmentos_por_direccion_base);

    int cantidadSegmentos = list_size(memoria->tablaDeSegmentos);

    for (int segmentoPosicion = 0; segmentoPosicion < cantidadSegmentos - 1; segmentoPosicion++) {
        t_segmento* segmentoActual = list_get(memoria->tablaDeSegmentos, segmentoPosicion);
        t_segmento* segmentoSiguiente = list_get(memoria->tablaDeSegmentos, segmentoPosicion);

        // Calcular la dirección base y el tamaño del hueco libre
        t_hueco_libre* huecoLibre = malloc(sizeof(t_hueco_libre));
        huecoLibre->direccionBase = segmentoActual->direccionBase + segmentoActual->size;
        huecoLibre->size = segmentoSiguiente->direccionBase - huecoLibre->direccionBase;

        if (huecoLibre->size > 0) {
            list_add(memoria->huecosLibres, huecoLibre);
            continue;
        }
        // Solo liberar cuando no se almacena en lista
        free(huecoLibre);
    }

    return memoria->huecosLibres;
}

void* leer_espacio_usuario(void* direccion, size_t size, int demora) {
    simular_tiempo_acceso(demora);

    // Realizar la lectura en la dirección indicada
    return memcpy(malloc(sizeof(direccion)), direccion, size);
}

void* escribir_espacio_usuario(void* direccion, size_t size, void* valor, int demora) {
    simular_tiempo_acceso(demora);

    // Realizar la escritura en la dirección indicada
    return memcpy(direccion, valor, size);
}




/////////////////////////////////////////////// Auxiliares ///////////////////////////////////

// Criterios de comparacion para t_list ////////////////////////
bool comparar_segmentos_por_direccion_base(const t_segmento* segmento1, const t_segmento* segmento2) {
    return segmento1->direccionBase <= segmento2->direccionBase;
}
bool comparar_segmentos_por_segmento_id(const t_segmento* segmento1, const t_segmento* segmento2) {
    return segmento1->id <= segmento2->id;
}
bool comparar_tabla_segmentos_por_segmento_id(const t_segmento_tabla* tablaSegmento1, const t_segmento_tabla* tablaSegmento2) {
    return tablaSegmento1->segmento->id <= tablaSegmento2->segmento->id;
}

bool filtrar_tabla_segmentos_por_proceso_id(const void* tablaElemento, void* idProceso) {
    int idProcesoCasteado = *((int*)idProceso);
    t_segmento_tabla* tablaElementoCasteado = (t_segmento_tabla*)tablaElemento;
    return (tablaElementoCasteado->idProceso == idProcesoCasteado);
}

t_list* obtener_tabla_segmentos_por_proceso_id(int procesoId) {
    t_segmento_tabla* segmentoTabla;
    t_list* segmentos = malloc(sizeof(t_list));
    segmentos = list_create();

    // TODO: Ver si se puede hacer algo del estilo segmentos = list_filter(memoria->tablaDeSegmentos, (procesoId == (.idProceso)).segmento)
    for(int i = 0; i < list_size(memoria->tablaDeSegmentos); i++) {
        segmentoTabla = list_get(memoria->tablaDeSegmentos, i);

        if (segmentoTabla->idProceso == procesoId) {
            list_add(segmentos, segmentoTabla->segmento);
        }
    }

    return segmentos;
}

// TODO Implementar recibir el segmento enviado por el kernel
t_segmento* recibir_segmento_kernel(t_list* pcbRecibido){
    t_segmento* segmento;
    

    return segmento;
}

////////////////////////////////////////////////

void* calcular_direccion(void* posicionBase, size_t desplazamiento) {
    return (void*)((uintptr_t)posicionBase + desplazamiento);
}

void simular_tiempo_acceso(int demora) {
    // Para convertir milisegundos a segundos, se divide el valor por 1000.
    sleep(demora / 1000);
}

size_t suma_size_huecos_disponibles(t_list* huecosLibres) {
    size_t sizeTotalHuecos = 0;
    for (int i = 0; i < list_size(huecosLibres); i++) {
        t_hueco_libre* hueco = list_get(huecosLibres, i);
        sizeTotalHuecos += hueco->size;
    }
    return sizeTotalHuecos;
}

/*
 * @param size_t size Tamaño pedido de memoria
 * Recorre memoria en busca de un espacio continuado libre con el tamaño dado
 * En caso de encontrarlo devuelve la posicion de memoria, sino devuelve NULL (segmentation fault)
 *
 * return void*
 */
void* buscar_espacio_contiguo(size_t size) {
    void* direccionBase;
    void* proximaDireccion = NULL;
    t_segmento* proximoSegmento;

    while(1) {
    	direccionBase = (proximaDireccion == NULL) ? memoria->espacioUsuario : proximaDireccion;
        proximoSegmento = ubicar_segmento_mas_cercano(memoria->tablaDeSegmentos, direccionBase);

        if (proximoSegmento == NULL) {
        	if ((uintptr_t)calcular_direccion(direccionBase, size) <=
        			(uintptr_t)calcular_direccion(memoria->espacioUsuario, memoria->sizeEspacioUsuario)) {
        		return direccionBase;
        	}
        	return NULL;
        }

        if ( (uintptr_t)calcular_direccion(direccionBase, size) < (uintptr_t)proximoSegmento->direccionBase ) {
        	return direccionBase;
        }

        // Para la siguiente iteración
        proximaDireccion = calcular_direccion(proximoSegmento->direccionBase, proximoSegmento->size);
    }
}

t_segmento* ubicar_segmento_mas_cercano(t_list* tablaSegmentos, void* direccion) {
    for (int i = 0; i < list_size(tablaSegmentos); i++) {
        t_segmento_tabla* fila = list_get(tablaSegmentos, i);

        if (fila->segmento->direccionBase >= direccion) {
                return fila->segmento;
        }
    }

    return NULL;
}

t_hueco_libre* proximo_hueco_libre(t_list* huecosLibres, void* direccion) {
    for (int i = 0; i < list_size(huecosLibres); i++) {
        t_hueco_libre* hueco = list_get(huecosLibres, i);

        if (hueco->direccionBase >= direccion) {
            return hueco;
        }
    }

    return NULL;
}
