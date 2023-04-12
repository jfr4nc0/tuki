#ifndef STRUCTS_GLOBAL_H_
#define STRUCTS_GLOBAL_H_

// Este struct op_code va a tener los comandos

// TODO: Idea de como separar comandos, o quiza convenga todos separados
/*
F_WRITE ARCHIVO 4 4 	F_READ ARCHIVO 16 4  	-> Comando Archivo INT INT
F_TRUNCATE ARCHIVO 64   F_SEEK ARCHIVO 10       -> Comando Archivo INT
WAIT DISCO				SIGNAL DISCO			-> Comando Disco
F_OPEN ARCHIVO			F_CLOSE ARCHIVO			-> COmando ARchivo
YIELD					EXIT					-> Comando

SET AX HOLA
MOV_OUT 120 AX
I/O 10
MOV_IN BX 120

CREATE_SEGMENT 1 128
DELETE_SEGMENT 1
*/

	typedef enum
	{
		MENSAJE,
		PAQUETE
	}op_code;

	typedef struct
	{
		int size;
		void* stream;
	}t_buffer;

	typedef struct
	{
		op_code codigo_operacion;
		t_buffer* buffer;
	}t_paquete;

#endif
