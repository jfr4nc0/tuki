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
    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, kernelLogger);
    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    conexionCPU = armar_conexion(config, CPU, kernelLogger);

    cargar_config_kernel(config, kernelLogger);

    log_debug(kernelLogger, "Vamos a usar el algoritmo %s", kernelConfig->ALGORITMO_PLANIFICACION);


    log_info(kernelLogger, "Conexion cpu inicial: %d", conexionCPU);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);

    enviar_codigo_operacion(conexionMemoria, AUX_SOY_KERNEL); // Le dice a memoria que módulo se conectó

    inicializar_estructuras();

    inicializar_diccionario_recursos();

    int servidorKernel = iniciar_servidor(config, kernelLogger);


    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    /*TODO: NUNCA LLEGA ACA PORQUE SE QUEDA ESPERANDO NUEVAS CONSOLAS,
    MOVER ESTAS FUNCIONES A CUANDO EL SISTEMA SOLICITE LA FINALIZACION*/

    terminar_programa(servidorKernel, kernelLogger, config);
    liberar_recursos_kernel();

    return 0;
}

void inicializar_estructuras(){
    archivosAbiertosGlobal = list_create();
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

    log_info(kernelLogger, "Config cargada en  'kernelConfig' ");

    return;
}


void inicializar_escucha_conexiones_consolas(int servidorKernel) {

    while(1) {

        int conexionConConsola = esperar_cliente(servidorKernel, kernelLogger);

        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, (void*) recibir_de_consola, (void*) (intptr_t) conexionConConsola);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void enviar_proceso_a_ready() {
    while(1) {
        sem_wait(&sem_grado_multiprogamacion);
        sem_wait(&sem_proceso_a_ready_inicializar);
        sem_wait(&sem_lista_estados[ENUM_NEW]);

        PCB* pcb = list_get(lista_estados[ENUM_NEW], 0);

        if (conexionMemoria > 0) {
            // Creo el pcb en memoria
            enviar_operacion(conexionMemoria, AUX_CREATE_PCB, sizeof(int), &pcb->id_proceso);
            codigo_operacion codigoRespuesta = recibir_operacion(conexionMemoria);
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
        }

        sem_wait(&sem_lista_estados[ENUM_READY]);
        cambiar_estado_proceso_sin_semaforos(pcb, ENUM_READY);
        sem_post(&sem_lista_estados[ENUM_NEW]);

        sem_post(&sem_lista_estados[ENUM_READY]);

        sem_post(&sem_proceso_a_ready_terminado);
    }
}

void recibir_de_consola(void *clienteAceptado) {

    log_info(kernelLogger, "Inicializando paquete.");
    int  conexionConConsola = (int) (intptr_t)clienteAceptado;
    recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iteratorSinLog);

    nuevo_proceso(listaInstrucciones, conexionConConsola);

    //crear_hilo_planificador(); //TODO: ESTO ESTA MAL ACA

    list_destroy(listaInstrucciones);

    //liberar_conexion(conexionConConsola);

    return;
}

void crear_hilo_planificadores() {
    log_info(kernelLogger, "Inicialización del planificador %s...", kernelConfig->ALGORITMO_PLANIFICACION);

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

void* liberar_pcb_de_exit(void* args){
    for (;;) {
        PCB *pcb_a_liberar = desencolar_primer_pcb(ENUM_EXIT);

        // TODO: MEMORIA --> adapter_memoria_finalizar_proceso(pcbALiberar);
        //stream_send_empty_buffer(pcb_get_socket(pcbALiberar), HEADER_proceso_terminado);

        //destruir_pcb(pcb_a_liberar);
        sem_post(&sem_grado_multiprogamacion);
    }

    return NULL;
}
/*
void destruir_pcb(PCB* pcb) {
    //pthread_mutex_lock(pcb_get_mutex(pcb));

    t_buffer *instrucciones = pcb->lista_instrucciones;
    if (instrucciones != NULL) {
        free(instrucciones->stream);
        free(instrucciones);
    }

    registros_cpu *registrosCpu = pcb->registrosCpu;
    if (registrosCpu != NULL) {
        if (registrosCpu->AX != NULL) free(registrosCpu->AX);
        if (registrosCpu->BX != NULL) free(registrosCpu->BX);
        if (registrosCpu->CX != NULL) free(registrosCpu->CX);
        if (registrosCpu->DX != NULL) free(registrosCpu->DX);
        // Registros 8 bytes
        if (registrosCpu->EAX != NULL) free(registrosCpu->EAX);
        if (registrosCpu->EBX != NULL) free(registrosCpu->EBX);
        if (registrosCpu->ECX != NULL) free(registrosCpu->ECX);
        if (registrosCpu->EDX != NULL) free(registrosCpu->EDX);
        // Registros 16 bytes
        if (registrosCpu->RAX != NULL) free(registrosCpu->RAX);
        if (registrosCpu->RBX != NULL) free(registrosCpu->RBX);
        if (registrosCpu->RCX != NULL) free(registrosCpu->RCX);
        if (registrosCpu->RDX != NULL) free(registrosCpu->RDX);

        free(registrosCpu);
    }

    list_destroy(pcb->lista_segmentos);

    list_destroy(pcb->lista_archivos_abiertos);

    free(pcb);
}
*/

PCB* desencolar_primer_pcb(pcb_estado estado) {
    int estadoNumerico = (int)estado;
    sem_wait(&sem_lista_estados[estadoNumerico]);
    // pthread_mutex_lock(mutex_lista_estados[estado_]);
    PCB *pcb = list_remove(lista_estados[estadoNumerico], 0);
    sem_post(&sem_lista_estados[estadoNumerico]);
    // pthread_mutex_unlock(mutex_lista_estados[estado_]);

    return pcb;
}


void _planificador_corto_plazo() {

    // Desalojo de PCBs
    pthread_t manejo_desalojo;
    pthread_create(&manejo_desalojo, NULL, manejo_desalojo_pcb, NULL);
    pthread_detach(manejo_desalojo);

    //Dispatcher
    while(1) {
        sem_wait(&sem_cpu_disponible);
        sem_wait(&sem_proceso_a_ready_terminado);
        PCB* pcbParaEjecutar;

        if(string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO")) {
            pcbParaEjecutar = elegir_pcb_segun_fifo();
        }
        else if (string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")) {
            pcbParaEjecutar = elegir_pcb_segun_hrrn();
        }

        agregar_a_lista_con_sem((void*)pcbParaEjecutar, ENUM_EXECUTING);

        log_trace(kernelLogger, "------MOSTRANDO PCB ELEGIDO POR ALGORITMO PARA ENVIAR A CPU--------");
        // mostrar_pcb(pcbParaEjecutar, kernelLogger);

        sem_post(&sem_proceso_a_executing);
    }
}

void* manejo_desalojo_pcb() {
    for(;;) {
        sem_wait(&sem_proceso_a_executing);
        sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
        PCB* pcb_para_cpu = list_remove(lista_estados[ENUM_EXECUTING], 0);
        sem_post(&sem_lista_estados[ENUM_EXECUTING]);

        timestamp inicio_ejecucion_proceso;
        timestamp fin_ejecucion_proceso;

        set_timespec(&inicio_ejecucion_proceso);
        if (conexionCPU > 0) {
        	enviar_pcb(conexionCPU, pcb_para_cpu, OP_EXECUTE_PCB, kernelLogger);
        } else {
        	log_error(kernelLogger, "ERROR CONEXION CPU NO FUE HECHA BIEN, NO SE PUEDE MANDAR EL PCB");
        	mostrar_pcb(pcb_para_cpu, kernelLogger);
        	liberar_recursos_kernel();
        	exit(EXIT_FAILURE);
        }
        codigo_operacion operacionRecibida = recibir_operacion(conexionCPU);
        log_debug(kernelLogger, "CODIGO DE OPERACION RECIBIDO: %d", operacionRecibida);

        PCB* pcb_en_ejecucion = malloc(sizeof(PCB));
        pcb_en_ejecucion = recibir_proceso_desajolado(pcb_para_cpu);
        free(pcb_para_cpu);

        set_timespec(&fin_ejecucion_proceso);

         // Actualizo el estimado en el pcb segun el real ejecutado
         double tiempo_en_cpu = obtener_diferencial_de_tiempo_en_milisegundos(&fin_ejecucion_proceso, &inicio_ejecucion_proceso);
         pcb_estimar_proxima_rafaga(pcb_en_ejecucion, tiempo_en_cpu);

         char* ultimaInstruccion;
         char** ultimaInstruccionDecodificada;
         ultimaInstruccion = string_duplicate((char *)list_get(pcb_en_ejecucion->lista_instrucciones, pcb_en_ejecucion->contador_instrucciones));
         ultimaInstruccionDecodificada = decode_instruccion(ultimaInstruccion, kernelLogger);


        t_data_desalojo* data = malloc(sizeof(t_data_desalojo));
        data->instruccion = ultimaInstruccionDecodificada;
        data->operacion = operacionRecibida;
        data->pcb = pcb_en_ejecucion;

        codigo_operacion res = manejo_instrucciones(data);
        if(res==AUX_SOLO_CON_COMPACTACION){
            data->operacion = AUX_SOLICITUD_COMPACTACION;
            res = manejo_instrucciones(data);
        }

         free(ultimaInstruccion);
         free(ultimaInstruccionDecodificada);
         free(data);
        }
    return NULL;
    }

codigo_operacion manejo_instrucciones(t_data_desalojo* data){
	codigo_operacion res;
	codigo_operacion operacion = data->operacion;
    PCB* pcb = data->pcb;
    char** instruccion = data->instruccion;

	switch(operacion) {
		 	 case I_YIELD: {// TODO mandar el proceso a la cola de ready
		 		 cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);
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

                enviar_operacion(conexionFileSystem, operacion, tamanioPalabra, nombreArchivo);
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
                sem_post(&sem_proceso_a_executing);
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

                sem_post(&sem_proceso_a_executing);
                sem_post(&sem_cpu_disponible);
                break;
            }

            case I_TRUNCATE: {
            	agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
            	sem_post(&sem_cpu_disponible);
                t_paquete* paquete = crear_paquete(operacion);
                agregar_a_paquete(paquete, (void*)instruccion[1], strlen(instruccion[1]));
                agregar_a_paquete(paquete, (void*)instruccion[2], strlen(instruccion[2]));

                enviar_paquete(paquete, conexionFileSystem);
                log_info(kernelLogger, "ENVIO TRUNCATE de archivo: %s, tamanio: %s", instruccion[1], instruccion[2]);
                eliminar_paquete(paquete);
                codigo_operacion cod1 = recibir_operacion(conexionFileSystem);
                log_info(kernelLogger, "Exitoso TRUNCATE de archivo: %s, tamanio: %s", instruccion[1], instruccion[2]);

                recibir_operacion(conexionFileSystem); // basura

                cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);

                sem_post(&sem_proceso_a_ready_terminado);
                break;
            }

            case I_F_SEEK: {
                char* nombreArchivo = instruccion[1];
                char *endptr;
                uint32_t puntero = strtoul(instruccion[2], &endptr, 10);
                t_archivo_abierto* archivoAbierto = encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);
                archivoAbierto->puntero = puntero;
                log_info(kernelLogger, F_SEEK_HECHO, pcb->id_proceso, nombreArchivo, puntero);
                agregar_a_lista_con_sem((void*)pcb, ENUM_EXECUTING);
                sem_post(&sem_proceso_a_executing);

                break;
            }
            case I_F_READ: {
            	agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
                enviar_f_read_write(pcb, instruccion, operacion);
                codigo_operacion codRes = recibir_operacion(conexionFileSystem);
				recibir_operacion(conexionFileSystem);
                break;
            }
            case I_F_WRITE: {
            	agregar_a_lista_con_sem((void*)pcb, ENUM_BLOCKED);
            	enviar_f_read_write(pcb, instruccion, operacion);
            	codigo_operacion codRes = recibir_operacion(conexionFileSystem);
            	mover_de_lista_con_sem(pcb->id_proceso, ENUM_READY, ENUM_BLOCKED);
            	recibir_operacion(conexionFileSystem);
                break;
            }
			 case AUX_SOLICITUD_COMPACTACION: {
				pthread_mutex_lock(&permiso_compactacion);

				log_info(kernelLogger,INICIO_COMPACTACIÓN);
				enviar_operacion(conexionMemoria, operacion, sizeof(int),1);
				res=recibir_operacion(conexionMemoria);

				pthread_mutex_unlock(&permiso_compactacion);
				if(res==AUX_OK){log_info(kernelLogger,FIN_COMPACTACIÓN);}
				else{log_error(kernelLogger,"ERROR compactacion");}
				break;
			 }
			 case I_CREATE_SEGMENT: {
                t_segmento_tabla* tabla_segmento = malloc(sizeof(tabla_segmento));
				t_segmento* segmento = malloc(sizeof(segmento));
				segmento->direccionBase = (void*)(intptr_t)0;
				segmento->id = atoi(instruccion[1]);
				segmento->size = strtoul(instruccion[2],NULL,10);
                tabla_segmento->idProceso = pcb->id_proceso;
                tabla_segmento->segmento = segmento;
                enviar_nuevo_segmento_por_pid(conexionMemoria,tabla_segmento);
                res = recibir_operacion(conexionMemoria);
				if (res == AUX_OK){
					 log_info(kernelConfig,CREAR_SEGMENTO,pcb->id_proceso,tabla_segmento->segmento->id,tabla_segmento->segmento->size);
				} else if(res == AUX_SOLO_CON_COMPACTACION){
					break;
				} else {
					log_error(kernelLogger,E__BAD_REQUEST);
				}
				break;
			 }
			 case I_DELETE_SEGMENT: {
				int id_segmento = atoi(instruccion[1]);

				enviar_operacion(conexionMemoria, operacion, sizeof(int), id_segmento);
				res = recibir_operacion(conexionMemoria);
				if (res == AUX_OK){
					log_info(kernelConfig,ELIMINAR_SEGMENTO,pcb->id_proceso,id_segmento);
				}else{
					log_error(kernelLogger,E__ELIMINAR_SEGMENTO,pcb->id_proceso,id_segmento);
				}
				break;
			 }
			 default:
			 	res=AUX_ERROR;
			 break;
		 }
	return res;
}

void enviar_f_read_write(PCB* pcb, char** instruccion, codigo_operacion codigoOperacion) {
    pthread_mutex_lock(&permiso_compactacion);
    t_paquete* paquete = crear_paquete(codigoOperacion);

    char* nombreArchivo = instruccion[1];
    strtok(nombreArchivo, "\n");

    t_archivo_abierto* archivoAbierto = encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);

    // 1: Nombre Archivo, 2: Dirección Fisica, 3: Cantidad de bytes
    agregar_a_paquete(paquete, (void*)instruccion[1], strlen(instruccion[1]));
    agregar_a_paquete(paquete, (void*)instruccion[2], strlen(instruccion[2]));
    agregar_a_paquete(paquete, (void*)instruccion[3], strlen(instruccion[3]));
    agregar_a_paquete(paquete, (void*)(intptr_t)pcb->id_proceso, sizeof(pcb->id_proceso));
    enviar_paquete(paquete, conexionFileSystem);
    eliminar_paquete(paquete);

    codigo_operacion codOp = recibir_operacion(conexionFileSystem);
    recibir_operacion(conexionFileSystem);
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
    log_warning("No se encontró el archivo %s en la lista de archivos abiertos.", nombreArchivo);
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
    // list_add_all(pcb_recibido->lista_archivos_abiertos, pcb_en_ejecucion->lista_archivos_abiertos); // rompe

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
    //sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
    pcb = list_remove(lista_estados[ENUM_READY], 0);
    sem_post(&sem_lista_estados[ENUM_READY]);
    //sem_post(&sem_lista_estados[ENUM_EXECUTING]);

    return pcb;

}

PCB* elegir_pcb_segun_hrrn(){
    PCB* pcb;
    sem_wait(&sem_lista_estados[ENUM_READY]);
    //sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
    list_sort(lista_estados[ENUM_READY], (void*) criterio_hrrn);
    pcb = list_remove(lista_estados[ENUM_READY], 0);
    //sem_post(&sem_lista_estados[ENUM_EXECUTING]);

    return pcb;
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

    pcb->lista_segmentos = list_create();
    pcb->lista_archivos_abiertos = list_create();
    pcb->estimacion_rafaga = kernelConfig->ESTIMACION_INICIAL;
    pcb->ready_timestamp = 0; //TODO
    //pcb->hrrn_alfa = kernelConfig->HRRN_ALFA;

    agregar_a_lista_con_sem(pcb, ENUM_NEW);
    log_info(kernelLogger, "Se crea el proceso %d en NEW", pcb->id_proceso);

    sem_post(&sem_proceso_a_ready_inicializar); // Le envio señal al otro hilo para que cree la estructura y lo mueva a READY cuando pueda

    return pcb;
    //char* list_ids = pids_on_list(ENUM_READY);

    //log_info(kernelLogger, "El pcb entro en la cola de %s", NEW);

    //log_info(kernelLogger, "Cola Ready %s: [%s]",kernelConfig->ALGORITMO_PLANIFICACION,list_ids);
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

    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {

        lista_estados[estado] = list_create();

        //sem_init(&sem_lista_estados[estado], 0, );
        sem_init(&sem_lista_estados[estado], 0, 1);
        // mutex_lista_estados[estado] = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        // pthread_mutex_init(mutex_lista_estados[estado], NULL);

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
            set_timespec((timestamp*)(time_t)pcb->ready_timestamp);
		}

		PCB* pcbEliminado = (PCB*)list_remove(lista_estados[estadoAnterior], index);

		if (pcbEliminado->id_proceso != pcb->id_proceso) {
			log_error(kernelLogger, "ERROR AL MOVER pcb de estado %s a estado %s",
					nombres_estados[estadoAnterior], nombres_estados[estadoNuevo] );
		}
		int indexDevuelto = list_add(lista_estados[estadoNuevo], (void*)pcb);
		log_info(kernelLogger, "Se ha añadido el pcb <%d> a la lista de estados <%s> devuelve indice %d",
				pcb->id_proceso, nombres_estados[estadoNuevo], indexDevuelto);

		sem_post(&sem_lista_estados[estadoAnterior]);
		sem_post(&sem_lista_estados[estadoNuevo]);
		return;
	}
	log_warning(kernelLogger, "PCB se intento mover de estado pero ya estaba en estado %s",
			nombres_estados[estadoAnterior]);
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

double calculo_HRRN(PCB* pcb) {
    double rafaga = rafaga_estimada(pcb);
    double res = 1.0 + (pcb->ready_timestamp / rafaga);
    return res;
}

static bool criterio_hrrn(PCB* pcb_A, PCB* pcb_B) {
    double a = calculo_HRRN(pcb_A);
    double b = calculo_HRRN(pcb_B);

    return a <= b;
}

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

    sem_t sem;
    sem_init(&sem, instancias, 0);
    recurso->sem_recurso = sem;

    dictionary_put(diccionario_recursos, nombre_recurso, recurso);

}

void inicializar_semaforos() {
    sem_init(&sem_grado_multiprogamacion, 0, kernelConfig->GRADO_MAX_MULTIPROGRAMACION);
    sem_init(&sem_cpu_disponible, 0, 1);
    sem_init(&sem_proceso_a_ready_inicializar, 0, 0);
    sem_init(&sem_proceso_a_ready_terminado, 0, 0);
    sem_init(&sem_proceso_a_executing, 0, 0);
    pthread_mutex_init(&permiso_compactacion,NULL);
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
    log_info(kernelLogger, "Cola %s %s: %s.",estado2, algoritmo, pids_aux);
    free(pids_aux);
}
