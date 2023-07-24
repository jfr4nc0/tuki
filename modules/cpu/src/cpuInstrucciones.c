#include "cpuInstrucciones.h"

int get_dir_fisica(t_segmento* segmento ,char* dir_logica, int segment_max){
	/*	Esquema de memoria: Segmentacion
	 * 	Direccion Logica: [ Nro Segmento | direccionBase ]
	 *	@return: La direccion fisica
	 */
	segmento->id = floor(atoi(dir_logica)/segment_max);
	segmento->direccionBase = atoi(dir_logica)%segment_max;
	segmento->size = segmento->id + segmento->direccionBase;

	if(segmento->size > segment_max){
		return -1;
	} else { return segmento->size; }
}

void* get_registro_cpu(char* registro, registros_cpu* registrosCpu){
	if (strcmp(registro, "AX") == 0) {
		return &(registrosCpu->AX);
	} else if (strcmp(registro, "BX") == 0) {
		return &(registrosCpu->BX);
	} else if (strcmp(registro, "CX") == 0) {
		return &(registrosCpu->CX);
	} else if (strcmp(registro, "DX") == 0) {
		return &(registrosCpu->DX);
	} else if (strcmp(registro, "EAX") == 0) {
		return &(registrosCpu->EAX);
	} else if (strcmp(registro, "EBX") == 0) {
		return &(registrosCpu->EBX);
	} else if (strcmp(registro, "ECX") == 0) {
		return &(registrosCpu->ECX);
	} else if (strcmp(registro, "EDX") == 0) {
		return &(registrosCpu->EDX);
	} else if (strcmp(registro, "RAX") == 0) {
		return &(registrosCpu->RAX);
	} else if (strcmp(registro, "RBX") == 0) {
		return &(registrosCpu->RBX);
	} else if (strcmp(registro, "RCX") == 0) {
		return &(registrosCpu->RCX);
	} else if (strcmp(registro, "RDX") == 0) {
		return &(registrosCpu->RDX);
	} else {
		return (int) -1;
	}
}

void cargar_data_instruccion(PCB* pcb, t_list* data_instruccion, char** instruccion, int cant){
	for(int i=0; i<=cant ;i++){
		list_add_in_index(data_instruccion,i,instruccion[i]);
	}
	pcb->data_instruccion = data_instruccion;
}

void instruccion_io(char* tiempo) { // TODO: @Francisca
	// Esta instrucción representa una syscall de I/O bloqueante. Se deberá devolver el Contexto de Ejecución actualizado al Kernel junto a la cantidad de unidades de tiempo que va a bloquearse el proceso.
}

void instruccion_f_seek(char* nombre_archivo, char* posicion) { // TODO: @Joan
	// Esta instrucción solicita al kernel actualizar el puntero del archivo a la posición pasada por parámetro.
}
void instruccion_f_read(char* nombre_archivo, char* dir_logica, char* cant_bytes) { // TODO: @Joan
	// Esta instrucción solicita al Kernel que se lea del archivo indicado, la cantidad de bytes pasada por parámetro y se escriba en la dirección física de Memoria la información leída.

}
void instruccion_f_write(char* nombre_archivo, char* dir_logica, char* cant_bytes) { // TODO: @Joan
	// Esta instrucción solicita al Kernel que se escriba en el archivo indicado, la cantidad de bytes pasada por parámetro cuya información es obtenida a partir de la dirección física de Memoria.
}
void instruccion_f_truncate(char* nombre_archivo,char* tamanio) { // TODO: @Joan
	// Esta instrucción solicita al Kernel que se modifique el tamaño del archivo al indicado por parámetro.
}
void instruccion_wait(char* recurso) { // TODO: @Sofia
	// Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.
}
void instruccion_signal(char* recurso) { // TODO: @Sofia
	// Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.
}
void instruccion_create_segment(char* id_segmento, char* tamanio) { // TODO: @Joan
	// Esta instrucción solicita al kernel la creación del segmento con el Id y tamaño indicado por parámetro.
}
void instruccion_delete_segment(char* id_segmento) { // TODO: @Joan
	// Esta instrucción solicita al kernel que se elimine el segmento cuyo Id se pasa por parámetro.
}
void instruccion_yield() {
	// Esta instrucción desaloja voluntariamente el proceso de la CPU. Se deberá devolver el Contexto de Ejecución actualizado al Kernel.
}
void instruccion_exit() {
	// Esta instrucción representa la syscall de finalización del proceso. Se deberá devolver el Contexto de Ejecución actualizado al Kernel para su finalización.
}