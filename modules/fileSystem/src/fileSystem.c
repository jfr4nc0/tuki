#include "fileSystem.h"


int main(int argc, char** argv) {
    loggerFileSystem = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);
    t_config* config = iniciar_config(argv[1], loggerFileSystem);
    inicializar_estructuras(config);

    pthread_mutex_init(&m_instruccion, NULL);

    conexionMemoria = armar_conexion(config, MEMORIA, loggerFileSystem);
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

	while(1) {
    codigo_operacion operacionRecibida = recibir_operacion(clienteKernel);
    codigo_operacion codigoRespuesta = AUX_ERROR;

		switch(operacionRecibida) {
			case KERNEL_CREAR_ARCHIVO: {
				char* nombreArchivo = obtener_mensaje_de_socket(clienteKernel);
				log_info(loggerFileSystem, "Por crear archivo: %s", nombreArchivo);
				devolver_instruccion_generico(crear_archivo(nombreArchivo), clienteKernel);
				break;
			}
			case I_F_OPEN: {
				char* nombreArchivo = obtener_mensaje_de_socket(clienteKernel);
				log_info(loggerFileSystem, "Por leer archivo: %s", nombreArchivo);

				devolver_instruccion_generico(existe_archivo(nombreArchivo), clienteKernel);
				free(nombreArchivo);
				break;
			}
			case I_TRUNCATE: {
				// pthread_mutex_lock(&m_instruccion);
//				t_archivo_abierto* archivo = obtener_archivo_completo_de_socket(clienteKernel);
                t_list* listaConParametros = recibir_paquete(clienteKernel);
                char* nombreArchivo = (char*)list_get(listaConParametros, 0);
                char* punteroTexto = (char*)list_get(listaConParametros, 1);

                char *ptr;
                uint32_t puntero = strtoul(punteroTexto, &ptr, 10);

				// archivo->puntero en este caso es la cantidad a truncar
				devolver_instruccion_generico(truncar_archivo(nombreArchivo, puntero), clienteKernel);
				// free(archivo);
				// pthread_mutex_unlock(&m_instruccion);
				break;
			}
			case I_F_READ: {
				// pthread_mutex_lock(&m_instruccion);
				char *nombreArchivo = NULL;
				uint32_t direccionFisica, cantidadBytes, puntero, pidProceso;
				recibir_buffer_lectura_archivo(clienteKernel, &nombreArchivo, &puntero, &direccionFisica, &cantidadBytes, &pidProceso);
				devolver_instruccion_generico(leer_archivo(nombreArchivo, puntero, direccionFisica, cantidadBytes, pidProceso), clienteKernel);
				free(nombreArchivo);
				// pthread_mutex_unlock(&m_instruccion);
				break;
			}
			case I_F_WRITE: {
				// pthread_mutex_lock(&m_instruccion);
				char *nombreArchivo = NULL;
				uint32_t direccionFisica, cantidadBytes, puntero, pidProceso;
				recibir_buffer_escritura_archivo(clienteKernel, &nombreArchivo, &puntero, &direccionFisica, &cantidadBytes, &pidProceso);
				solicitar_informacion_memoria(direccionFisica, cantidadBytes, pidProceso);
				char* informacionAEscribir = (char*)recibir_buffer_informacion_memoria(cantidadBytes);
				devolver_instruccion_generico(escribir_archivo(informacionAEscribir, nombreArchivo, puntero, cantidadBytes), clienteKernel);

				free(nombreArchivo);
				// pthread_mutex_unlock(&m_instruccion);
				break;
			}
    	}
    }
    return;
}

void devolver_instruccion_generico(bool funciono, int cliente) {
    codigo_operacion codigoRespuesta = funciono ? AUX_OK : AUX_ERROR;

    return enviar_codigo_operacion(cliente, codigoRespuesta);
}
char* obtener_mensaje_de_socket(int cliente) {
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;
	buffer = recibir_buffer(&tamanio, cliente);
	return leer_string(buffer, &desplazamiento);
}

t_archivo_abierto* obtener_archivo_completo_de_socket(int cliente) {
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;
	buffer = recibir_buffer(&tamanio, cliente);

    t_archivo_abierto* archivo = malloc(sizeof(t_archivo_abierto));
    archivo->nombreArchivo = leer_string(buffer, &desplazamiento);
    archivo->puntero = leer_uint32(buffer, &desplazamiento);
	return archivo;
}

void recibir_buffer_lectura_archivo(int clienteKernel, char **nombreArchivo, uint32_t *puntero,
uint32_t *direccionFisica, uint32_t *cantidadBytes, uint32_t *pid) {
    int tamanio = 0;
    t_buffer *bufferLectura = recibir_buffer(&tamanio, clienteKernel);

    char *nombreArchivoLectura = buffer_unpack_string(bufferLectura);
    *nombreArchivo = strdup(nombreArchivoLectura);
    free(nombreArchivoLectura);

    uint32_t punteroBuffer;
    buffer_unpack(bufferLectura, &punteroBuffer, sizeof(punteroBuffer));
    *puntero = punteroBuffer;

    uint32_t direccion;
    buffer_unpack(bufferLectura, &direccion, sizeof(direccion));
    *direccionFisica = direccion;

    uint32_t bytes;
    buffer_unpack(bufferLectura, &bytes, sizeof(bytes));
    *cantidadBytes = bytes;

    uint32_t pidProceso;
    buffer_unpack(bufferLectura, &pidProceso, sizeof(pidProceso));
    *pid = pidProceso;

    free(bufferLectura->stream);
    free(bufferLectura);
    return;
}

// Recibir información de Memoria para escribir en los bloques.
void* recibir_buffer_informacion_memoria(uint32_t cantidadBytes) {
    int tamanio = 0;
    recibir_operacion(conexionMemoria);
    void* informacionRecibida = malloc(cantidadBytes);

    t_buffer *bufferInformacion = recibir_buffer(&tamanio, conexionMemoria);
    buffer_unpack(bufferInformacion, informacionRecibida, cantidadBytes);

    free(bufferInformacion->stream);
    free(bufferInformacion);
    return informacionRecibida;
}


void recibir_buffer_escritura_archivo(int clienteKernel, char **nombreArchivo, uint32_t *puntero, uint32_t *direccionFisica, uint32_t *cantidadBytes, uint32_t* pid)
{
    int tamanio = 0;
    t_buffer *bufferLectura = recibir_buffer(&tamanio, clienteKernel);

    char *nombreArchivoLectura = buffer_unpack_string(bufferLectura);
    *nombreArchivo = strdup(nombreArchivoLectura);
    free(nombreArchivoLectura);

    uint32_t punteroBuffer;
    buffer_unpack(bufferLectura, &punteroBuffer, sizeof(punteroBuffer));
    *puntero = punteroBuffer;

    uint32_t direccion;
    buffer_unpack(bufferLectura, &direccion, sizeof(direccion));
    *direccionFisica = direccion;

    uint32_t bytes;
    buffer_unpack(bufferLectura, &bytes, sizeof(bytes));
    *cantidadBytes = bytes;

    uint32_t pidProceso;
    buffer_unpack(bufferLectura, &pidProceso, sizeof(pidProceso));
    *pid = pidProceso;

    free(bufferLectura->stream);
    free(bufferLectura);
    return;
}


void solicitar_informacion_memoria(uint32_t direccionFisica, uint32_t cantidadBytes, uint32_t pid) {
    t_buffer *bufferLectura = buffer_create();
    buffer_pack(bufferLectura, &direccionFisica, sizeof(uint32_t));
    buffer_pack(bufferLectura, &cantidadBytes, sizeof(uint32_t));
    buffer_pack(bufferLectura, &pid, sizeof(uint32_t));
    stream_send_buffer(conexionMemoria, I_F_WRITE, bufferLectura);
    free(bufferLectura->stream);
    free(bufferLectura);
    return;
}
