#include "memoria.h"
#include "administrarMemoria.h"

t_memoria_config* memoriaConfig;

int main(int argc, char** argv) {
    loggerMemoria = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* configInicial = iniciar_config(argv[1], loggerMemoria);
    cargar_config_memoria(configInicial);

    int servidorMemoria = iniciar_servidor(configInicial, loggerMemoria);
    inicializar_memoria(memoriaConfig->TAM_MEMORIA, memoriaConfig->TAM_SEGMENTO_0,memoriaConfig->ALGORITMO_ASIGNACION);
	pthread_mutex_init(&mutex_memoria_ocupada,NULL);

    atender_conexiones(servidorMemoria);

    terminar_programa(servidorMemoria, loggerMemoria, configInicial);
    free(memoriaConfig);
    liberar_memoria();

	return 0;
}


void testing_funciones(){
	codigo_operacion res;
	res = inicializar_proceso(1, 128);
	// res = crear_segmento_por_pid(1,100);
	// res = crear_segmento_por_pid(1,80);
}

void atender_conexiones(int servidorMemoria) {
    while (1){
    	int clienteAceptado = esperar_cliente(servidorMemoria, loggerMemoria);
    	log_trace(loggerMemoria, "cliente: %d",clienteAceptado);
		pthread_t hilo_administrar_cliente;
		pthread_create(&hilo_administrar_cliente, NULL, (void*) administrar_cliente, (void*) (intptr_t) clienteAceptado);
		pthread_detach(hilo_administrar_cliente);
    }
}

void administrar_cliente(void* clienteAceptado) {
        pthread_t thread_cpu;
        pthread_t thread_kernel;
        pthread_t thread_file_system;

        log_trace(loggerMemoria, "cliente: %d",(int)(intptr_t)clienteAceptado);
        // Recibo una primera operación para saber que módulo se conectó
		int modulo = recibir_operacion((int)(intptr_t)clienteAceptado); // recibe ok
		log_trace(loggerMemoria, "Modulo: %d",modulo);
		// No se almacena ya que se ignora, pero necesito llamarlo para liberar el mensaje
		recibir_paquete((int)(intptr_t)clienteAceptado);

        switch(modulo) {
            case AUX_SOY_CPU:
                pthread_create(&thread_cpu, NULL, (void*) ejecutar_cpu_pedido, clienteAceptado);
                pthread_join(thread_cpu, NULL);
            break;
            case AUX_SOY_KERNEL:
    		    pthread_create(&thread_kernel, NULL, (void*)ejecutar_kernel_pedido, clienteAceptado);
    		    pthread_join(thread_kernel, NULL);
            break;
            case AUX_SOY_FILE_SYSTEM:
    		    pthread_create(&thread_file_system, NULL, (void*)ejecutar_file_system_pedido, clienteAceptado);
    		    pthread_join(thread_file_system, NULL);
            break;
        }
}

void ejecutar_file_system_pedido(void *clienteAceptado) {
	int  conexionConFileSystem = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConFileSystem, FILE_SYSTEM);
}

void ejecutar_cpu_pedido(void *clienteAceptado) {
	int  conexionConCPU = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConCPU, CPU);
}


void ejecutar_kernel_pedido(void *clienteAceptado) {
	int  conexionConKernel = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConKernel, KERNEL);
}

void cargar_config_memoria(t_config* configInicial) {
    memoriaConfig = malloc(sizeof(t_memoria_config));
    memoriaConfig->PUERTO_ESCUCHA = extraer_int_de_config(configInicial, "PUERTO_ESCUCHA", loggerMemoria);
    memoriaConfig->TAM_MEMORIA = extraer_int_de_config(configInicial, "TAM_MEMORIA", loggerMemoria);
    memoriaConfig->TAM_SEGMENTO_0 = extraer_int_de_config(configInicial, "TAM_SEGMENTO_0", loggerMemoria);
    memoriaConfig->CANT_SEGMENTOS = extraer_int_de_config(configInicial, "CANT_SEGMENTOS", loggerMemoria);
    memoriaConfig->RETARDO_MEMORIA = extraer_int_de_config(configInicial, "RETARDO_MEMORIA", loggerMemoria);
    memoriaConfig->RETARDO_COMPACTACION = extraer_int_de_config(configInicial, "RETARDO_COMPACTACION", loggerMemoria);
    memoriaConfig->ALGORITMO_ASIGNACION = extraer_string_de_config(configInicial, "ALGORITMO_ASIGNACION", loggerMemoria);

    log_info(loggerMemoria, I__CONFIG_GENERIDO_CARGADO, MEMORIA);
}

void iterator(char* value) {
    log_info(loggerMemoria, "%s ", value);
}

void ejecutar_instrucciones(int cliente, char* modulo) {
	log_info(loggerMemoria, "Esperando instrucciones de: %s ", modulo);

	while(1){
		codigo_operacion codigoDeOperacion = recibir_operacion(cliente);
		log_info(loggerMemoria, I__RECIBO_INSTRUCCION, codigoDeOperacion, modulo);
		pthread_mutex_lock(&mutex_memoria_ocupada);
		administrar_instrucciones(cliente, codigoDeOperacion, modulo);
		pthread_mutex_unlock(&mutex_memoria_ocupada);
    }

    return;
}

void administrar_instrucciones(int cliente, codigo_operacion codigoDeOperacion, char* modulo) {
    codigo_operacion codigoRespuesta = AUX_ERROR;

	switch(codigoDeOperacion){
		case I_F_WRITE: {
			int tamanio = 0;
			int desp = 0;
//			recibir_operacion(cliente);
			char* buffer = recibir_buffer(&tamanio, cliente);
			int pid = leer_int(buffer, &desp);
			void* direccionFisica = leer_puntero(buffer, &desp);
			uint32_t cantidadBytes = leer_uint32(buffer, &desp);
			char* respuesta = leer_espacio_usuario(direccionFisica, cantidadBytes, memoriaConfig->RETARDO_MEMORIA);

			t_paquete* paquete = crear_paquete(AUX_OK);
			agregar_puntero_a_paquete(paquete, respuesta);
			enviar_paquete(paquete, cliente);
			eliminar_paquete(paquete);
			/*
				agregar_int_a_paquete(paquete, pidProceso);
				agregar_puntero_a_paquete(paquete, direccionFisica);
				agregar_uint32_a_paquete(paquete, cantidadBytes);
				enviar_paquete(paquete, cliente);
			 */
		}
		case I_F_READ: {

		}
		case AUX_CREATE_PCB:
		{
			void* buffer;
			int tamanio = 0;
			int desplazamiento = 0;

			buffer = recibir_buffer(&tamanio, cliente);
			int pid = leer_int(buffer, &desplazamiento);
			codigoRespuesta = inicializar_proceso(pid, sizeof(int));
			t_list* listaSegmentosPorPid = obtener_tabla_segmentos_por_proceso_id(pid);
			log_info(loggerMemoria, CREACION_DE_PROCESO, pid);
			if (codigoRespuesta == AUX_OK) {
				enviar_lista_segmentos_del_proceso(cliente, listaSegmentosPorPid, loggerMemoria);
			} else {
				enviar_codigo_operacion(cliente, codigoRespuesta);
			}
			break;
		}
		case I_CREATE_SEGMENT:
		{
			t_segmento_tabla* tabla_segmento = recibir_segmento_por_pid(cliente);
			 codigoRespuesta = crear_segmento_por_pid(tabla_segmento->idProceso, tabla_segmento->segmento);
			if(codigoRespuesta == AUX_OK){
				t_list* listaSegmentosPorPid = obtener_tabla_segmentos_por_proceso_id(tabla_segmento->idProceso);
				enviar_lista_segmentos_del_proceso(cliente, listaSegmentosPorPid, loggerMemoria);
			} else { enviar_codigo_operacion(cliente, codigoRespuesta);}
			break;
		}
		case I_DELETE_SEGMENT:
		{
			t_segmento_tabla* tabla_segmento = recibir_segmento_por_pid(cliente);
			codigoRespuesta = eliminar_segmento(tabla_segmento->idProceso, tabla_segmento->segmento);
			if(codigoRespuesta == AUX_OK){
				t_list* listaSegmentosPorPid = obtener_tabla_segmentos_por_proceso_id(tabla_segmento->idProceso);
				enviar_lista_segmentos_del_proceso(cliente, listaSegmentosPorPid, loggerMemoria);
			} else {
				enviar_codigo_operacion(cliente, codigoRespuesta);
			}
			break;
		}
		case AUX_SOLICITUD_COMPACTACION:
		{
			compactar_memoria();
			// enviar_codigo_operacion(cliente, codigoRespuesta);
			break;
		}
		case AUX_FINALIZAR_PROCESO:
		{
			t_list* listaRecibida = recibir_paquete(cliente);
			int pid = *(int*)list_get(listaRecibida, 0);
			finalizar_proceso(pid);
			// enviar_codigo_operacion(cliente,codigoRespuesta);
			break;
		}
		case I_MOV_IN:{

			int tamanio = 0;
			int desplazamiento = 0;

			t_buffer* buffer = recibir_buffer(&tamanio, cliente);

			int id_proceso = leer_int(buffer, &desplazamiento);
			uint32_t direccionFisica = leer_uint32(buffer, &desplazamiento);
			uint32_t tamanio_a_leer= leer_uint32(buffer, &desplazamiento);

			leer_espacio_usuario((void*) direccionFisica, (size_t) tamanio_a_leer, memoriaConfig->RETARDO_MEMORIA);
			log_info(loggerMemoria, LOG_ESCRIBIR_LEER, id_proceso, "LEER", direccionFisica, tamanio_a_leer, modulo);
			enviar_confirmacion(cliente, AUX_OK);

			break;
		}
		case I_MOV_OUT:{

			int tamanio = 0;
			int desplazamiento = 0;

			t_buffer* buffer = recibir_buffer(&tamanio, cliente);

			int id_proceso = leer_int(buffer, &desplazamiento);
			uint32_t direccionFisica = leer_uint32(buffer, &desplazamiento);
			uint32_t tamanio_a_leer= leer_uint32(buffer, &desplazamiento);
			char* bytes_a_escribir = leer_string(buffer, &desplazamiento);

			escribir_espacio_usuario((void*) direccionFisica, (size_t) tamanio_a_leer, (void*)bytes_a_escribir, memoriaConfig->RETARDO_MEMORIA);
			log_info(loggerMemoria, LOG_ESCRIBIR_LEER, id_proceso, "ESCRIBIR", direccionFisica, tamanio_a_leer, modulo);
			enviar_confirmacion(cliente, AUX_OK);

			break;
		}
		default:
		{
			log_error(loggerMemoria, E__CODIGO_INVALIDO);
			enviar_codigo_operacion(cliente, AUX_ERROR);
			liberar_conexion(cliente);
			exit(EXIT_FAILURE);
			return;
			break;
		}
	}
}

void enviar_confirmacion(int conexion, codigo_operacion codOperacion) {
	if (conexion > 0) {
		t_paquete* paquete = crear_paquete(codOperacion);
		enviar_paquete(paquete, conexion);
		free(paquete);
	}
}
