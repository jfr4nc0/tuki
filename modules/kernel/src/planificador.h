#include <shared/shared.h>

#define CANTIDAD_ESTADOS 5

void inicializar_planificador();
void inicializar_listas_estados();
void proximo_a_ejecutar();
void cambiar_estado_pcb(PCB* ,pcb_estado ,int );
void agregar_a_lista_con_sem(void*, t_list *, sem_t);
void liberar_listas_estados();
void loggear_cola_ready(char*);
void cambiar_a_ready();
void agregar_pcb_a_paquete(t_paquete* , PCB* );
PCB* remover_de_lista(int, t_list*, sem_t);
char* pids_on_list(pcb_estado estado);

sem_t sem_proceso_a_ready;

pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t sem_lista_estados[CANTIDAD_ESTADOS];

char* get_nombre_estado(pcb_estado estado);

const char* nombres_estados[] = {
        "NEW",
        "READY",
        "BLOCKED",
        "EXECUTING",
        "EXIT"
};
