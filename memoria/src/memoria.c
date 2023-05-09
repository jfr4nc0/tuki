#include "../include/memoria.h"

#include "../../shared/src/funciones.c"
#include "utils.c"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int servidorMemoria = iniciar_servidor(config, logger);
    handshakeConCPU(servidorMemoria);



    //terminar_programa(cliente_aceptado, logger, config);
}

void handshakeConCPU(int servidorMemoria){
	int cliente_aceptado = esperar_cliente(servidorMemoria, logger);
	char* mensaje_de_cpu = receive(cliente_aceptado);
	if (strcmp(mensaje_de_cpu, "CPU") == 0) {
	    send(cliente_aceptado, "MEMORIA", 7, 0);
	    printf("Handshake exitoso.\n");
	} else {
	    printf("Error en el handshake.\n");
	}
	free(mensaje_de_cpu);
}
