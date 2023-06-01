#ifndef CPU_CONSTANTES_H_
#define CPU_CONSTANTES_H_

// LOGS ////////////////////////
#define INSTRUCCION_EJECUTADA        "PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>"
#define ACCESO_MEMORIA               "PID: <PID> - Acción: <LEER / ESCRIBIR> - Segmento: <NUMERO SEGMENTO> - Dirección Física: <DIRECCION FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
#define ERROR_SEGMENTATION_FAULT     "PID: <PID> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>"
////////////////////////////////

#define DEFAULT_LOG_PATH      "../logs/cpu.log"
#define DEFAULT_CONFIG_PATH   "cpu.config"

/**************** INSTRUCCIONES ****************/


#endif
