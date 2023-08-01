#ifndef CPU_INSTRUCCIONES_H
#define CPU_INSTRUCCIONES_H
/*
>>>>>>> main
#include <shared/shared.h>

typedef struct {
	int RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

int get_dir_fisica(t_segmento*,char*, int);
void* get_registro_cpu(char*, registros_cpu*);
void cargar_data_instruccion(PCB*, t_list*, char**, int);

void instruccion_mov_in(char* registro,char* dir_logica, PCB* pcb);
void instruccion_mov_out(char* dir_logica,char* registro, PCB* pcb);
void instruccion_io(char* tiempo);
void instruccion_f_open(char* nombre_archivo);
void instruccion_f_close(char* nombre_archivo);
void instruccion_f_seek(char* nombre_archivo, char* posicion);
void instruccion_f_read(char* nombre_archivo, char* dir_logica, char* cant_bytes);
void instruccion_f_write(char* nombre_archivo, char* dir_logica, char* cant_bytes);
void instruccion_f_truncate(char* nombre_archivo,char* tamanio);
void instruccion_wait(char* recurso);
void instruccion_signal(char* recurso);
void instruccion_create_segment(char* id_segmento, char* tamanio);
void instruccion_delete_segment(char* id_segmento);
void instruccion_yield();
void instruccion_exit();
*/


#endif
