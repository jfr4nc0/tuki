#ifndef CONSOLA_GENERAL_H_
#define CONSOLA_GENERAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>

#include <shared/shared.h>


void validarArgumentos(int, char**);
void enviarInstrucciones(char*, int, t_log*);

#define DEFAULT_INSTRUCCIONES_PATH   "tuki-pruebas/prueba-base/instrucciones.txt"
#define DEFAULT_LOG_PATH             "logs/consola.log"
// /home/utnso/eclipse-workspace/tp-2023-1c-KernelPanic/
#define DEFAULT_PATH_CONFIG          "tuki-pruebas/prueba-base/consola.config"
   

#endif
