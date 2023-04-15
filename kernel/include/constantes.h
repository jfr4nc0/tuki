#ifndef CONSTANTES_KERNEL_H_
#define CONSTANTES_KERNEL_H_

/////// LOGS OBLIGATORIOS///////////
#define ABRIR_ARCHIVO               "PID: <PID> - Abrir Archivo: <NOMBRE ARCHIVO>"
#define ACTUALIZAR_PUNTERO_ARCHIVO     "PID: <PID> - Actualizar puntero Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO>" // Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
#define CAMBIO_DE_ESTADO            "PID: <PID> - Estado Anterior: <ESTADO_ANTERIOR> - Estado Actual: <ESTADO_ACTUAL>"
#define CERRAR_ARCHIVO              "PID: <PID> - Cerrar Archivo: <NOMBRE ARCHIVO>"
#define CREACION_DE_PROCESO         "Se crea el proceso <PID> en NEW"
#define CREAR_SEGMENTO                 "PID: <PID> - Crear Segmento - Id: <ID SEGMENTO> - Tamaño: <TAMAÑO>"
#define ELIMINAR_SEGMENTO           "PID: <PID> - Eliminar Segmento - Id Segmento: <ID SEGMENTO>"
#define ESCRIBIR_ARCHIVO            "PID: <PID> -  Escribir Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define FIN_COMPACTACIÓN            "Se finalizó el proceso de compactación"
#define FIN_DE_PROCESO              "Finaliza el proceso <PID> - Motivo: <SUCCESS / SEG_FAULT / OUT_OF_MEMORY>"
#define I_O                         "PID: <PID> - Ejecuta IO: <TIEMPO>"
#define INGRESO_A_READY             "Cola Ready <ALGORITMO>: [<LISTA DE PIDS>]"
#define INICIO_COMPACTACIÓN         "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>"
#define LEER_ARCHIVO                "PID: <PID> - Leer Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define MOTIVO_DE_BLOQUEO           "PID: <PID> - Bloqueado por: <IO / NOMBRE_RECURSO / NOMBRE_ARCHIVO>"
#define SIGNAL                      "PID: <PID> - Signal: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Signal
#define TRUNCAR_ARCHIVO             "PID: <PID> - Archivo: <NOMBRE ARCHIVO> - Tamaño: <TAMAÑO>"
#define WAIT                        "PID: <PID> - Wait: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Wait
////////////////////////////////////

#define PATH_LOG_KERNEL             "logs/kernel.log"
#define PATH_CONFIG_KERNEL             "kernel.config"

#endif
