#ifndef ADMINISTRAR_MEMORIA_H_
#define ADMINISTRAR_MEMORIA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include <shared/shared.h>
#include "compartido.h"

#define CREACION_DE_SEGMENTO        "PID: <%d> - Crear Segmento: %d - Base: %p - TAMAÑO: %zu"

#define ELIMINACION_DE_SEGMENTO     "PID: %d - Eliminar Segmento: %d - Base: %p - TAMAÑO: %zu"
#define I__SEGMENTO_0_CREADO        "Segmento0 generico creado de tamaño %zu"
#define INICIO_COMPACTACIÓN         "Solicitud de Compactación"
#define ACCESO_ESPACIO_USUARIO      "PID: %d - Acción: %s <LEER / ESCRIBIR> - Dirección física: <DIRECCIÓN_FÍSICA> - Tamaño: %zu<TAMAÑO> - Origen: %s <CPU / FS>"
#define FIN_ACCESO_ESPACIO_USUARIO  "PID: %d - Acción: %s <LEER / ESCRIBIR> Finalizada"



#endif
