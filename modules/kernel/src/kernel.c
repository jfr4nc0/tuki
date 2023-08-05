#include "kernel.h"

void liberar_recursos_kernel() {
    free(kernelConfig);
    liberar_listas_estados();
    liberar_conexion(conexionCPU);
    liberar_conexion(conexionMemoria);
    liberar_conexion(conexionFileSystem);
}


int main(int argc, char** argv) {
    kernelLogger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);
    t_config* config = iniciar_config(argv[1], kernelLogger);
    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    if(conexionMemoria > 0){
    	enviar_codigo_operacion(conexionMemoria, AUX_SOY_KERNEL);
    }
    conexionCPU = armar_conexion(config, CPU, kernelLogger);

    cargar_config_kernel(config, kernelLogger);

    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);


    int servidorKernel = iniciar_servidor(config, kernelLogger);

    inicializar_estructuras();

    inicializar_diccionario_recursos();

    inicializar_escucha_conexiones_consolas(servidorKernel);

    /*TODO: NUNCA LLEGA ACA PORQUE SE QUEDA ESPERANDO NUEVAS CONSOLAS,
    MOVER ESTAS FUNCIONES A CUANDO EL SISTEMA SOLICITE LA FINALIZACION*/

    terminar_programa(servidorKernel, kernelLogger, config);
    liberar_recursos_kernel();

    return 0;
}

void inicializar_estructuras(){
    inicializar_listas_estados();
/*
    inicializar_archivo_estado(ENUM_FREE);
    inicializar_archivo_estado(ENUM_BLOCK);
*/
    inicializar_semaforos();
    crear_hilo_planificadores();

}

void cargar_config_kernel(t_config* config, t_log* kernelLogger) {
    kernelConfig = malloc(sizeof(t_kernel_config));

    kernelConfig->IP_MEMORIA = extraer_string_de_config(config, "IP_MEMORIA", kernelLogger);
    kernelConfig->PUERTO_MEMORIA = extraer_string_de_config(config, "PUERTO_MEMORIA", kernelLogger);
    kernelConfig->IP_FILE_SYSTEM = extraer_string_de_config(config, "IP_FILE_SYSTEM", kernelLogger);
    kernelConfig->PUERTO_FILE_SYSTEM = extraer_string_de_config(config, "PUERTO_FILE_SYSTEM", kernelLogger);
    kernelConfig->IP_CPU = extraer_string_de_config(config, "IP_CPU", kernelLogger);
    kernelConfig->PUERTO_CPU = extraer_string_de_config(config, "PUERTO_CPU", kernelLogger);
    kernelConfig->PUERTO_ESCUCHA = extraer_string_de_config(config, "PUERTO_ESCUCHA", kernelLogger);
    kernelConfig->ALGORITMO_PLANIFICACION = extraer_string_de_config(config, "ALGORITMO_PLANIFICACION", kernelLogger);
    kernelConfig->ESTIMACION_INICIAL = extraer_int_de_config(config, "ESTIMACION_INICIAL", kernelLogger);
    kernelConfig->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    kernelConfig->GRADO_MAX_MULTIPROGRAMACION = extraer_int_de_config(config, "GRADO_MAX_MULTIPROGRAMACION", kernelLogger);
    kernelConfig->RECURSOS = config_get_array_value(config, "RECURSOS");
    kernelConfig->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    return;
}


void inicializar_escucha_conexiones_consolas(int servidorKernel) {

    while(1) {

        int conexionConConsola = esperar_cliente(servidorKernel, kernelLogger);
        log_debug(kernelLogger, "Se conecto una consola");
        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, (void*) recibir_de_consola, (void*) (intptr_t) conexionConConsola);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void enviar_proceso_a_ready() {
    while(1) {
    	log_info(kernelLogger, "esperando nuevas consolas");
        sem_wait(&sem_proceso_a_ready_inicializar);
        sem_wait(&sem_lista_estados[ENUM_NEW]);

        PCB* pcb = list_get(lista_estados[ENUM_NEW], 0);

        pthread_mutex_lock(&mutex_memoria);
        if (conexionMemoria > 0) {
            // Creo el pcb en memoria

        	t_paquete* paquete = crear_paquete(AUX_CREATE_PCB);
        	agregar_int_a_paquete(paquete, pcb->id_proceso);
        	enviar_paquete(paquete, conexionMemoria);
        	eliminar_paquete(paquete);

        	int tamanio = 0;
        	int desplazamiento = 0;

        	recibir_operacion(conexionMemoria);
        	char* buffer = recibir_buffer(&tamanio, conexionMemoria);
        	pcb->lista_segmentos = deserializar_tabla_segmentos(buffer, &desplazamiento);
        	free(buffer);

            //list_iterate(pcb->lista_segmentos, (void*)iteratorConLog);
        	/*

            codigo_operacion codigoRespuesta = recibir_operacion(conexionMemoria);
            log_warning(kernelLogger, "el cod op recibido de memoria es %d", codigoRespuesta);
            if (codigoRespuesta == AUX_ERROR) {
                log_error(kernelLogger, "Segmentation fault la creacion del proceso %d, ", pcb->id_proceso);
            } else if (codigoRespuesta == AUX_SOLO_CON_COMPACTACION) {
                log_info(kernelLogger, "Se necesita compactar para proceso %d", pcb->id_proceso);
            } else if (codigoRespuesta == AUX_OK) {
                pcb->lista_segmentos = recibir_lista_segmentos(conexionMemoria);
                log_debug(kernelLogger, "Se creó en memoria el proceso %d, semgmentos creados: %d", pcb->id_proceso, list_size(pcb->lista_segmentos));
                mostrarListaSegmentos(pcb->lista_segmentos);
            } else {
                log_error(kernelLogger, "Error interno en Modulo Memoria para crear proceso id: %d.", pcb->id_proceso);
            }
            */
        }
        pthread_mutex_unlock(&mutex_memoria);

        sem_wait(&sem_grado_multiprogamacion);
        sem_wait(&sem_lista_estados[ENUM_READY]);
        pcb->ready_timestamp = time(NULL);
        cambiar_estado_proceso_sin_semaforos(pcb, ENUM_READY);
        sem_post(&sem_lista_estados[ENUM_NEW]);
        sem_post(&sem_lista_estados[ENUM_READY]);
        sem_post(&sem_proceso_a_ready_terminado);
    }
}

void iteratorConLog(char* value) {
    log_debug(kernelLogger, "%s", value);
}

void iterator_id_proceso(PCB* pcb) {
    log_debug(kernelLogger, "%d ", pcb->id_proceso);
}

void recibir_de_consola(void *clienteAceptado) {

    int  conexionConConsola = (int) (intptr_t)clienteAceptado;
    recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iteratorSinLog);

    nuevo_proceso(listaInstrucciones, conexionConConsola);


    list_destroy(listaInstrucciones);


    return;
}

void crear_hilo_planificadores() {

    pthread_create(&planificador_largo_plazo, NULL, (void*) _planificador_largo_plazo, NULL);
    pthread_detach(planificador_largo_plazo);

    pthread_create(&planificador_corto_plazo, NULL, (void*) _planificador_corto_plazo, NULL);
    pthread_detach(planificador_corto_plazo);
}

void _planificador_largo_plazo() {
    pthread_t hilo_procesos_a_ready;
    pthread_create(&hilo_procesos_a_ready, NULL, (void*) enviar_proceso_a_ready, NULL);
    pthread_detach(hilo_procesos_a_ready);

    return;
}

void destruir_pcb(PCB* pcb) {
    //pthread_mutex_lock(pcb_get_mutex(pcb));

    list_destroy_and_destroy_elements(pcb->lista_instrucciones, (void*)free);

    free(pcb->registrosCpu);
    //list_destroy_and_destroy_elements(pcb->lista_segmentos, (void*)destruir_segmento);

    list_destroy_and_destroy_elements(pcb->lista_archivos_abiertos, (void*)free);


    free(pcb);
}

void destruir_segmento(t_segmento* segmento){

	free(segmento->direccionBase);
	free(segmento);
}

PCB* desencolar_primer_pcb(pcb_estado estado) {
    //int estadoNumerico = estado;
    sem_wait(&proceso_para_finalizar);
    sem_wait(&sem_lista_estados[estado]);
    // pthread_mutex_lock(mutex_lista_estados[estado_]);
    PCB *pcb = (PCB*) list_get(lista_estados[estado], 0);

    list_remove_and_destroy_element(lista_estados[estado], 0, (void*)free);

    sem_post(&sem_lista_estados[estado]);
    // pthread_mutex_unlock(mutex_lista_estados[estado_]);

    return pcb;
}


void _planificador_corto_plazo() {

    // Desalojo de PCBs
    pthread_t manejo_desalojo;
    pthread_create(&manejo_desalojo, NULL, (void*)manejo_desalojo_pcb, NULL);
    pthread_detach(manejo_desalojo);

    //Dispatcher
    while(1) {

    	sem_wait(&sem_cpu_disponible);
        sem_wait(&sem_proceso_a_ready_terminado);
        PCB* pcbParaEjecutar;

        log_info(kernelLogger, "Cola Ready %s: %s", kernelConfig->ALGORITMO_PLANIFICACION, pids_on_list(ENUM_READY));

        if(string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO")) {
        	pcbParaEjecutar = elegir_pcb_segun_fifo();

        }
        else if (string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")) {
        	pcbParaEjecutar = elegir_pcb_segun_hrrn();
        }

        agregar_a_lista_con_sem((void*)pcbParaEjecutar, ENUM_EXECUTING);


        log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcbParaEjecutar->id_proceso, nombres_estados[ENUM_READY], nombres_estados[ENUM_EXECUTING]);

        pthread_mutex_unlock(&sem_proceso_a_executing);

    }
}

void manejo_desalojo_pcb() {
    while(1) {

        pthread_mutex_lock(&sem_proceso_a_executing);

        sem_wait(&sem_lista_estados[ENUM_EXECUTING]);

        PCB* pcb_para_cpu = list_remove(lista_estados[ENUM_EXECUTING], 0);

        sem_post(&sem_lista_estados[ENUM_EXECUTING]);

        double inicio_ejecucion_proceso = time(NULL);

        if (conexionCPU > 0) {
        	enviar_pcb(conexionCPU, pcb_para_cpu, OP_EXECUTE_PCB, kernelLogger);
        } else {
        	log_error(kernelLogger, "ERROR CONEXION CPU NO FUE HECHA BIEN, NO SE PUEDE MANDAR EL PCB");
        	mostrar_pcb(pcb_para_cpu, kernelLogger);
        	liberar_recursos_kernel();
        	exit(EXIT_FAILURE);
        }

        codigo_operacion operacionRecibida = recibir_operacion(conexionCPU);

        //log_debug(kernelLogger, "CODIGO DE OPERACION RECIBIDO: %d", operacionRecibida);

        PCB* pcb_recibido = recibir_proceso_desajolado(pcb_para_cpu);
        free(pcb_para_cpu);

        double fin_ejecucion_proceso = time(NULL);

        double tiempo_en_cpu = fin_ejecucion_proceso - inicio_ejecucion_proceso;

        pcb_estimar_proxima_rafaga(pcb_recibido, tiempo_en_cpu);

        char* ultimaInstruccion;
        char** ultimaInstruccionDecodificada;
        ultimaInstruccion = string_duplicate(list_get(pcb_recibido->lista_instrucciones, pcb_recibido->contador_instrucciones));
        ultimaInstruccionDecodificada = decode_instruccion(ultimaInstruccion, kernelLogger);

        pcb_recibido->contador_instrucciones++;

        t_data_desalojo* data = malloc(sizeof(t_data_desalojo));
        data->instruccion = ultimaInstruccionDecodificada;
        data->operacion = operacionRecibida;
        data->pcb = pcb_recibido;

        codigo_operacion res = manejo_instrucciones(data);

        if(res==I_CREATE_SEGMENT){
            data->operacion = I_CREATE_SEGMENT;
            res = manejo_instrucciones(data);
        }

         free(ultimaInstruccion);
         free(ultimaInstruccionDecodificada);
         free(data);
        }
    return;
}

codigo_operacion manejo_instrucciones(t_data_desalojo* data){
	codigo_operacion res;
	const codigo_operacion operacionNoSobreEscribir = data->operacion;
    PCB* pcb = data->pcb;
    char** instruccion = data->instruccion;

        switch(operacionNoSobreEscribir) {
            case I_YIELD: {
               	agregar_a_lista_con_sem((void*)pcb, ENUM_READY);
               	pcb->ready_timestamp = time(NULL);
               	sem_post(&sem_cpu_disponible);
               	sem_post(&sem_proceso_a_ready_terminado);
               	break;
            }
            case I_F_OPEN: {
                 char* nombreArchivo = instruccion[1];
                strtok(nombreArchivo, "\n");

                bool existeSemaforo = dictionary_has_key(tablaArchivosAbiertos, nombreArchivo);

                if (existeSemaforo) {
                    log_debug(kernelLogger, ABRIR_ARCHIVO_BLOQUEADO, pcb->id_proceso, nombreArchivo);
                    agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
                 	t_semaforo_recurso* semaforoArchivo = (t_semaforo_recurso*) dictionary_get(tablaArchivosAbiertos, nombreArchivo);
                	t_estado* estado = semaforoArchivo == NULL ? NULL : semaforoArchivo->estadoRecurso;

                    pthread_mutex_lock(estado->mutexEstado);
                    list_add(estado->listaProcesos, pcb);
                    semaforoArchivo->instancias--;
                    sem_wait(estado->semaforoEstado);
                    pthread_mutex_unlock(estado->mutexEstado);

                    log_info(kernelLogger, ABRIR_ARCHIVO, pcb->id_proceso, nombreArchivo);
                    sem_post(&sem_proceso_a_ready_terminado);
                    break;
                }

                size_t tamanioPalabra = strlen(nombreArchivo);

                enviar_operacion(conexionFileSystem, operacionNoSobreEscribir, tamanioPalabra, nombreArchivo);
                codigo_operacion operacionDelFileSystem = recibir_operacion(conexionFileSystem);
                recibir_operacion(conexionFileSystem); // 0 basura

                if (operacionDelFileSystem != AUX_OK) {
                    // Si no existe lo creo
                    enviar_operacion(conexionFileSystem, KERNEL_CREAR_ARCHIVO, tamanioPalabra, nombreArchivo);
                    recibir_operacion(conexionFileSystem);
                    recibir_operacion(conexionFileSystem); // Por 0 basura
                }

                // abrir archivo globalmente
                t_semaforo_recurso* semaforoArchivo = malloc(sizeof(t_semaforo_recurso));
                semaforoArchivo->instancias = 0;
                semaforoArchivo->estadoRecurso = crear_archivo_estado(ENUM_ARCHIVO_BLOCK);
                dictionary_put(tablaArchivosAbiertos, nombreArchivo, semaforoArchivo);

                t_archivo_abierto* archivoAbierto = malloc(sizeof(t_archivo_abierto));
                archivoAbierto->nombreArchivo = nombreArchivo;
                archivoAbierto->puntero = 0;

                // abrir archivo proceso
                list_add(pcb->lista_archivos_abiertos, archivoAbierto);

                agregar_a_lista_con_sem((void*)pcb, ENUM_EXECUTING);

                log_info(kernelLogger, ABRIR_ARCHIVO, pcb->id_proceso, nombreArchivo);
                pthread_mutex_unlock(&sem_proceso_a_executing);
                sem_post(&sem_cpu_disponible);
                break;
            }

            case I_F_CLOSE: {
                char* nombreArchivo = instruccion[1];
                strtok(nombreArchivo, "\n");
                list_remove_element(pcb->lista_archivos_abiertos, nombreArchivo);

                t_semaforo_recurso* semaforoArchivo = (t_semaforo_recurso*) dictionary_get(tablaArchivosAbiertos, nombreArchivo);
                t_estado* estado = semaforoArchivo->estadoRecurso;

                bool debeDesbloquearAlgunProceso = !list_is_empty(estado->listaProcesos);
                if (debeDesbloquearAlgunProceso) {
                    pthread_mutex_lock(estado->mutexEstado);

                    PCB* pcb = list_remove(estado->listaProcesos, 0);
                    log_info(kernelLogger, CERRAR_ARCHIVO_DESBLOQUEA_PCB,
                    		pcb->id_proceso, nombreArchivo, pcb->id_proceso);

                    cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);
                    sem_post(estado->semaforoEstado);
                    pthread_mutex_unlock(estado->mutexEstado);
                } else {
                    // Ya no quedan procesos que usen el archivo
                    dictionary_remove(tablaArchivosAbiertos, nombreArchivo);
                }
                agregar_a_lista_con_sem(pcb, ENUM_EXECUTING);

                log_info(kernelLogger, CERRAR_ARCHIVO, pcb->id_proceso, nombreArchivo);

                pthread_mutex_unlock(&sem_proceso_a_executing);
                sem_post(&sem_cpu_disponible);
                break;
            }

            case I_TRUNCATE: {
            	agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
            	PCB* pcbBlocked = list_get(lista_estados[ENUM_BLOCKED], 0);
            	sem_post(&sem_cpu_disponible);
                t_paquete* paquete = crear_paquete(operacionNoSobreEscribir);
                agregar_a_paquete(paquete, (void*)instruccion[1], strlen(instruccion[1]));
                agregar_a_paquete(paquete, (void*)instruccion[2], strlen(instruccion[2]));

                enviar_paquete(paquete, conexionFileSystem);
                log_info(kernelLogger, "ENVIO TRUNCATE de archivo: %s, tamanio: %s", instruccion[1], instruccion[2]);
                eliminar_paquete(paquete);
                codigo_operacion cod1 = recibir_operacion(conexionFileSystem);
                log_info(kernelLogger, "Exitoso TRUNCATE de archivo: %s, tamanio: %s", instruccion[1], instruccion[2]);

                recibir_operacion(conexionFileSystem); // basura

                cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);
                PCB* pcbReady = list_get(lista_estados[ENUM_READY], 0);

                sem_post(&sem_proceso_a_ready_terminado);
                break;
            }
            case I_F_SEEK: {
                char* nombreArchivo = instruccion[1];
                char *endptr;
                uint32_t puntero = strtoul(instruccion[2], &endptr, 10);
                t_archivo_abierto* archivoAbierto = encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);
                archivoAbierto->puntero = puntero;
                log_debug(kernelLogger, F_SEEK_HECHO, pcb->id_proceso, nombreArchivo, puntero);
                agregar_a_lista_con_sem((void*)pcb, ENUM_EXECUTING);
                pthread_mutex_unlock(&sem_proceso_a_executing);
                sem_post(&sem_cpu_disponible);
                break;
            }
            case I_F_READ: {
            	codigo_operacion cod_op = recibir_operacion(conexionCPU); // basura

                char* buffer;
                int tamanio = 0;
                int desplazamiento = 0;
                buffer = recibir_buffer(&tamanio, conexionCPU);

                int direccionFisica = leer_int(buffer, &desplazamiento);
				agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
				enviar_f_read_write(pcb, instruccion, operacionNoSobreEscribir, direccionFisica);
				mover_de_lista_con_sem(pcb->id_proceso, ENUM_READY, ENUM_BLOCKED);
				sem_post(&sem_proceso_a_ready_terminado);
                sem_post(&sem_cpu_disponible);
                break;
            }
            case I_F_WRITE: {
                codigo_operacion cod_op = recibir_operacion(conexionCPU); // basura
                char* buffer;
                int tamanio = 0;
                int desplazamiento = 0;
                buffer = recibir_buffer(&tamanio, conexionCPU);

                int direccionFisica = leer_int(buffer, &desplazamiento);
            	agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
            	enviar_f_read_write(pcb, instruccion, operacionNoSobreEscribir, direccionFisica);
            	mover_de_lista_con_sem(pcb->id_proceso, ENUM_READY, ENUM_BLOCKED);
            	sem_post(&sem_proceso_a_ready_terminado);
                sem_post(&sem_cpu_disponible);
                break;
            }
        	case I_EXIT: {
        	    terminar_proceso(pcb, EXIT__SUCCESS);
        		break;
        	}
        	case SEGMENTATION_FAULT:{
        		terminar_proceso(pcb, EXIT_SEGMENTATION_FAULT);
        		break;
        	}
        	case I_IO: {
        	    char* tiempo_de_io_string = instruccion[1];
        		int tiempo_de_io = atoi(tiempo_de_io_string);
        		log_info(kernelLogger, LOG_CAMBIO_DE_ESTADO, pcb->id_proceso, EXECUTING, BLOCKED);
        		agregar_a_lista_con_sem(pcb, ENUM_BLOCKED);
        		log_info(kernelLogger, "PID: %d - Bloqueado por: IO", pcb->id_proceso);
        		log_info(kernelLogger, "PID: %d - Ejecuta IO: %d", pcb->id_proceso, tiempo_de_io);

        		intervalo_de_pausa(tiempo_de_io*1000);

        		sem_wait(&sem_lista_estados[ENUM_BLOCKED]);
        		list_remove(lista_estados[ENUM_BLOCKED], 0);
        		sem_post(&sem_lista_estados[ENUM_BLOCKED]);
                agregar_a_lista_con_sem(pcb, ENUM_READY);
        		pcb->ready_timestamp = time(NULL);
        		log_info(kernelLogger, LOG_CAMBIO_DE_ESTADO, pcb->id_proceso, nombres_estados[ENUM_BLOCKED], nombres_estados[ENUM_READY]);
        		sem_post(&sem_cpu_disponible);
        		sem_post(&sem_proceso_a_ready_terminado);
        		break;
        	}
        	case I_WAIT:{
        	    char* nombre_recurso = instruccion[1];
        		instruccion_wait(pcb, nombre_recurso);
        		free(nombre_recurso);
        		break;
        	}
        	case I_SIGNAL:{
        	    char* nombre_recurso = instruccion[1];
        		instruccion_signal(pcb, nombre_recurso);
        		free(nombre_recurso);
        		break;
        	}
			 case I_CREATE_SEGMENT: { //id_segmento, tamanio
				 pthread_mutex_lock(&mutex_memoria);
				 int id_segmento = atoi(instruccion[1]);
				 int tamanio_segmento = atoi(instruccion[2]);

                 enviar_pcb(conexionMemoria, pcb, I_CREATE_SEGMENT, kernelLogger);
				 t_paquete* paquete = crear_paquete(I_CREATE_SEGMENT);

                // enviar_pcb(conexionCPU, pcb, AUX_OK, kernelLogger);
				agregar_int_a_paquete(paquete, id_segmento);
				agregar_int_a_paquete(paquete, tamanio_segmento);
				enviar_paquete(paquete, conexionMemoria);
				eliminar_paquete(paquete);

                codigo_operacion codigoRespuesta = recibir_operacion(conexionMemoria);

                if (codigoRespuesta == AUX_OK) {
                    char* buffer;
                    int tamanioBuffer = 0;
                    int desplazamiento = 0;

                    t_segmento *segmento = (t_segmento*)list_get(pcb->lista_segmentos, id_segmento);
                    segmento->id = id_segmento;
                    segmento->size = tamanio_segmento;

                    buffer = recibir_buffer(&tamanioBuffer, conexionMemoria);
                    segmento->direccionBase = leer_puntero(buffer, &desplazamiento);

                    list_replace(pcb->lista_segmentos, id_segmento, (void*)segmento);
                    t_segmento* segmentoReemplazado = (t_segmento*)list_get(pcb->lista_segmentos, id_segmento);
                    free(buffer);
                    log_info(kernelLogger, "PID: %d - Crear Segmento - Id: %d - Tamaño: %d Realizado", pcb->id_proceso, segmento->id, segmento->size);
                    agregar_a_lista_con_sem((void*)pcb, ENUM_EXECUTING);
                    pthread_mutex_unlock(&mutex_memoria);
                    pthread_mutex_unlock(&sem_proceso_a_executing);
                    return AUX_OK;

                }else if(codigoRespuesta == COMPACTACION){
                    log_info(kernelLogger, "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>");
                    pthread_mutex_lock(&permiso_compactacion);
                    log_info(kernelLogger, "Inicia la compactacion");

                    enviar_operacion(conexionMemoria, COMPACTACION, 0,0);
                    recibir_operacion(conexionMemoria);
                    recibir_operacion(conexionMemoria);


                    t_list *tablas_de_segmentos_actualizadas = recibir_todas_las_tablas_segmentos(conexionMemoria);
                    actualizar_todas_las_tablas_de_segmentos(tablas_de_segmentos_actualizadas);

                    log_info(kernelLogger, "Se finalizó el proceso de compactación");
                    pthread_mutex_unlock(&permiso_compactacion);
                    pthread_mutex_unlock(&mutex_memoria);
                    pthread_mutex_unlock(&sem_proceso_a_executing);
                    return I_CREATE_SEGMENT;
                    break;
                }else if(codigoRespuesta == OUT_OF_MEMORY){
                	pthread_mutex_unlock(&mutex_memoria);
                	terminar_proceso(pcb, OUT_OF_MEMORY);
                	return OUT_OF_MEMORY;
                	break;
                }

				break;
			 }
			 case I_DELETE_SEGMENT: {

				 pthread_mutex_lock(&mutex_memoria);
				 int id_segmento = atoi(instruccion[1]);

				 enviar_pcb(conexionMemoria, pcb, I_DELETE_SEGMENT, kernelLogger);
				 t_paquete* paquete = crear_paquete(I_DELETE_SEGMENT);

				 // enviar_pcb(conexionCPU, pcb, AUX_OK, kernelLogger);
				 agregar_int_a_paquete(paquete, id_segmento);
				 enviar_paquete(paquete, conexionMemoria);
				 eliminar_paquete(paquete);

				 codigo_operacion codigoRespuesa = recibir_operacion(conexionMemoria);

				 if (codigoRespuesa == AUX_OK) {
					 //t_list *tabla_segmentos_actualizada = recibir_tabla_segmentos(conexionMemoria);

					 //cambiar la tabla de segmentos del proceso por la nueva
					 int size;
					 void* buffer = recibir_buffer(&size, conexionMemoria);
					 int desplazamiento = 0;
					 t_list* tabla_segmentos_actualizada = deserializar_tabla_segmentos(buffer, &desplazamiento);
					 free(buffer);

					 log_debug(kernelLogger, "Lista de segmentos antes de eliminarse: ");
					 mostrarListaSegmentos(pcb->lista_segmentos);
					 //pcb->lista_segmentos = tabla_segmentos_actualizada;
					 log_debug(kernelLogger, "Lista de segmentos despues de eliminarse: ");
					 pcb->lista_segmentos = tabla_segmentos_actualizada;
					 mostrarListaSegmentos(pcb->lista_segmentos);
					 pthread_mutex_unlock(&mutex_memoria);

					 log_info(kernelLogger, "PID: <%d> - Eliminar Segmento - Id Segmento: <%d>", pcb->id_proceso, id_segmento);
				 }

				 agregar_a_lista_con_sem(pcb, ENUM_EXECUTING);
				 pthread_mutex_unlock(&sem_proceso_a_executing);

				break;
			 }
			 default: {
				 break;
			 }
        }
        return operacionNoSobreEscribir;
}

t_segmento* buscar_segmento(t_list* listaSegmentos, int id_segmento) {
	for (int indice = 0; indice < list_size(listaSegmentos); indice++) {
		t_segmento* segmento = list_get(listaSegmentos, id_segmento);
		if (segmento->id == id_segmento) {
			return segmento;
		}
	}
	log_warning(kernelLogger, "Kernel no encontró el segmento %d", id_segmento);
	return NULL;
}

t_list* recibir_tabla_segmentos(int socket_cliente){
    int size;
    void* buffer = recibir_buffer(&size, socket_cliente);

    int desplazamiento = 0;

    t_list* tabla_segmentos = deserializar_tabla_segmentos(buffer, &desplazamiento);

    free(buffer);

    return tabla_segmentos;
}

t_list* recibir_todas_las_tablas_segmentos(int socket_cliente){
	int size;
	void* buffer = recibir_buffer(&size, socket_cliente);

	int desplazamiento = 0;

	t_list* tablas_segmentos = deserealizar_todas_las_tablas_segmentos(buffer, &desplazamiento);

	free(buffer);

	return tablas_segmentos;
}

void actualizar_todas_las_tablas_de_segmentos(t_list* nuevas_tablas){
    for (int i = 0; i < nuevas_tablas->elements_count; i++){
        t_tabla_segmentos* tabla_actualizada = list_get(nuevas_tablas, i);
        PCB* proceso = buscar_proceso(tabla_actualizada->PID);
        if (proceso != NULL){
            list_clean_and_destroy_elements(proceso->lista_segmentos, free);
            proceso->lista_segmentos = tabla_actualizada->segmentos;
        }
    }
}

PCB *buscar_proceso(int idProceso){

    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        t_list* listaPorEstado = lista_estados[estado];
        if (listaPorEstado) {
			for (int index = 0; index < list_size(listaPorEstado); index++) {
				PCB* pcb = list_get(listaPorEstado, index);
				if (pcb->id_proceso == idProceso) {
					log_trace(kernelLogger, "PCB con id %d Lista encontrada en lista de estados: %s", pcb->id_proceso, nombres_estados[estado]);
					return pcb;
				}
			}
        }
    }
    return NULL;
    /*
    int resulatdo_buesqueda; // en -1 entoces no lo encontro
    if (EJECUTANDO->contexto->PID == pid_buscado)
        return EJECUTANDO;

    int indice;
    int estado;
    for (int i = 0; i < CANTIDAD_ESTADOS; i++){
    	indice = obtener_index_pcb_de_lista(i, pcb->id_proceso);
    	estado = i;
    }

    PCB* pcb_buscado = (PCB*)list_get(lista_estados[estado], indice);

    //TODO
    //log_info(kernelLogger, "No hay proceso con PID: <%d> al que se le pueda actualizar la tabla de segmetnos", pid_buscado);
*/
}

/*
void crear_segmento(PCB* pcb_recibido, char* id_segmento, char* tamanio) {
	char** parametros_a_memoria = string_array_new();
	string_array_push(&parametros_a_memoria, id_segmento);
	string_array_push(&parametros_a_memoria, tamanio);
	string_array_push(&parametros_a_memoria, string_itoa(pcb_recibido->id_proceso));
	enviar_msj_con_parametros(conexionMemoria, I_CREATE_SEGMENT, parametros_a_memoria);

	codigo_operacion respuesta = recibir_operacion(conexionMemoria);
	switch(respuesta){
		case SEGMENTO_ CREADO: {
			log_info(kernelLogger, CREAR_SEGMENTO, pcb_recibido->id_proceso, atoi(id_segmento), atoi(tamanio));
			pcb_recibido->lista_segmentos = recibir_lista_segmentos(conexionMemoria);  //TODO: para inicializar pcb, y cada vez que se modifique hay que recibir la lista actualizada
			break;
		}
		case OUT_OF_ MEMORY: {
			terminar_proceso(pcb_recibido, OUT_OF_MEMORY);
			break;
		}
		case COMPACTA CION: {
			log_info(kernelLogger, "Compactación: Esperando Fin de Operaciones de FS");
			sem_wait(&sem_compactacion);
			log_info(kernelLogger, "Compactación: Se solicitó compactación.");
			enviar_codigo_operacion(conexionMemoria, COMPACTACION);

			recibir_operacion(conexionMemoria); // es al pedo, pero necesita recibirlo si o si
			t_list* lista_procesos_con_segmentos = recibir_procesos_con_segmentos(conexionMemoria); //TODO lista de t_proceso_actualizado con lista de segmentos cada uno
			actualizar_segmentos(lista_procesos_con_segmentos, pcb_recibido); //TODO
			log_info(kernelLogger, "Se finalizó el proceso de compactación.");
			sem_post(&sem_compactacion);
			crear_segmento(pcb_recibido, id_segmento, tamanio);
			break;
		}
		default:
			break;
	}
	string_array_destroy(parametros_a_memoria);
}
*/
/* lo tenemos implementado distinto, por eso no funciona
t_list* recibir_tabla_segmentos(int socket) {
	size_t size_payload;
    if (recv(socket, &size_payload, sizeof(size_t), 0) != sizeof(size_t)) {
        exit(EXIT_FAILURE);
    }

	void* stream_a_recibir = malloc(size_payload);
    if (recv(socket, stream_a_recibir, size_payload, 0) != size_payload) {
        free(stream_a_recibir);
        exit(EXIT_FAILURE);
    }

    t_list* tabla_segmentos = deserializar_tabla_segmentos(stream_a_recibir, size_payload);

	free(stream_a_recibir);
	return tabla_segmentos;
}
t_list* deserializar_tabla_segmentos(void* stream, int size_payload) {
	size_t desplazamiento = 0;

	t_list* tabla_segmentos = list_create();

	t_segmento* segmento;

	while(desplazamiento < size_payload) {
		segmento = malloc(sizeof(t_segmento));

		memcpy(&(segmento->id), stream + desplazamiento, sizeof(segmento->id));
		desplazamiento += sizeof(segmento->id);

		memcpy(&(segmento->direccionBase), stream + desplazamiento, sizeof(segmento->direccionBase));
		desplazamiento += sizeof(segmento->direccionBase);

		memcpy(&(segmento->tamanio), stream + desplazamiento, sizeof(segmento->size));
		desplazamiento += sizeof(segmento->size);

		list_add(tabla_segmentos, segmento);
	}

	return tabla_segmentos;
}
*/
void instruccion_wait(PCB *pcb_en_ejecucion, char *nombre_recurso){

	if (!dictionary_has_key(diccionario_recursos, nombre_recurso)) {
        log_info(kernelLogger, "ERROR - PID: %d - %s NO existe", pcb_en_ejecucion->id_proceso, nombre_recurso);
        terminar_proceso(pcb_en_ejecucion, WAIT_RECURSO_NO_EXISTENTE); //TODO: codigo de inexistencia de recurso
    }
    else{
        t_recurso *recurso = dictionary_get(diccionario_recursos, nombre_recurso);
        recurso->instancias--;
        log_info(kernelLogger, "PID: %d - Wait: %s - Instancias: %d", pcb_en_ejecucion->id_proceso, nombre_recurso, recurso->instancias);
        if (recurso->instancias < 0) { // Chequea si debe bloquear al proceso por falta de instancias

        	sem_t semaforo_recurso = recurso->sem_recurso;
        	sem_wait(&semaforo_recurso);

        	list_add(recurso->procesos_bloqueados, pcb_en_ejecucion);
        	recurso->instancias = 0;
        	sem_post(&semaforo_recurso);

            log_info(kernelLogger, "PID: %u - Bloqueado por: %s", pcb_en_ejecucion->id_proceso, nombre_recurso);

            sem_post(&sem_cpu_disponible);

        } else { // Si el proceso no se bloquea en el if anterior, puede usar el recurso
        	agregar_a_lista_con_sem((void*)pcb_en_ejecucion, ENUM_EXECUTING);
        	pthread_mutex_unlock(&sem_proceso_a_executing);
        }
    }
    return;
}

void instruccion_signal(PCB *pcb_en_ejecucion, char *nombre_recurso){

	if (!dictionary_has_key(diccionario_recursos, nombre_recurso)) {
	    log_info(kernelLogger, "ERROR - PID: %u - %s NO existe", pcb_en_ejecucion->id_proceso, nombre_recurso);
	    terminar_proceso(pcb_en_ejecucion, SIGNAL_RECURSO_NO_EXISTENTE); //TODO: codigo de inexistencia de recurso
	}
	else{
		t_recurso *recurso = dictionary_get(diccionario_recursos, nombre_recurso);
		recurso->instancias++;
		agregar_a_lista_con_sem((void*)pcb_en_ejecucion, ENUM_EXECUTING);
		pthread_mutex_unlock(&sem_proceso_a_executing);

		log_info(kernelLogger, "PID: %d - Signal: %s - Instancias: %d", pcb_en_ejecucion->id_proceso, nombre_recurso, recurso->instancias);
		//if(recurso->instancias == 0){
		if(list_size(recurso->procesos_bloqueados) > 0){
		    // Desbloquea al primer proceso de la cola de bloqueados del recurso
			sem_t semaforo_recurso = recurso->sem_recurso;

			sem_wait(&semaforo_recurso);

			PCB* pcb = list_remove(recurso->procesos_bloqueados, 0);
			log_warning(kernelLogger, "removiendo PID %d de la lista de bloqueados de: %s", pcb->id_proceso, recurso->nombre);
			sem_post(&semaforo_recurso);

			agregar_a_lista_con_sem(pcb, ENUM_READY);

			sem_post(&sem_proceso_a_ready_terminado);

		}
	}
	return;
}

void enviar_f_read_write(PCB* pcb, char** instruccion, codigo_operacion codigoOperacion, int direccionFisica) {
    pthread_mutex_lock(&permiso_compactacion);
    t_paquete* paquete = crear_paquete(codigoOperacion);

    // 1: Nombre Archivo, 2: Dirección Fisica, 3: Cantidad de bytes
    char* nombreArchivo = instruccion[1];
    strtok(nombreArchivo, "\n");
    char* cantidadBytes = instruccion[3];
    strtok(cantidadBytes, "\n");

    t_archivo_abierto* archivoAbierto = encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);

    agregar_a_paquete(paquete, (void*)nombreArchivo, strlen(nombreArchivo));
    agregar_int_a_paquete(paquete, direccionFisica);
    agregar_a_paquete(paquete, (void*)cantidadBytes, strlen(cantidadBytes));
    agregar_int_a_paquete(paquete, pcb->id_proceso);
    agregar_uint32_a_paquete(paquete, archivoAbierto->puntero);
    enviar_paquete(paquete, conexionFileSystem);
    eliminar_paquete(paquete);

    codigo_operacion codOp = recibir_operacion(conexionFileSystem);
    codigo_operacion codigoRespuesta = recibir_operacion(conexionFileSystem);

    pthread_mutex_unlock(&permiso_compactacion);
}

t_estado* crear_archivo_estado(t_nombre_estado nombreEstado) {
    t_estado* estado = malloc(sizeof(t_estado));

    // Creo y seteo el nombre estado y la lista para guardar procesos
    estado->nombreEstado = nombreEstado;
    estado->listaProcesos = list_create();

    // Creo y seteo el semaforo estado
    sem_t *semaforoEstado = malloc(sizeof(*semaforoEstado));
    sem_init(semaforoEstado, 0, 0);
    estado->semaforoEstado = semaforoEstado;

    // Creo y seteo el mutex del estado
    pthread_mutex_t *mutexEstado = malloc(sizeof(*(mutexEstado)));
    pthread_mutex_init(mutexEstado, NULL);
    estado->mutexEstado = mutexEstado;

    return estado;
}


int list_get_index(t_list *list, bool (*cutting_condition)(void *temp, void *target), void* target)
{
    // Linear search algorithm to find an item with a given condition
    for (int i = 0; i < list_size(list); i++) {
        void *temp = list_get(list, i);

        if (cutting_condition(temp, target)) {
            return i;
        }
    }

    return -1;
}

t_archivo_abierto* encontrar_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo) {
    int cantidadArchivos = list_size(listaArchivosAbiertos);

    for (int i = 0; i < cantidadArchivos; i++) {
        t_archivo_abierto* archivoAbierto = list_get(listaArchivosAbiertos, i);
        if (strcmp(archivoAbierto->nombreArchivo, nombreArchivo) == 0) {
            return archivoAbierto;
        }
    }
    log_warning(kernelLogger, "No se encontró el archivo %s en la lista de archivos abiertos.", nombreArchivo);
    return NULL;

}

int encontrar_index_archivo_abierto(t_list* listaArchivosAbiertos, char* nombreArchivo) {
    int cantidadArchivos = list_size(listaArchivosAbiertos);

    for (int i = 0; i < cantidadArchivos; i++) {
        t_archivo_abierto* archivoAbierto = list_get(listaArchivosAbiertos, i);
        if (archivoAbierto->nombreArchivo == nombreArchivo) {
            return i;
        }
    }

    return -1;

}


/*
t_dictionary* crear_diccionario_semaforos_recursos(char **recursos, char **instanciasRecursos)
{
    t_dictionary *diccionarioSemaforosRecursos = dictionary_create();

    for (int i = 0; recursos[i] != NULL && instanciasRecursos[i] != NULL; i++) {
        int32_t instancias = (uint32_t) atoi(instanciasRecursos[i]);
        t_semaforo_recurso *semaforoRecurso = __crear_semaforo_recurso(instancias);

        char *nombreRecurso = recursos[i];
        dictionary_put(diccionarioSemaforosRecursos, nombreRecurso, (void *) semaforoRecurso);
    }

    return diccionarioSemaforosRecursos;
}
*/

double obtener_diferencial_de_tiempo_en_milisegundos(timestamp *end, timestamp *start) {
    const uint32_t SECS_TO_MILISECS = 1000;
    const uint32_t NANOSECS_TO_MILISECS = 1000000;
    return (double) ( (end->tv_sec - start->tv_sec) * SECS_TO_MILISECS + (end->tv_nsec - start->tv_nsec) / NANOSECS_TO_MILISECS );
}

void pcb_estimar_proxima_rafaga(PCB *pcb_ejecutado, double tiempo_en_cpu){

	double alfa_hrrn = kernelConfig->HRRN_ALFA;

    double estimadoProxRafagaPcb = pcb_ejecutado->estimacion_rafaga;
    double estimadoProxRafagaActualizado = alfa_hrrn * tiempo_en_cpu + (1.0 - alfa_hrrn) * estimadoProxRafagaPcb;

    pcb_ejecutado->estimacion_rafaga = estimadoProxRafagaActualizado;

    return;
}

PCB* recibir_proceso_desajolado(PCB* pcb_en_ejecucion) {

    PCB* pcb_recibido = recibir_pcb(conexionCPU);

    int id_proceso_en_ejecucion = pcb_en_ejecucion->id_proceso;
    int id_pcb_recibido = pcb_recibido->id_proceso;

    if(id_proceso_en_ejecucion != id_pcb_recibido) {
        log_error(kernelLogger, "El PID: %d del proceso desalojado no coincide con el proceso en ejecución con PID: %d", id_proceso_en_ejecucion, id_pcb_recibido);
        exit(EXIT_FAILURE);
    }

    // pcb_recibido->lista_archivos_abiertos = pcb_en_ejecucion->lista_archivos_abiertos;
    if (list_size(pcb_en_ejecucion->lista_archivos_abiertos) > 0) {
    	list_add_all(pcb_recibido->lista_archivos_abiertos, pcb_en_ejecucion->lista_archivos_abiertos); // rompe
    }

    return pcb_recibido;
}

void set_timespec(timestamp *timespec)
{
    int retVal = clock_gettime(CLOCK_REALTIME, timespec);

    if (retVal == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
}

PCB* elegir_pcb_segun_fifo(){
    PCB* pcb;

    sem_wait(&sem_lista_estados[ENUM_READY]);

    pcb = list_remove(lista_estados[ENUM_READY], 0);

    sem_post(&sem_lista_estados[ENUM_READY]);

    return pcb;

}

PCB* elegir_pcb_segun_hrrn() {

    sem_wait(&sem_lista_estados[ENUM_READY]);

    PCB* pcb = obtener_maximo_por_R(lista_estados[ENUM_READY]);

    sem_post(&sem_lista_estados[ENUM_READY]);

    return pcb;
}

PCB* obtener_maximo_por_R(t_list* lista_procesos){

	PCB* pcb_maximo = list_get(lista_procesos, 0);
	double tiempo_actual = time(NULL);

	for(int i = 1; i < list_size(lista_procesos); i++){
		PCB* pcb_aux = list_get(lista_procesos, i);
		if(__calcular_valor_hrrn(pcb_aux, tiempo_actual) > __calcular_valor_hrrn(pcb_maximo, tiempo_actual)){
			pcb_maximo = pcb_aux;
		}
	}

	int indice = obtener_index_pcb_de_lista(ENUM_READY, pcb_maximo->id_proceso);

	list_remove(lista_estados[ENUM_READY], indice);

	return pcb_maximo;
}

double __calcular_valor_hrrn(PCB *pcb, double tiempoActual){

    double tiempoEnReady = tiempoActual - pcb->ready_timestamp;

    return (pcb->estimacion_rafaga + tiempoEnReady) / pcb->estimacion_rafaga;
}

void terminar_proceso(PCB* pcb_para_finalizar, codigo_operacion motivo_finalizacion){

	log_info(kernelLogger, "Finaliza el proceso con PID %d - Motivo: %s", pcb_para_finalizar->id_proceso, obtener_motivo(motivo_finalizacion));

	destruir_pcb(pcb_para_finalizar);

	sem_post(&sem_cpu_disponible);
	sem_post(&sem_grado_multiprogamacion);

}

char* obtener_motivo(codigo_operacion codigo_motivo){

	switch(codigo_motivo){
		case EXIT__SUCCESS:{
			return "EXIT_SUCCESS";
			break;
		}
		case WAIT_RECURSO_NO_EXISTENTE:{
			return "WAIT_RECURSO_NO_EXISTENTE";
			break;
		}
		case SIGNAL_RECURSO_NO_EXISTENTE:{
			return "SIGNAL_RECURSO_NO_EXISTENTE";
			break;
		}
		case EXIT_SEGMENTATION_FAULT:{
			return "EXIT_SEGMENTATION_FAULT";
			break;
		}
		case OUT_OF_MEMORY:{
			return "OUT_OF_MEMORY";
			break;
		}
		default:
			break;
	}
	return NULL;
}

/*
 * Ingresa nuevo proceso, por lo tanto se crea un pcb que contiene información del mismo
 *  además se agrega un proceso a lista de new
 *  Devuelve el pcb creado
 *  @param t_list* listaInstrucciones Instrucciones que el proceso debe ejecutar
 *  @param int clienteAceptado cliente que pide crear un proceso nuevo
 *  return PCB*
 */
PCB* nuevo_proceso(t_list* listaInstrucciones, int clienteAceptado) {
    PCB* pcb = malloc(sizeof(PCB));

    pcb->id_proceso = contadorProcesoId;
    contadorProcesoId++;

    pcb->estado = ENUM_NEW;
    pcb->lista_instrucciones = list_create();
    list_add_all(pcb->lista_instrucciones, listaInstrucciones);
    pcb->contador_instrucciones = 0;

    pcb->registrosCpu = malloc(sizeof(registros_cpu));
    strcpy(pcb->registrosCpu->AX, "AX");
    strcpy(pcb->registrosCpu->BX, "BX");
    strcpy(pcb->registrosCpu->CX, "CX");
    strcpy(pcb->registrosCpu->DX, "DX");
    strcpy(pcb->registrosCpu->EAX, "EAX");
    strcpy(pcb->registrosCpu->EBX, "EBX");
    strcpy(pcb->registrosCpu->ECX, "ECX");
    strcpy(pcb->registrosCpu->EDX, "EDX");
    strcpy(pcb->registrosCpu->RAX, "RAX");
    strcpy(pcb->registrosCpu->RBX, "RBX");
    strcpy(pcb->registrosCpu->RCX, "RCX");
    strcpy(pcb->registrosCpu->RDX, "RDX");
/*
    strcpy(pcb->registrosCpu->AX, "");
    strcpy(pcb->registrosCpu->BX, "");
    strcpy(pcb->registrosCpu->CX, "");
    strcpy(pcb->registrosCpu->DX, "");
    strcpy(pcb->registrosCpu->EAX, "");
    strcpy(pcb->registrosCpu->EBX, "");
    strcpy(pcb->registrosCpu->ECX, "");
    strcpy(pcb->registrosCpu->EDX, "");
    strcpy(pcb->registrosCpu->RAX, "");
    strcpy(pcb->registrosCpu->RBX, "");
    strcpy(pcb->registrosCpu->RCX, "");
    strcpy(pcb->registrosCpu->RDX, "");
*/
    //pcb->lista_segmentos = list_create();
    pcb->lista_archivos_abiertos = list_create();
    pcb->estimacion_rafaga = kernelConfig->ESTIMACION_INICIAL / 1000;
    pcb->ready_timestamp = 0;

	agregar_a_lista_con_sem(pcb, ENUM_NEW);
	log_info(kernelLogger, "Se crea el proceso %d en NEW", pcb->id_proceso);

    sem_post(&sem_proceso_a_ready_inicializar); // Le envio señal al otro hilo para que cree la estructura y lo mueva a READY cuando pueda

    return pcb;

}


void cambiar_estado_proceso_sin_semaforos(PCB* pcb, pcb_estado estadoNuevo) {
    pcb_estado estadoAnterior = pcb->estado;
    pcb->estado = estadoNuevo;
    list_remove_element(lista_estados[estadoAnterior], pcb);
    list_add(lista_estados[estadoNuevo], pcb);

    char* estadoAntes = nombres_estados[estadoAnterior];
    char* estadoPosterior = nombres_estados[estadoNuevo];
    log_info(kernelLogger,LOG_CAMBIO_DE_ESTADO, pcb->id_proceso, estadoAntes, estadoPosterior);
}

/*
 * Esta funcion mueve un proceso de un estado a otro CON SEMAFOROS, actualizando listas y pcb
 * @param PCB* pcb PCB que sirve para identificar de que proceso se trata
 * @param pcb_estado estado al que se quiera mover el proceso
 * return void
 */
void cambiar_estado_proceso_con_semaforos(PCB* pcb, pcb_estado estadoNuevo) {
    pcb_estado estadoAnterior = pcb->estado;
    mover_de_lista_con_sem(pcb->id_proceso, estadoNuevo, pcb->estado);

    char* estadoAntes = nombres_estados[estadoAnterior];
    char* estadoPosterior = nombres_estados[estadoNuevo];
    log_info(kernelLogger, LOG_CAMBIO_DE_ESTADO, pcb->id_proceso, estadoAntes, estadoPosterior);
}

void inicializar_listas_estados() {

    for (int estado = 0; estado < (CANTIDAD_ESTADOS-1); estado++) {

        lista_estados[estado] = list_create();
        sem_init(&sem_lista_estados[estado], 0, 1);

    }
}

void liberar_listas_estados() {
    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        sem_destroy(&sem_lista_estados[estado]);
    }
}

/*
* Agrega un proceso nuevo a la lista,
* no se me ocurre otro uso que el del estado nuevo
*/
void agregar_a_lista_con_sem(void* elem, int estado) {
	PCB* pcb = (PCB*) elem;
	pcb->estado = estado;
    sem_wait(&sem_lista_estados[estado]);
    list_add(lista_estados[estado], (void*)pcb);
    sem_post(&sem_lista_estados[estado]);
    return;
}

int obtener_index_pcb_de_lista(int estadoPasado, int idProceso) {
	t_list* listaPorEstado = lista_estados[estadoPasado];

    for (int index = 0; index < list_size(listaPorEstado); index++) {
		PCB* pcb = list_get(listaPorEstado, index);
		if (pcb->id_proceso == idProceso) {
			return index;
		}
	}

    log_warning(kernelLogger, "PCB con id %d no encontrado en lista de estados: %s, se busca en las otras", idProceso, nombres_estados[estadoPasado]);

    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        t_list* listaPorEstado = lista_estados[estado];
        for (int index = 0; index < list_size(listaPorEstado); index++) {
            PCB* pcb = list_get(listaPorEstado, index);
            if (pcb->id_proceso == idProceso) {
                log_warning(kernelLogger, "PCB con id %d Lista encontrada en lista de estados: %s", pcb->id_proceso, nombres_estados[estado]);
                return index;
            }
        }
    }
    log_error(kernelLogger, "No se encontró el pcb id <%d> en ninguna lista de estados", idProceso);
	return -1;
}

/*
* Funcion auxiliar de cambiar_estado_proceso_con_semaforos
*/
void mover_de_lista_con_sem(int idProceso, int estadoNuevo, int estadoAnterior) {
	if (estadoNuevo != estadoAnterior) {
		sem_wait(&sem_lista_estados[estadoNuevo]);
		sem_wait(&sem_lista_estados[estadoAnterior]);
		int index = obtener_index_pcb_de_lista(estadoAnterior, idProceso);
        PCB* pcb = (PCB*)list_get(lista_estados[estadoAnterior], index);
        pcb->estado = estadoNuevo;

		if (pcb->estado == ENUM_READY) {
            // set_timespec((timestamp*)(time_t)pcb->ready_timestamp);
		}

		PCB* pcbEliminado = (PCB*)list_remove(lista_estados[estadoAnterior], index);

		if (pcbEliminado->id_proceso != pcb->id_proceso) {
			log_error(kernelLogger, "ERROR AL MOVER pcb de estado %s a estado %s",
					nombres_estados[estadoAnterior], nombres_estados[estadoNuevo] );
		}
        list_add(lista_estados[estadoNuevo], pcbEliminado);

		//int indexDevuelto = list_add(lista_estados[estadoNuevo], (void*)pcb);
		//log_info(kernelLogger, "Se ha añadido el pcb <%d> a la lista de estados <%s> devuelve indice %d", pcb->id_proceso, nombres_estados[estadoNuevo], indexDevuelto);

		sem_post(&sem_lista_estados[estadoAnterior]);
		sem_post(&sem_lista_estados[estadoNuevo]);
		return;
	}else {
	log_warning(kernelLogger, "PCB se intento mover de estado pero ya estaba en estado %s",nombres_estados[estadoAnterior]);
	}
	return;
}

/*------------ ALGORITMO HRRN -----------------*/

double rafaga_estimada(PCB* pcb) {
    // TODO Usar timestamp.h para tomar el tiempo de ingreso y calcularlo para hrrn
    double alfa = kernelConfig->HRRN_ALFA;
    double ultima_rafaga = pcb->estimacion_rafaga;
    double rafaga = ultima_rafaga != 0 ? ((alfa * ultima_rafaga) + ((1 - alfa) * ultima_rafaga)) : kernelConfig->ESTIMACION_INICIAL;

    return rafaga;
}
/*
double calculo_HRRN(PCB* pcb) {
    double rafaga = rafaga_estimada(pcb);
    double res = 1.0 + (pcb->ready_timestamp / rafaga);
    return res;
}

static bool criterio_hrrn(PCB* pcb_A, PCB* pcb_B) {
    double a = calculo_HRRN(pcb_A);
    double b = calculo_HRRN(pcb_B);

    return a > b;
}
*/
void inicializar_diccionario_recursos() {
    tablaArchivosAbiertos = dictionary_create();
    diccionario_recursos = dictionary_create();

    int indice = 0;
    while(kernelConfig->RECURSOS[indice] != NULL && kernelConfig->INSTANCIAS_RECURSOS[indice] != NULL) {
        crear_cola_recursos(kernelConfig->RECURSOS[indice], atoi(kernelConfig->INSTANCIAS_RECURSOS[indice]));

        indice++;
    }
}

void crear_cola_recursos(char* nombre_recurso, int instancias) {

    t_recurso* recurso = malloc(sizeof(t_recurso));

    recurso->nombre = nombre_recurso;
    recurso->instancias = instancias;
    recurso->procesos_bloqueados = list_create();

    sem_t sem;
    sem_init(&sem, instancias, 1);
    recurso->sem_recurso = sem;

    dictionary_put(diccionario_recursos, nombre_recurso, recurso);

}

void inicializar_semaforos() {
    sem_init(&sem_grado_multiprogamacion, 0, kernelConfig->GRADO_MAX_MULTIPROGRAMACION);
    sem_init(&sem_cpu_disponible, 0, 1);
    sem_init(&sem_proceso_a_ready_inicializar, 0, 0);
    sem_init(&sem_proceso_a_ready_terminado, 0, 0);
    pthread_mutex_init(&sem_proceso_a_executing, NULL);
    pthread_mutex_lock(&sem_proceso_a_executing);
    pthread_mutex_init(&permiso_compactacion,NULL);
    sem_init(&proceso_para_finalizar, 0, 0);
    sem_init(&proceso_en_exit, 0, 0);
    sem_init(&sem_compactacion, 0, 1);
    pthread_mutex_init(&mutex_memoria,NULL);
    // pthread_mutex_init(mutexTablaAchivosAbiertos, NULL);
    /* TODO: JOAN Y JOACO
     * LOS CHICOS TENIAN ESTOS TAMBIEN Y ES PROBABLE QUE LOS NECESITEN
     *
        pthread_mutex_init(&mutexSocketMemoria, NULL);
        pthread_mutex_init(&mutexSocketFilesystem, NULL);
        sem_init(&semFRead, 0, 1);
        sem_init(&semFWrite, 0, 1);
        fRead = false;
        fWrite = false;
     */
}

void agregar_a_lista(PCB* pcb, t_list* lista, sem_t m_sem) {
    sem_wait(&m_sem);
    list_add(lista, pcb);
    sem_post(&m_sem);
}

////////////////////////////////////////

char* pids_on_list(pcb_estado estado) {
    char* aux = string_new();
    string_append(&aux,"[");
    int pid_aux;
    for(int i = 0 ; i < list_size(lista_estados[estado]); i++) {
        PCB* pcb = list_get(lista_estados[estado],i);
        pid_aux = pcb->id_proceso;
        string_append(&aux,string_itoa(pid_aux));
        if(i != list_size(lista_estados[estado])-1) string_append(&aux,"|");
    }
    string_append(&aux,"]");
    return aux;
}

void loggear_cola_lista(pcb_estado estado) {
    char* pids_aux = string_new();
    char* algoritmo = kernelConfig->ALGORITMO_PLANIFICACION;
    pids_aux = pids_on_list(estado);
    char* estado2 = nombres_estados[estado];
    log_info(kernelLogger, "Cola Ready %s: %s.",algoritmo, pids_aux);
    free(pids_aux);
}
