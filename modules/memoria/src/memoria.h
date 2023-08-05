#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include <shared/shared.h>
#include "compartido.h"

t_log* loggerMemoria;

//pthread_mutex_t mutex_memoria_ocupada;

typedef struct {
	int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_SEGMENTO_0;
    int CANT_SEGMENTOS;
    int RETARDO_MEMORIA;
    int RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
} t_memoria_config;

typedef struct {
	int idProceso;
    size_t size;
} t_parametros_write_read;


#define DEFAULT_CONFIG_PATH         "tuki-pruebas/prueba-memoria/memoria.config"

#define DEFAULT_MEMORIA_LOG_PATH    "../../../logs/memoria.log"

// NOWNOWNOW
typedef struct {
        void* base;
        int tamanio;
        bool libre;
} t_hueco;


typedef struct
{
    int cantidad_parametros;
    char **parametros;
} t_parametros_variables;

void* memoria_principal;
t_list* lista_huecos;
t_list* tabla_segmentos_global;

extern pthread_mutex_t m_memoria;

void cargar_config_memoria(t_config*);
void inicializar_memoria(int tamanio_memoria, int tamanio_segmento_0, char* algoritmo);
t_list* crear_tabla_segmentos();
void agregar_segmento_0(t_list* tabla_segmentos);
void atender_conexiones(int socket_servidor);

void ejecutar_filesystem_pedido(void* socket);
void recibir_acceso(t_parametros_variables **parametros, int *PID, int socket);
int leer_int_memoria(void *buffer, int *desplazamiento);
t_parametros_variables* deserealizar_motivos_desalojo(void *buffer, int*desplazamiento);
char *leer_valor_direccion_fisica(long direccion_fisica, int tamanio, int pid, char *origen);
void escribir_valor_direccion_fisica(char *valor, long direccion_fisica, int pid, char *origen);
void enviar_mensaje_memoria(char* mensaje, int socket_cliente);
void *serializar_paquete(t_paquete *paquete, int bytes);
void liberar_parametros_desalojo(t_parametros_variables *parametros_variables);
void vaciar_parametros_desalojo(t_parametros_variables *parametros);
void vaciar_parametros_desalojo(t_parametros_variables *parametros);


void compactar();
void eliminar_segmento(t_list *tabla_segmentos, int id_segmento, int PID);
int obtener_index_tabla_segmentos(int PID);

////////////
t_hueco* get_hueco_con_best_fit(int tamanio);
t_hueco* get_hueco_con_worst_fit(int tamanio);
bool comprobar_compactacion(int tamanio);
void modificar_lista_huecos(t_hueco* hueco, int tamanio);
////////////

void ejecutar_cpu_pedido(void*);

void ejecutar_kernel_pedido(void* socket_modulo);
int recibir_int(int socket);
void agregar_a_paquete_dato_serializado(t_paquete *paquete, void *valor, int tamanio);
void finalizar_proceso(t_list *tabla_segmentos, int PID);
void liberar_segmento(t_segmento *segmento);
void liberar_tabla_segmentos(t_tabla_segmentos *ts);
void comprobar_consolidacion_huecos_aledanios(int index_hueco);
//t_ctx *recibir_contexto(int socket);
//t_ctx *deserializar_contexto(void *buffer, int *desplazamiento);
// void crear_segmento(PCB *proceso);
t_paquete* crear_segmento(int id_segmento, int tamanio, PCB* pcb, int socket);

void terminar_programa_memoria(int conexion, t_log* logger, t_config* config);

void escribir_valor_en_memoria(int dirFisica, void* bytesRecibidos, uint32_t tamanio, int pid, char *origen);

char* leer_espacio_usuario(void* direccion, size_t size);

void escribir_espacio_usuario(void* direccion, size_t size, void* valor);

/*
// LOGS ////////////////////////////////////
#define CREACION_DE_PROCESO         "Creación de Proceso PID: %d"
#define ELIMINACION_DE_PROCESO      "Eliminación de Proceso PID: %d"
#define RESULTADO_COMPACTACION      "Por cada segmento de cada proceso se deberá imprimir una línea con el siguiente formato:\n PID: %d - Segmento: %d - Base: %d - Tamaño %d"

#define I__RECIBO_INSTRUCCION        "Me llegaron los siguientes valores para la operacion numero: %d desde %s"

///////////////////////////////////////////

void cargar_config_memoria(t_config*);
void ejecutar_file_system_pedido(void *);
void ejecutar_cpu_pedido(void *);
void ejecutar_kernel_pedido(void *);
void ejecutar_instrucciones(int, char*);
void atender_conexiones(int);

void administrar_instrucciones(int , codigo_operacion , char*);
void administrar_cliente(void*);
void testing_funciones();
void incializar_estructuras();
void enviar_confirmacion(int conexion, codigo_operacion codOperacion);
t_parametros_write_read* obtenerParametrosWriteRead(t_list* listaRecibida);
*/
#endif
