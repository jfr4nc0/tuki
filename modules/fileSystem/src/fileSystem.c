#include "fileSystem.h"


int main(int argc, char** argv) {
    loggerFileSystem = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, loggerFileSystem);
    inicializar_estructuras(config);

    int conexionMemoria = armar_conexion(config, MEMORIA, loggerFileSystem);
    // Notifico a memoria que soy el módulo file system
    enviar_codigo_operacion(conexionMemoria, AUX_SOY_FILE_SYSTEM);

    int servidorFileSystem = iniciar_servidor(config, loggerFileSystem);

    atender_kernel(servidorFileSystem);

    terminar_programa(servidorFileSystem, loggerFileSystem, config);
    liberar_conexion(conexionMemoria);

}

void atender_kernel(int servidorFileSystem) {
    while (1) {
        int clienteAceptado = esperar_cliente(servidorFileSystem, loggerFileSystem);
        pthread_t thread_kernel;

        pthread_create(&thread_kernel, NULL, (void*)ejecutar_instrucciones_kernel, (void*) (intptr_t) clienteAceptado);
        pthread_join(thread_kernel, NULL);


        liberar_conexion(clienteAceptado);
    }
}

void iterator(char* value) {
    log_info(loggerFileSystem, "%s ", value);
}

void ejecutar_instrucciones_kernel(void* cliente) {
	int clienteKernel = (int) (intptr_t) cliente;

    codigo_operacion operacionRecibida = recibir_operacion(clienteKernel);
    codigo_operacion codigoRespuesta = AUX_ERROR;


    switch(operacionRecibida) {
  		case I_F_OPEN:
            char* nombreArchivo = obtener_mensaje_de_socket(clienteKernel);
			log_info(loggerFileSystem, "Por leer archivo: %s", nombreArchivo);

			if (existe_archivo(nombreArchivo)) {
                enviar_codigo_operacion(clienteKernel, AUX_OK);
			} else {
                enviar_codigo_operacion(clienteKernel, AUX_ERROR);
            }
			break;
    }
    return;
}

char* obtener_mensaje_de_socket(int cliente) {
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;
	buffer = recibir_buffer(&tamanio, cliente);
	return leer_string(buffer, &desplazamiento);
}
