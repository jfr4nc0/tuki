#ifndef UTILS_GLOBAL_H_

	#define UTILS_GLOBAL_H_

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
