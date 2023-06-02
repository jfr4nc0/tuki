#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "constantes.h"
#include "../../shared/structs.h"
#include "../../shared/constantes.h"
#include "../../shared/constructor.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"

#define DEFAULT_LOG_PATH            "logs/memoria.log"
#define DEFAULT_CONFIG_PATH         "memoria.config"

// LOGS ////////////////////////////////////
#define CREACION_DE_PROCESO         "Creación de Proceso PID: <PID>"
#define ELIMINACION_DE_PROCESO      "Eliminación de Proceso PID: <PID>"
#define CREACION_DE_SEGMENTO        "PID: <PID> - Crear Segmento: < id SEGMENTO> - Base: <DIRECCIÓN BASE> - TAMAÑO: <TAMAÑO>"
#define ELIMINACION_DE_SEGMENTO     "PID: <PID> - Eliminar Segmento: < id SEGMENTO> - Base: <DIRECCIÓN BASE> - TAMAÑO: <TAMAÑO>"
#define INICIO_COMPACTACIÓN         "Solicitud de Compactación"
#define RESULTADO_COMPACTACION      "Por cada segmento de cada proceso se deberá imprimir una línea con el siguiente formato:\n PID: <PID> - Segmento: < id SEGMENTO> - Base: <BASE> - Tamaño <TAMAÑO>"
#define ACCESO_ESPACIO_USUARIO      "PID: <PID> - Acción: <LEER / ESCRIBIR> - Dirección física: <DIRECCIÓN_FÍSICA> - Tamaño: <TAMAÑO> - Origen: <CPU / FS>"
///////////////////////////////////////////


#endif
