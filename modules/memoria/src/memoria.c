#include "memoria.h"

t_log* loggerMemoria;
t_memoria_config* memoriaConfig;
t_tabla_segmentos* tablaSegmentos;

int main() {
    loggerMemoria = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* configInicial = iniciar_config(DEFAULT_CONFIG_PATH, loggerMemoria);

    int servidorMemoria = iniciar_servidor(configInicial, loggerMemoria);

    while (1){
		int clienteAceptado = esperar_cliente(servidorMemoria, loggerMemoria);
        // Recibo una primera operación para saber que módulo se conectó
        int codigoDeOperacion = recibir_operacion(clienteAceptado);
        pthread_t thread_cpu;
        pthread_t thread_kernel;
        pthread_t thread_file_system;

        switch(codigoDeOperacion) {
            case AUX_SOY_CPU:
                pthread_create(&thread_cpu, NULL, (void*) ejecutar_cpu_pedido, (void*) (intptr_t) clienteAceptado);
                pthread_join(thread_cpu, NULL);
            break;
            case AUX_SOY_KERNEL:
    		    pthread_create(&thread_kernel, NULL, (void*)ejecutar_kernel_pedido, (void*) (intptr_t) clienteAceptado);
    		    pthread_join(thread_kernel, NULL);
            break;
            case AUX_SOY_FILE_SYSTEM:
    		    pthread_create(&thread_file_system, NULL, (void*)ejecutar_file_system_pedido, (void*) (intptr_t) clienteAceptado);
    		    pthread_join(thread_file_system, NULL);
            break;
        }
    }

    inicializar_segmento_generico();

    terminar_programa(servidorMemoria, loggerMemoria, configInicial);
    free(memoriaConfig);
    free(segmentoGeneral);
    free(tablaSegmentos);

	return 0;
}


void ejecutar_file_system_pedido(void *clienteAceptado) {
	int  conexionConFileSystem = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones_memoria(conexionConFileSystem, FILE_SYSTEM);
}

void ejecutar_cpu_pedido(void *clienteAceptado) {
	int  conexionConCPU = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones_memoria(conexionConCPU, CPU);
}


void ejecutar_kernel_pedido(void *clienteAceptado) {
	int  conexionConKernel = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones_memoria(conexionConKernel, KERNEL);
}

// Creo segmento de memoria generico que puede ser usado por los demas módulos
void inicializar_segmento_generico() {
    segmentoGeneral = malloc(memoriaConfig->TAM_SEGMENTO_0);

    if (segmentoGeneral == NULL) {
        log_error(loggerMemoria, E__MALLOC_ERROR, memoriaConfig->TAM_SEGMENTO_0);
        abort();
    }
    log_info(loggerMemoria, "Segmento generico creado");

    return;
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


void cargarTablaDeSegmentos(int max_segmentos) {
    tablaSegmentos->segmentos = malloc(max_segmentos * sizeof(t_segmento));
    tablaSegmentos->cantidad_segmentos_usados = 0;
    tablaSegmentos->capacidad_segmentos = max_segmentos;
}

void agregar_segmento(t_tabla_segmentos* tabla, t_segmento nuevo_segmento) {
    if (tabla->cantidad_segmentos_usados < tabla->capacidad_segmentos) {
        tabla->segmentos[tabla->cantidad_segmentos_usados] = nuevo_segmento;
        tabla->cantidad_segmentos_usados++;

        return;
    }

    // TODO: ver como manejar Error
    log_error(loggerMemoria, "No se puede agregar más segmentos, se alcanzó el límite máximo");
    abort();
}

void iterator(char* value) {
    log_info(loggerMemoria, "%s ", value);
}


void ejecutar_instrucciones_memoria(int cliente, char* modulo) {
    int codigoDeOperacion = recibir_operacion(cliente);
    log_info(loggerMemoria, I__RECIBO_INSTRUCCION, codigoDeOperacion, modulo);
    t_list* listaRecibida = recibir_paquete(cliente);
    list_iterate(listaRecibida, (void*) iterator);
    enviar_mensaje("Instrucción recibida", cliente, loggerMemoria);

    return;
}
