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

typedef struct timespec timestamp;

typedef struct ParametrosHiloIO {
    uint32_t idProceso;
    char *nombreArchivo;
    uint32_t punteroArchivo;
    uint32_t direccionFisica;
    uint32_t cantidadBytes;
    uint32_t pidProceso;
} t_parametros_hilo_IO;

typedef struct {
    codigo_operacion operacion;
    PCB* pcb;
    char** instruccion;
}t_data_desalojo;
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

pthread_mutex_t permiso_compactacion;
pthread_mutex_t mutex_memoria;

/*----------------- FUNCIONES ------------------*/

void iteratorConLog(char* value);
t_segmento* buscar_segmento(t_list* listaSegmentos, int id_segmento);
void inicializar_estructuras();
void _planificador_largo_plazo();
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
PCB* nuevo_proceso(t_list* , int);
void enviar_proceso_a_ready();

double calculo_HRRN(PCB*);
double rafaga_estimada(PCB*);
void *__ejecucion_desalojo_pcb(void *);
PCB* elegir_pcb_segun_fifo();
PCB* elegir_pcb_segun_hrrn();
void manejo_desalojo_pcb();
codigo_operacion manejo_instrucciones(t_data_desalojo* data);

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
void mover_de_lista_con_sem(int idProceso, int estadoNuevo, int estadoAnterior);
//////////////////

PCB* recibir_proceso_desajolado(PCB* pcb_en_ejecucion);
void destruir_segmento(t_segmento* segmento);

PCB *remover_pcb_segun_maximo_hrrn(pcb_estado *estado);
PCB *__estado_obtener_pcb_segun_maximo_hrrn_atomic(pcb_estado * estado);
PCB *__estado_obtener_pcb_segun_maximo_hrrn(pcb_estado *estado);
double __calcular_valor_hrrn(PCB *pcb, double tiempoActual);
PCB* obtener_maximo_por_R(t_list* lista_procesos);
//double obtener_diferencial_de_tiempo_en_milisegundos(timestamp *end, timestamp *start);
t_list* recibir_todas_las_tablas_segmentos(int socket_cliente);
void actualizar_todas_las_tablas_de_segmentos(t_list* nuevas_tablas);

t_list* recibir_tabla_segmentos(int socket_cliente);
PCB *buscar_proceso(int pid_buscado);


// t_semaforo_recurso* diccionario_semaforos_recursos_get_semaforo_recurso(tablaArchivosAbiertos, nombreArchivo);

t_estado* crear_archivo_estado(t_nombre_estado nombreEstado);

void instruccion_wait(PCB *, char *);

////////////////////////////////////////////////////

int obtener_recursos(int);

void enviar_f_read_write(PCB* pcb, char** instruccion, codigo_operacion codigoOperacion, int direccionFisica);

void terminar_proceso(PCB* , codigo_operacion);
void instruccion_signal(PCB *pcb_en_ejecucion, char *nombre_recurso);

void cambiar_estado_proceso_sin_semaforos(PCB* pcb, pcb_estado estadoNuevo);
t_archivo_abierto* encontrar_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo);

int encontrar_index_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo);
t_semaforo_recurso* inicializar_archivo_estado(t_nombre_estado nombreEstado);

int obtener_index_pcb_de_lista(int estado, int idProceso);

char* obtener_motivo(codigo_operacion );
void crear_segmento(PCB* pcb_recibido, char* id_segmento, char* tamanio);


/*----------------- SEMAFOROS / HILOS ------------------*/
sem_t sem_proceso_a_ready_inicializar;
sem_t sem_proceso_a_ready_terminado;
sem_t sem_grado_multiprogamacion;
sem_t sem_cpu_disponible;
sem_t proceso_para_finalizar;
sem_t proceso_en_exit;
sem_t sem_compactacion;

pthread_t planificador_largo_plazo;
pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t sem_lista_estados[CANTIDAD_ESTADOS];
pthread_mutex_t* mutex_lista_estados[CANTIDAD_ESTADOS];

pthread_mutex_t* mutexTablaAchivosAbiertos;
pthread_mutex_t sem_proceso_a_executing;

t_dictionary* diccionario_recursos;
t_dictionary* tablaArchivosAbiertos;

/*-------------------- LOGS OBLIGATORIOS ------------------*/
#define ABRIR_ARCHIVO               "PID: <%d> - Abrir Archivo: <%s> realizado"
#define ABRIR_ARCHIVO_BLOQUEADO     "PID: <%d> - Esperando para abrir Archivo: <%s>"
#define ACTUALIZAR_PUNTERO_ARCHIVO  "PID: <%d> - Actualizar puntero Archivo: <%s> - Puntero <%d>" // Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
#define CERRAR_ARCHIVO              "PID: <%d> - Cerrar Archivo: <%s> terminado"
#define CERRAR_ARCHIVO_DESBLOQUEA_PCB "PID: <%d> - Al cerrar el Archivo: <%s> debloquea al PID <%d>"
#define CREACION_DE_PROCESO         "Se crea el proceso <%d> en NEW"
#define CREAR_SEGMENTO              "PID: <%d> - Crear Segmento - Id: <%d> - Tamaño: <%zu>"
#define ELIMINAR_SEGMENTO           "PID: <%d> - Eliminar Segmento - Id Segmento: <%d>"
#define ESCRIBIR_ARCHIVO            "PID: <%d> -  Escribir Archivo: <%s> - Puntero <%d> - Dirección Memoria <%p> - Tamaño <%zu>"


#define FIN_COMPACTACIÓN            "Se finalizó el proceso de compactación"
#define FIN_DE_PROCESO              "Finaliza el proceso %d - Motivo: %s" // MOTIVOS PUEDEN SER SUCCESS / SEG_FAULT / OUT_OF_MEMORY
#define I_O                         "PID: %d - Ejecuta IO: <TIEMPO>"
#define INGRESO_A_READY             "Cola Ready %s: [<LISTA DE PIDS>]"
#define INICIO_COMPACTACIÓN         "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>"
#define LEER_ARCHIVO                "PID: %d - Leer Archivo: %s - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define MOTIVO_DE_BLOQUEO           "PID: %d - Bloqueado por: <IO / NOMBRE_RECURSO / NOMBRE_ARCHIVO>"
#define SIGNAL                      "PID: %d - Signal: %s - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Signal
#define TRUNCAR_ARCHIVO             "PID: %d - Archivo: %s - Tamaño: <TAMAÑO>"
#define WAIT                        "PID: %d - Wait: %s - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Wait
#define LOG_CAMBIO_DE_ESTADO        "PID: %d - Estado Anterior: %s - Estado Actual: %s"
#define F_SEEK_HECHO                "PID: %d - Instruccion F_SEEK hecha correctamente, archivo %s ahora apuntando al puntero %d"
#define E__CREAR_SEGMENTO           "PID: %d - ERROR en crear Segmento - Id: %d - Tamaño: %d"
#define E__ELIMINAR_SEGMENTO        "Error al eliminar el segmento <Id:%d>"
#define E__PERMISOS_INSUFICIENTES   "Permisos insuficientes para <PID:%d>"
////////////////////////////////////

#define PATH_LOG_KERNEL             "../../../logs/kernel.log"
#define PATH_CONFIG_KERNEL          "tuki-pruebas/prueba-memoria/kernel.config"

#endif
