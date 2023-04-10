//TODO separar en archivos mas especificos. Quiza constantes.h configuraciones.h logs.h y messages.h

// Configuracion
#define LONGITUD_MAXIMA_CADENA 1000


// Signos
#define ENTER "\n"
#define SIGN_CONSOLA "> "
#define EMPTY_STRING ""

// CONSTANTES
#define MODO_LECTURA_ARCHIVO "r"



// ERROR MESSAGES
#define E__BAD_REQUEST "BAD REQUEST"
#define E__LOGGER_CREATE "No se pudo crear logger"
#define E__PAQUETE_CREATE	"Error al crear paquete"
#define E__ARCHIVO_CREATE	"Error al crear/leer archivo"

// LOGS
##define ABRIR_ARCHIVO "PID: <PID> - Abrir Archivo: <NOMBRE ARCHIVO>”
##define ACTUALIZAR_PUNTERO Archivo "PID: <PID> - Actualizar puntero Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO>” Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
##define CAMBIO_DE_ESTADO "PID: <PID> - Estado Anterior: <ESTADO_ANTERIOR> - Estado Actual: <ESTADO_ACTUAL>”
##define CERRAR_ARCHIVO "PID: <PID> - Cerrar Archivo: <NOMBRE ARCHIVO>”
##define CREACION_DE_PROCESO "Se crea el proceso <PID> en NEW”
##define CREAR_SEGMENTO "PID: <PID> - Crear Segmento - Id: <ID SEGMENTO> - Tamaño: <TAMAÑO>”
##define ELIMINAR_SEGMENTO "PID: <PID> - Eliminar Segmento - Id Segmento: <ID SEGMENTO>”
##define ESCRIBIR_ARCHIVO "PID: <PID> -  Escribir Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>”
##define FIN_COMPACTACIÓN "Se finalizó el proceso de compactación”
##define FIN_DE_PROCESO "Finaliza el proceso <PID> - Motivo: <SUCCESS / SEG_FAULT / OUT_OF_MEMORY>”
##define I/O:  “PID: <PID> - Ejecuta IO: <TIEMPO>”
##define INGRESO_A_READY "Cola Ready <ALGORITMO>: [<LISTA DE PIDS>]”
##define INICIO_COMPACTACIÓN "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>”
##define LEER_ARCHIVO "PID: <PID> - Leer Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>”
##define MOTIVO_DE_BLOQUEO "PID: <PID> - Bloqueado por: <IO / NOMBRE_RECURSO / NOMBRE_ARCHIVO>”
##define SIGNAL "PID: <PID> - Signal: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>” Nota: El valor de las instancias es después de ejecutar el Signal
##define TRUNCAR_ARCHIVO "PID: <PID> - Archivo: <NOMBRE ARCHIVO> - Tamaño: <TAMAÑO>”
##define WAIT "PID: <PID> - Wait: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>” Nota: El valor de las instancias es después de ejecutar el Wait
