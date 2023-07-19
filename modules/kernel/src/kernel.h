#ifndef KERNEL_H_
#define KERNEL_H_

#include <shared/shared.h>

/*------------------ VARIABLES GLOBALES --------------*/
int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

int contadorProcesoId = 1; // El 0 se lo dejamos al segmento0

t_log* kernelLogger;


/*----------------- STRUCTS ------------------*/
typedef enum {
    ENUM_NEW,
    ENUM_READY,
    ENUM_EXECUTING,
    ENUM_BLOCKED,
    ENUM_EXIT,
}pcb_estado;

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
	int id_proceso; // Identificador del proceso, unico en todo el sistema
	pcb_estado estado;
	t_list* lista_instrucciones; // Lista de instrucciones a ejecutar
	int contador_instrucciones; // Numero de la proxima instruccion a ejecutar
	registros_cpu* registrosCpu;
	t_list* lista_segmentos;
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
	double processor_burst; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	double ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
}PCB;

typedef struct{
    char* nombre;
    int instancias;
    // t_list* lista_procesos;
    sem_t sem_recurso;
}t_recurso;

typedef struct timespec timestamp;

/*----------------- FUNCIONES ------------------*/

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void recibir_de_consola(void*);
void iterator(char* value);
PCB* nuevo_proceso(t_list* , int);
void proceso_a_ready();
void mostrar_pcb(PCB*);

static bool criterio_hrrn(PCB*, PCB*);
double calculo_HRRN(PCB*);
double rafaga_estimada(PCB*);
void *__ejecucion_desalojo_pcb(void *);
PCB* elegir_pcb_segun_fifo();
PCB* elegir_pcb_segun_hrrn();
void *manejo_desalojo_pcb(void *);
void recibir_proceso_desalojado(PCB* );

void crear_hilo_planificador(int);
void proximo_a_ejecutar();
char* pids_on_list(pcb_estado estado);

void inicializar_semaforos();
void crear_cola_recursos(char*, int);
void inicializar_diccionario_recursos();
int inicializar_servidor_kernel(void);

/// Funciones de listas ///
void cambiar_estado_proceso(PCB*, pcb_estado);
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
void envio_pcb(int , PCB* , codigo_operacion );

// PRUEBAS
void envio_pcb_a_cpu(int , PCB* , codigo_operacion );
void agregar_pcb_a_paquete_para_cpu(t_paquete* , PCB* );
void agregar_registros_a_paquete_cpu(t_paquete* , registros_cpu* );

////////////////////////////////////////////////////

int obtener_recursos(int);

/*----------------- SEMAFOROS / HILOS ------------------*/
sem_t sem_proceso_a_ready;
sem_t sem_grado_multiprogamacion;
sem_t sem_cpu_disponible;

pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t sem_lista_estados[CANTIDAD_ESTADOS];

t_dictionary* diccionario_recursos;


/*-------------------- LOGS OBLIGATORIOS ------------------*/
#define ABRIR_ARCHIVO               "PID: <PID> - Abrir Archivo: <NOMBRE ARCHIVO>"
#define ACTUALIZAR_PUNTERO_ARCHIVO     "PID: <PID> - Actualizar puntero Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO>" // Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
#define CAMBIO_DE_ESTADO            "PID: <PID> - Estado Anterior: <ESTADO_ANTERIOR> - Estado Actual: <ESTADO_ACTUAL>"
#define CERRAR_ARCHIVO              "PID: <PID> - Cerrar Archivo: <NOMBRE ARCHIVO>"
#define CREACION_DE_PROCESO         "Se crea el proceso <PID> en NEW"
#define CREAR_SEGMENTO                 "PID: <PID> - Crear Segmento - Id: < id SEGMENTO> - Tamaño: <TAMAÑO>"
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
////////////////////////////////////

#define PATH_LOG_KERNEL             "logs/kernel.log"
#define PATH_CONFIG_KERNEL          "tuki-pruebas/prueba-base/kernel.config"

#endif
