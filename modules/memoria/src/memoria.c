#include "memoria.h"
#include "administrarMemoria.h"

t_memoria_config* memoriaConfig;

int main() {
    loggerMemoria = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* configInicial = iniciar_config(DEFAULT_CONFIG_PATH, loggerMemoria);
    cargar_config_memoria(configInicial);

    int servidorMemoria = iniciar_servidor(configInicial, loggerMemoria);
    inicializar_memoria(memoriaConfig->TAM_MEMORIA, memoriaConfig->TAM_SEGMENTO_0,memoriaConfig->ALGORITMO_ASIGNACION);

	incializar_estructuras();
//	testing_funciones();

    atender_conexiones(servidorMemoria);

    terminar_programa(servidorMemoria, loggerMemoria, configInicial);
    free(memoriaConfig);
    liberar_memoria();

	return 0;
}

void incializar_estructuras(){
	pthread_mutex_init(&mutex_memoria_ocupada,NULL);
}

void testing_funciones(){
	codigo_operacion res;
	// res = inicializar_proceso(1,128);
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
		administrar_instrucciones(cliente, codigoDeOperacion);
		pthread_mutex_unlock(&mutex_memoria_ocupada);
    	}

    return;
}

void administrar_instrucciones(int cliente, codigo_operacion codigoDeOperacion) {
    codigo_operacion codigoRespuesta = AUX_ERROR;

	t_list* listaRecibida = recibir_paquete(cliente);
	int pid = *(int*)list_get(listaRecibida, 0);

	switch(codigoDeOperacion){
		case AUX_CREATE_PCB: //TODO FIX no crea el pcb, chequear si es correcto
		{
			codigoRespuesta = inicializar_proceso(pid, sizeof(int)); //TODO FIX
			t_list* obtenerSegmentosPorIdProceso = obtener_tabla_segmentos_por_proceso_id(pid);

			if (codigoRespuesta == AUX_OK) {
				log_info(loggerMemoria, CREACION_DE_PROCESO, pid);
				enviar_operacion(cliente, codigoRespuesta, sizeof(obtenerSegmentosPorIdProceso), obtenerSegmentosPorIdProceso);
			} else { enviar_codigo_operacion(cliente, codigoRespuesta); }
			break;
		}
		case I_CREATE_SEGMENT:
		{
			t_segmento* segmento = recibir_segmento_kernel(listaRecibida);
			codigoRespuesta = crear_segmento_por_pid(pid, segmento);

			if(codigoRespuesta == AUX_OK){
				enviar_operacion(cliente,codigoRespuesta, sizeof(segmento->direccionBase), segmento->direccionBase);
			} else { enviar_codigo_operacion(cliente, codigoRespuesta);}
			break;
		}
		case I_DELETE_SEGMENT:
		{
			t_segmento* segmento = recibir_segmento_kernel(listaRecibida);
			if(eliminar_segmento(pid, segmento->id)!=NULL){
				enviar_codigo_operacion(cliente, AUX_OK);
			} else {enviar_codigo_operacion(cliente, AUX_ERROR);}

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
			finalizar_proceso(pid);
			// enviar_codigo_operacion(cliente,codigoRespuesta);
			break;
		}
		default:
		{
			log_error(loggerMemoria,E__CODIGO_INVALIDO);
			enviar_codigo_operacion(cliente, AUX_ERROR);
			break;
		}
	}
}
