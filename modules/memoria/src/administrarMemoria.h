#ifndef ADMINISTRAR_MEMORIA_H_
#define ADMINISTRAR_MEMORIA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include <shared/shared.h>
#include "compartido.h"

#define ELIMINACION_DE_SEGMENTO     "PID: %d - Eliminar Segmento: %d - Base: %p - TAMAÑO: %zu"
#define I__SEGMENTO_0_CREADO        "Segmento0 generico creado de tamaño %zu"
#define INICIO_COMPACTACIÓN         "Solicitud de Compactación"
#define ACCESO_ESPACIO_USUARIO      "PID: %d - Acción: %s <LEER / ESCRIBIR> - Dirección física: <DIRECCIÓN_FÍSICA> - Tamaño: %zu<TAMAÑO> - Origen: %s <CPU / FS>"
#define FIN_ACCESO_ESPACIO_USUARIO  "PID: %d - Acción: %s <LEER / ESCRIBIR> Finalizada"

// Estructura para representar la memoria
typedef struct {
    void* espacioUsuario; // Espacio de memoria
    size_t sizeEspacioUsuario;
    t_list* segmentos;
    t_list* huecosLibres;
    t_list* tablaDeSegmentos;
} t_memoria;

typedef struct {
    void* direccionBase;
    size_t size;
    int id;
} t_segmento;

// Elemento de la tabla segmento
typedef struct {
    t_segmento* segmento;
    int idProceso;
} t_segmento_tabla;

typedef struct {
    void* direccionBase;
    size_t size;
} t_hueco_libre;

extern t_log* loggerMemoria;

void inicializar_memoria(size_t sizeMemoriaTotal, size_t sizeSegmento0);
void liberar_memoria();
void* calcular_direccion(void*, size_t);
void* crear_segmento(int idProceso, size_t size);
t_list* eliminar_segmento(int idProceso, int segmentoId);
void* leer_espacio_usuario(void* direccion, size_t size, int demora);
void* escribir_espacio_usuario(void* direccion, size_t size, void* valor, int demora);
void simular_tiempo_acceso(int);
void* buscar_espacio_contiguo(size_t);
t_hueco_libre* proximo_hueco_libre(t_list*, void*);
t_segmento* ubicar_segmento_mas_cercano(t_list*, void*);
t_list* recalcular_huecos_libres();
bool comparar_segmentos_por_direccion_base(const t_segmento*, const t_segmento*);
bool añadir_segmento(int idProceso, size_t size, void* direccion_base);
bool comparar_tabla_segmentos_por_segmento_id(const t_segmento_tabla* segmento1, const t_segmento_tabla* segmento2);
bool comparar_segmentos_por_segmento_id(const t_segmento* segmento1, const t_segmento* segmento2);
size_t suma_size_huecos_disponibles(t_list* huecosLibres);
codigo_operacion inicializar_proceso(int idProceso, size_t pcbSize);
void finalizar_proceso(int idProceso);
void compactar_memoria();
int guardarSegmentoEnTabla(t_segmento* segmento, int idProceso);

void iteratorTabla(t_segmento_tabla* elemento);

void iteratorSegmento(t_segmento* elemento);
t_list* obtener_tabla_segmentos_por_proceso_id(int procesoId);

#endif