#include "../include/utils.h"

int validarArgumentos(int argc, char** argv) {
	if (argc<2) {
			printf(": Enviar dos parametros (pathConfig y pathInstrucciones) \n");
			printf("Parametros enviados: %d\n", argc);
			for(int i=0; i<argc; i++) {
				printf("%s\n", argv[i]);
			}
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
