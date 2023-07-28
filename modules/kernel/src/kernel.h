#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/dictionary.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#include <shared/shared.h>

/*------------------ VARIABLES GLOBALES --------------*/
int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

int contadorProcesoId = 1; // El 0 se lo dejamos al segmento0

t_log* kernelLogger;

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
    char* IP_FILE_SYSTEM;
    char* PUERTO_FILE_SYSTEM;
    char* IP_CPU;
    char* PUERTO_CPU;
    char* IP_ESCUCHA;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    double ESTIMACION_INICIAL;
    double HRRN_ALFA;
    int GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;
}t_kernel_config;

t_kernel_config* kernelConfig;

typedef struct {
    char* nombre;
    int instancias;
    t_list* procesos_bloqueados;
    sem_t sem_recurso;
}t_recurso;


typedef struct ParametrosHiloIO {
    uint32_t idProceso;
    char *nombreArchivo;
    uint32_t punteroArchivo;
    uint32_t direccionFisica;
    uint32_t cantidadBytes;
    uint32_t pidProceso;
} t_parametros_hilo_IO;


typedef struct {
    t_nombre_estado nombreEstado;
    t_list* listaProcesos;
    sem_t* semaforoEstado;
    pthread_mutex_t* mutexEstado;
} t_estado;
typedef struct {
    int instancias;
    t_estado* estadoRecurso;
}t_semaforo_recurso;

typedef struct timespec timestamp;

pthread_mutex_t permiso_compactacion;

/*----------------- FUNCIONES ------------------*/

void inicializar_estructuras();
void _planificador_largo_plazo();
void* liberar_pcb_de_exit();
void destruir_pcb(PCB* pcb);
PCB *desencolar_primer_pcb(pcb_estado estado);
void _planificador_corto_plazo();
double obtener_diferencial_de_tiempo_en_milisegundos(timestamp *end, timestamp *start);
void pcb_estimar_proxima_rafaga(PCB *pcb_ejecutado, double tiempo_en_cpu);
void set_timespec(timestamp *timespec);


t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void recibir_de_consola(void*);
void iterator(char* value);
PCB* nuevo_proceso(t_list* , int);
void enviar_proceso_a_ready();
void mostrar_pcb(PCB*);

static bool criterio_hrrn(PCB*, PCB*);
double calculo_HRRN(PCB*);
double rafaga_estimada(PCB*);
void *__ejecucion_desalojo_pcb(void *);
PCB* elegir_pcb_segun_fifo();
PCB* elegir_pcb_segun_hrrn();
void *manejo_desalojo_pcb();
void recibir_proceso_desalojado(PCB*, int );
PCB* recibir_pcb_de_cpu();

void crear_hilo_planificadores();
void proximo_a_ejecutar();
char* pids_on_list(pcb_estado estado);

void inicializar_semaforos();
void crear_cola_recursos(char*, int);
void inicializar_diccionario_recursos();
int inicializar_servidor_kernel(void);

/// Funciones de listas ///
void cambiar_estado_proceso_con_semaforos(PCB*, pcb_estado);
void agregar_a_lista_con_sem(void*, int);
void liberar_listas_estados();
void inicializar_listas_estados();
void mover_de_lista_con_sem(void*, int, int);
//////////////////

// Funciones para enviar un pcb a cpu //////////////
void agregar_pcb_a_paquete(t_paquete* , PCB* );
void agregar_long_a_paquete(t_paquete* , long );
void agregar_longlong_a_paquete(t_paquete* , long long );
void agregar_lista_a_paquete(t_paquete* , t_list* );
void agregar_int_a_paquete(t_paquete* , int );
void agregar_arreglo_a_paquete(t_paquete* , char** );
void agregar_valor_a_paquete(t_paquete* , void* , int );
void agregar_registros_a_paquete(t_paquete* , registros_cpu* );
void agregar_registro4bytes_a_paquete(t_paquete* , char[4] );
void agregar_registro8bytes_a_paquete(t_paquete* , char[8] );
void agregar_registro16bytes_a_paquete(t_paquete* , char[16] );

void envio_pcb(int , PCB* , codigo_operacion );

// PRUEBAS
void envio_pcb_a_cpu(int , PCB* , codigo_operacion );
void agregar_pcb_a_paquete_para_cpu(t_paquete* , PCB* );
void agregar_registros_a_paquete_cpu(t_paquete* , registros_cpu* );
PCB* recibir_proceso_desajolado(PCB* pcb_en_ejecucion);
// t_semaforo_recurso* diccionario_semaforos_recursos_get_semaforo_recurso(tablaArchivosAbiertos, nombreArchivo);

t_estado* crear_archivo_estado(t_nombre_estado nombreEstado);

////////////////////////////////////////////////////

int obtener_recursos(int);
void terminar_proceso(PCB* , codigo_operacion);
void instruccion_signal(PCB *pcb_en_ejecucion, char *nombre_recurso);
void enviar_f_read_write(PCB* pcb, char**, codigo_operacion);
void cambiar_estado_proceso_sin_semaforos(PCB* pcb, pcb_estado estadoNuevo);
t_archivo_abierto* encontrar_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo);

int encontrar_index_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo);
void agregar_lista_archivos_a_paquete(t_paquete* paquete, t_list* lista);
t_semaforo_recurso* inicializar_archivo_estado(t_nombre_estado nombreEstado);

t_list* archivosAbiertosGlobal;

/*----------------- SEMAFOROS / HILOS ------------------*/
sem_t sem_proceso_a_ready_inicializar;
sem_t sem_proceso_a_ready_terminado;
sem_t sem_proceso_a_executing;
sem_t sem_grado_multiprogamacion;
sem_t sem_cpu_disponible;

pthread_t planificador_largo_plazo;
pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t sem_lista_estados[CANTIDAD_ESTADOS];
pthread_mutex_t* mutex_lista_estados[CANTIDAD_ESTADOS];

pthread_mutex_t* mutexTablaAchivosAbiertos;

t_dictionary* diccionario_recursos;
t_dictionary* tablaArchivosAbiertos;


/*-------------------- LOGS OBLIGATORIOS ------------------*/
#define ABRIR_ARCHIVO               "PID: <PID> - Abrir Archivo: <NOMBRE ARCHIVO>"
#define ACTUALIZAR_PUNTERO_ARCHIVO     "PID: <PID> - Actualizar puntero Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO>" // Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
#define CERRAR_ARCHIVO              "PID: <PID> - Cerrar Archivo: <NOMBRE ARCHIVO>"
#define CREACION_DE_PROCESO         "Se crea el proceso <PID> en NEW"
#define CREAR_SEGMENTO              "PID: <PID> - Crear Segmento - Id: < id SEGMENTO> - Tamaño: <TAMAÑO>"
#define ELIMINAR_SEGMENTO           "PID: <PID> - Eliminar Segmento - Id Segmento: < id SEGMENTO>"
#define ESCRIBIR_ARCHIVO            "PID: <PID> -  Escribir Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define FIN_COMPACTACIÓN            "Se finalizó el proceso de compactación"
#define FIN_DE_PROCESO              "Finaliza el proceso <PID> - Motivo: <SUCCESS / SEG_FAULT / OUT_OF_MEMORY>"
#define I_O                         "PID: <PID> - Ejecuta IO: <TIEMPO>"
#define INGRESO_A_READY             "Cola Ready <ALGORITMO>: [<LISTA DE PIDS>]"
#define INICIO_COMPACTACIÓN         "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>"
#define LEER_ARCHIVO                "PID: <PID> - Leer Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define MOTIVO_DE_BLOQUEO           "PID: <PID> - Bloqueado por: <IO / NOMBRE_RECURSO / NOMBRE_ARCHIVO>"
#define SIGNAL                      "PID: <PID> - Signal: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Signal
#define TRUNCAR_ARCHIVO             "PID: <PID> - Archivo: <NOMBRE ARCHIVO> - Tamaño: <TAMAÑO>"
#define WAIT                        "PID: <PID> - Wait: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Wait
#define LOG_CAMBIO_DE_ESTADO "PID: %d - Estado Anterior: %s - Estado Actual: %s"

////////////////////////////////////


#define PATH_LOG_KERNEL             "logs/kernel.log"
#define PATH_CONFIG_KERNEL          "tuki-pruebas/prueba-base/kernel.config"

#endif
