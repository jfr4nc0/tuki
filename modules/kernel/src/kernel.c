#include "kernel.h"

void liberar_recursos_kernel() {
    free(kernelConfig);
    liberar_listas_estados();
    liberar_conexion(conexionCPU);
    liberar_conexion(conexionMemoria);
    liberar_conexion(conexionFileSystem);
}


int main(int argc, char** argv) {
	/*
    timestamp * valorTime = (timestamp *)(time_t)5;
	double valor = 5;
	set_timespec(valorTime);
	*/
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
                t_list* segmentos = recibir_paquete(conexionMemoria);
                log_debug(kernelLogger, "Se creó en memoria el proceso %d, semgmentos creados: %d", pcb->id_proceso, list_size(segmentos));
                pcb->lista_segmentos = segmentos;
            } else {
                log_error(kernelLogger, "Error interno en Modulo Memoria para crear proceso id: %d.", pcb->id_proceso);
            }
        }

        sem_wait(&sem_lista_estados[ENUM_READY]);
        cambiar_estado_proceso_sin_semaforos(pcb, ENUM_READY);
        sem_post(&sem_lista_estados[ENUM_NEW]);

        list_add(lista_estados[ENUM_READY], pcb);
        sem_post(&sem_lista_estados[ENUM_READY]);

        sem_post(&sem_proceso_a_ready_terminado);
    }
}

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}

void recibir_de_consola(void *clienteAceptado) {

    log_info(kernelLogger, "Inicializando paquete.");
    int  conexionConConsola = (int) (intptr_t)clienteAceptado;
    recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iterator);

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

        cambiar_estado_proceso_con_semaforos(pcbParaEjecutar, ENUM_EXECUTING);

        log_trace(kernelLogger, "---------------MOSTRANDO PCB A ENVIAR A CPU---------------");
        mostrar_pcb(pcbParaEjecutar);

        sem_post(&sem_proceso_a_executing);
    }
}

void *manejo_desalojo_pcb() {
    for(;;) {
        sem_wait(&sem_proceso_a_executing);
        sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
        PCB* pcb_en_ejecucion = list_get(lista_estados[ENUM_EXECUTING], 0);
        sem_post(&sem_lista_estados[ENUM_EXECUTING]);

        timestamp inicio_ejecucion_proceso;
        timestamp fin_ejecucion_proceso;

        set_timespec(&inicio_ejecucion_proceso);
        envio_pcb_a_cpu(conexionCPU, pcb_en_ejecucion, OP_EXECUTE_PCB);

        codigo_operacion operacionRecibida = recibir_operacion(conexionCPU);
        log_info(kernelLogger, "CODIGO DE OPERACION RECIBIDO: %d", operacionRecibida);

        pcb_en_ejecucion = recibir_proceso_desajolado(pcb_en_ejecucion);
        pcb_en_ejecucion->contador_instrucciones++;
        set_timespec(&fin_ejecucion_proceso);

         // Actualizo el estimado en el pcb segun el real ejecutado
         double tiempo_en_cpu = obtener_diferencial_de_tiempo_en_milisegundos(&fin_ejecucion_proceso, &inicio_ejecucion_proceso);
         pcb_estimar_proxima_rafaga(pcb_en_ejecucion, tiempo_en_cpu);

         char* ultimaInstruccion = malloc(sizeof(char*));
         char** ultimaInstruccionDecodificada = malloc(sizeof(char*));
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
		 	 case I_YIELD: {
		 		 break;
		 	 }
		 	 case I_F_OPEN: {
                bool estaEnReady = true;
                char* nombreArchivo = instruccion[1];
                strtok(nombreArchivo, "$");

                t_semaforo_recurso* semaforoArchivo = (t_semaforo_recurso*) dictionary_get(tablaArchivosAbiertos, nombreArchivo);
                t_estado* estado = semaforoArchivo == NULL ? NULL : semaforoArchivo->estadoRecurso;

                if (semaforoArchivo != NULL) {
                    // Esta abierto y siendo usado, hay que bloquearlo
                    pthread_mutex_lock(estado->mutexEstado);

                    list_add(estado->listaProcesos, pcb);

                    cambiar_estado_proceso_con_semaforos(pcb, ENUM_BLOCKED);
                    pthread_mutex_unlock(estado->mutexEstado);
                    semaforoArchivo->instancias--;
                    sem_post(estado->semaforoEstado);
                    sem_post(&sem_proceso_a_ready_terminado);
                    // continue;
                }

                size_t tamanioPalabra = strlen(nombreArchivo)-1;
                log_error(kernelLogger, "El tamanio del nombre de archivo es %zu", tamanioPalabra);

                enviar_operacion(conexionFileSystem, operacion, tamanioPalabra, nombreArchivo);
                codigo_operacion operacionRecibida = recibir_operacion(conexionFileSystem);

                if (operacionRecibida != AUX_OK) {
                    // Si no existe lo creo
                    enviar_operacion(conexionFileSystem, KERNEL_CREAR_ARCHIVO, tamanioPalabra, nombreArchivo);
                    recibir_operacion(conexionFileSystem);
                }

                // abrir archivo globalmente
                semaforoArchivo->instancias = 0;
                semaforoArchivo->estadoRecurso = crear_archivo_estado(ENUM_ARCHIVO_BLOCK);
                dictionary_put(tablaArchivosAbiertos, nombreArchivo, semaforoArchivo);

                t_archivo_abierto* archivoAbierto = malloc(sizeof(t_archivo_abierto));
                archivoAbierto->nombreArchivo = nombreArchivo;
                archivoAbierto->puntero = 0;

                // abrir archivo proceso
                list_add(pcb->lista_archivos_abiertos, archivoAbierto);


                cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);

                break;
            }

            case I_F_CLOSE: {
                char* nombreArchivo = instruccion[1];
                strtok(nombreArchivo, "$");
                list_remove_element(pcb->lista_archivos_abiertos, nombreArchivo);

                t_semaforo_recurso* semaforoArchivo = (t_semaforo_recurso*) dictionary_get(tablaArchivosAbiertos, nombreArchivo);
                t_estado* estado = semaforoArchivo->estadoRecurso;

                pthread_mutex_lock(estado->mutexEstado);
                bool debeDesbloquearAlgunProceso = !list_is_empty(estado->listaProcesos);
                PCB* pcb = list_remove(estado->listaProcesos, 0);
                if (debeDesbloquearAlgunProceso) {
                    sem_wait(estado->semaforoEstado);
                    pthread_mutex_lock(estado->mutexEstado);
                    cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);
                    pthread_mutex_unlock(estado->mutexEstado);
                } else {
                    // Ya no quedan procesos que usen el archivo
                    dictionary_remove(tablaArchivosAbiertos, nombreArchivo);
                }

                sem_post(&sem_proceso_a_executing);
                break;
            }

            case I_TRUNCATE: {
                char* nombreArchivo = instruccion[1];
                strtok(nombreArchivo, "$");
                cambiar_estado_proceso_con_semaforos(pcb, ENUM_BLOCKED);

                t_archivo_abierto* archivo;
                archivo->nombreArchivo = nombreArchivo;
                archivo->puntero = (uint32_t)(uintptr_t)instruccion[2];
                // cambiar_estado_proceso_con_semaforos(pcb, ENUM_BLOCKED);
                enviar_operacion(conexionFileSystem, operacion, sizeof(t_archivo_abierto), archivo);
                recibir_operacion(conexionFileSystem);

                cambiar_estado_proceso_con_semaforos(pcb, ENUM_READY);
                sem_post(&sem_proceso_a_ready_terminado);
                break;
            }

            case I_F_SEEK: {
                char* nombreArchivo = instruccion[1];
                uint32_t puntero = (uint32_t)(uintptr_t)instruccion[2];
                strtok(nombreArchivo, "$");
                t_archivo_abierto* archivoAbierto = encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);
                archivoAbierto->puntero = puntero;
                sem_post(&sem_proceso_a_executing);

                break;
            }
            case I_F_READ: {
                cambiar_estado_proceso_con_semaforos(pcb, ENUM_BLOCKED);
                enviar_f_read_write(pcb, instruccion, operacion);
                break;
            }
			 case AUX_SOLICITUD_COMPACTACION: {
				pthread_mutex_lock(&permiso_compactacion);

				log_info(kernelLogger,INICIO_COMPACTACIÓN);
				enviar_operacion(conexionMemoria, operacion, sizeof(int*),1);
				res=recibir_operacion(conexionMemoria);
				
				pthread_mutex_unlock(&permiso_compactacion);
				if(res==AUX_OK){log_info(kernelLogger,FIN_COMPACTACIÓN);}
				else{log_error(kernelLogger,"ERROR compactacion");}
				break;
			 }
			 case I_CREATE_SEGMENT: {
				t_segmento* segmento;
				segmento->direccionBase = 0;
				segmento->id = instruccion[1];
				segmento->size = instruccion[2];
				
				enviar_operacion(conexionMemoria, operacion, sizeof(t_segmento*), segmento);
				res = recibir_operacion(conexionMemoria);
				if (res == AUX_OK){
					log_info(kernelConfig,CREAR_SEGMENTO,pcb->id_proceso,segmento->id,segmento->size);
				} else if(res == AUX_SOLO_CON_COMPACTACION){
					break;
				} else {
					log_error(kernelLogger,E__BAD_REQUEST);
				}
				break;
			 }
			 case I_DELETE_SEGMENT: {
				int id_segmento = instruccion[1];

				enviar_operacion(conexionMemoria, operacion, sizeof(int), id_segmento);
				res = recibir_operacion(conexionMemoria);
				if (res == AUX_OK){
					log_info(kernelConfig,ELIMINAR_SEGMENTO,pcb->id_proceso,id_segmento);
				}else{
					log_error(kernelLogger,E__ELIMINAR_SEGMENTO,pcb->id_proceso,id_segmento);
				}
				break;
			 }

			 case I_F_WRITE: {
				pthread_mutex_lock(&permiso_compactacion);

				pthread_mutex_unlock(&permiso_compactacion);
			 }
			 default:
			 	res=AUX_ERROR;
			 break;
		 }
	return res;
}

void enviar_f_read_write(PCB* pcb, char** instruccion, codigo_operacion codigoOperacion) {
    pthread_mutex_lock(&permiso_compactacion);
    char* nombreArchivo = instruccion[1];
    strtok(nombreArchivo, "$");
    uint32_t direccion = (uint32_t)(uintptr_t)instruccion[2];
    uint32_t cantidadBytes = (uint32_t)(uintptr_t)instruccion[3];
    t_parametros_hilo_IO* parametrosHilos;
    uint32_t punteroArchivo = (uint32_t)(intptr_t)(void*)encontrar_archivo_abierto(pcb->lista_archivos_abiertos, nombreArchivo);

    parametrosHilos->nombreArchivo = nombreArchivo;
    parametrosHilos->punteroArchivo = punteroArchivo;
    parametrosHilos->idProceso = pcb->id_proceso;
    parametrosHilos->cantidadBytes = cantidadBytes;
    parametrosHilos->direccionFisica = direccion;

    enviar_operacion(conexionFileSystem, codigoOperacion, sizeof(t_parametros_hilo_IO), parametrosHilos);
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
        if (archivoAbierto->nombreArchivo == nombreArchivo) {
            return archivoAbierto;
        }
    }

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

    PCB* pcb_recibido = recibir_pcb_de_cpu();

    int id_proceso_en_ejecucion = pcb_en_ejecucion->id_proceso;
    int id_pcb_recibido = pcb_recibido->id_proceso;

    if(id_proceso_en_ejecucion != id_pcb_recibido) {
        log_error(kernelLogger, "El PID: %d del proceso desalojado no coincide con el proceso en ejecución con PID: %d", id_proceso_en_ejecucion, id_pcb_recibido);
        exit(EXIT_FAILURE);
    }

    // pcb_recibido->lista_archivos_abiertos = pcb_en_ejecucion->lista_archivos_abiertos;
    list_add_all(pcb_recibido->lista_archivos_abiertos, pcb_en_ejecucion->lista_archivos_abiertos);

    return pcb_recibido;
}

PCB* recibir_pcb_de_cpu() {
    PCB* pcb = malloc(sizeof(PCB));

    char* buffer;
    int tamanio = 0;
    int desplazamiento = 0;

    buffer = recibir_buffer(&tamanio, conexionCPU);

    pcb->id_proceso = leer_int(buffer, &desplazamiento);

    pcb->estado = leer_int(buffer, &desplazamiento);

    pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento); // NO esta funcionando bien

    pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);

    pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento); //TODO: Modificar cuando se mergee memoria


    pcb->lista_archivos_abiertos = list_create();
    /*
     *     int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
    for (int i = 0; i < cantidad_de_archivos; i++) {
            t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));

            archivo_abierto->nombreArchivo = leer_string(buffer, &desplazamiento);
            archivo_abierto->puntero = leer_int(buffer, &desplazamiento);

            list_add(pcb->lista_archivos_abiertos, archivo_abierto);
            free(archivo_abierto);
    }
    */

    pcb->registrosCpu = malloc(sizeof(registros_cpu));
    strcpy(pcb->registrosCpu->AX, leer_registro_4_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->BX, leer_registro_4_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->CX, leer_registro_4_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->DX, leer_registro_4_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->EAX, leer_registro_8_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->EBX, leer_registro_8_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->ECX, leer_registro_8_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->EDX, leer_registro_8_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->RAX, leer_registro_16_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->RBX, leer_registro_16_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->RCX, leer_registro_16_bytes(buffer, &desplazamiento));
    strcpy(pcb->registrosCpu->RDX, leer_registro_16_bytes(buffer, &desplazamiento));

    pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
    pcb->ready_timestamp = leer_double(buffer, &desplazamiento);
    log_info(kernelLogger, "---------------------------------------------Recibi el proceso de PID: %d", pcb->id_proceso);
    mostrar_pcb(pcb);
    return pcb;
}


/*
PCB* recibir_proceso_desalojado(int socket){
    PCB* pcb = malloc(sizeof(PCB));

    char* buffer;
    int tamanio = 0;
    int desplazamiento = 0;

    buffer = recibir_buffer(&tamanio, socket);

    pcb->id_proceso = leer_int(buffer, &desplazamiento);

    pcb->estado = leer_int(buffer, &desplazamiento);

    pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento); // NO esta funcionando bien

    pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);

    pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento); //TODO: Modificar cuando se mergee memoria

    pcb->lista_archivos_abiertos = list_create();
    int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
    for (int i = 0; i < cantidad_de_archivos; i++) {
    t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));

    archivo_abierto->id = leer_int(buffer, &desplazamiento);
    archivo_abierto->puntero = leer_int(buffer, &desplazamiento);

    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
    free(archivo_abierto);
    }

    pcb->registrosCpu = malloc(sizeof(registros_cpu));
    pcb->registrosCpu->AX = leer_registro_4_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->BX = leer_registro_4_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->CX = leer_registro_4_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->DX = leer_registro_4_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->EAX = leer_registro_8_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->EBX = leer_registro_8_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->ECX = leer_registro_8_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->EDX = leer_registro_8_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->RAX = leer_registro_16_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->RBX = leer_registro_16_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->RCX = leer_registro_16_bytes(buffer, &desplazamiento);
    pcb->registrosCpu->RDX = leer_registro_16_bytes(buffer, &desplazamiento);

    pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
    pcb->ready_timestamp = leer_double(buffer, &desplazamiento);

    return pcb;
}
*/
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
    pcb = list_get(lista_estados[ENUM_READY], 0);
    sem_post(&sem_lista_estados[ENUM_READY]);
    //sem_post(&sem_lista_estados[ENUM_EXECUTING]);

    return pcb;

}

PCB* elegir_pcb_segun_hrrn(){
    PCB* pcb;
    sem_wait(&sem_lista_estados[ENUM_READY]);
    //sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
    list_sort(lista_estados[ENUM_READY], (void*) criterio_hrrn);
    pcb = list_get(lista_estados[ENUM_READY], 0);
    sem_post(&sem_lista_estados[ENUM_READY]);
    //sem_post(&sem_lista_estados[ENUM_EXECUTING]);

    return pcb;
}

void mostrar_pcb(PCB* pcb){
    log_trace(kernelLogger, "PID: %d", pcb->id_proceso);
    char* estado = nombres_estados[pcb->estado];
    log_trace(kernelLogger, "ESTADO: %s", estado);
    log_trace(kernelLogger, "INSTRUCCIONES A EJECUTAR: ");
    list_iterate(pcb->lista_instrucciones, (void*) iterator);
    log_trace(kernelLogger, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
    log_trace(kernelLogger, "Registro AX: %s", pcb->registrosCpu->AX);
    log_trace(kernelLogger, "Registro BX: %s", pcb->registrosCpu->BX);
    log_trace(kernelLogger, "Registro CX: %s", pcb->registrosCpu->CX);
    log_trace(kernelLogger, "Registro DX: %s", pcb->registrosCpu->DX);
    log_trace(kernelLogger, "Registro EAX: %s", pcb->registrosCpu->EAX);
    log_trace(kernelLogger, "Registro EBX: %s", pcb->registrosCpu->EBX);
    log_trace(kernelLogger, "Registro ECX: %s", pcb->registrosCpu->ECX);
    log_trace(kernelLogger, "Registro EDX: %s", pcb->registrosCpu->EDX);
    log_trace(kernelLogger, "Registro RAX: %s", pcb->registrosCpu->RAX);
    log_trace(kernelLogger, "Registro RBX: %s", pcb->registrosCpu->RBX);
    log_trace(kernelLogger, "Registro RCX: %s", pcb->registrosCpu->RCX);
    log_trace(kernelLogger, "Registro RDX: %s", pcb->registrosCpu->RDX);
    log_trace(kernelLogger, "LISTA SEGMENTOS: ");
    list_iterate(pcb->lista_segmentos, (void*) iterator);
    log_trace(kernelLogger, "LISTA ARCHIVOS ABIERTOS: ");
    // list_iterate(pcb->lista_archivos_abiertos, (void*) iterator);
    log_trace(kernelLogger, "ESTIMACION HHRN: %f", pcb->estimacion_rafaga);
    log_trace(kernelLogger, "TIMESTAMP EN EL QUE EL PROCESO LLEGO A READY POR ULTIMA VEZ: %f", pcb->ready_timestamp);
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
    pcb->estado = estadoNuevo;
    mover_de_lista_con_sem(pcb, estadoNuevo, estadoAnterior);

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
    sem_wait(&sem_lista_estados[estado]);
    list_add(lista_estados[estado], elem);
    sem_post(&sem_lista_estados[estado]);
}

/*
* Funcion auxiliar de cambiar_estado_proceso_con_semaforos
*/
void mover_de_lista_con_sem(void* elem, int estadoNuevo, int estadoAnterior) {
    sem_wait(&sem_lista_estados[estadoNuevo]);
    sem_wait(&sem_lista_estados[estadoAnterior]);
    PCB* pcb = elem;
    pcb->estado = estadoNuevo;
    if (pcb->estado == ENUM_READY) {
    	// TODO: FALLA
         // set_timespec((timestamp*)(time_t)pcb->ready_timestamp);
    }
    list_remove_element(lista_estados[estadoAnterior], elem);
    list_add(lista_estados[estadoNuevo], elem);

    sem_post(&sem_lista_estados[estadoAnterior]);
    sem_post(&sem_lista_estados[estadoNuevo]);

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

//////////Funciones para enviar un pcb a cpu //////////////////////////////
void envio_pcb(int conexion, PCB* pcb, codigo_operacion codigo) {
    t_paquete* paquete = crear_paquete(codigo);
    agregar_pcb_a_paquete(paquete, pcb);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}
// ---------------------------------------PRUEBAS
void envio_pcb_a_cpu(int conexion, PCB* pcb_a_enviar, codigo_operacion codigo) {
    t_paquete* paquete = crear_paquete(codigo);
    agregar_pcb_a_paquete_para_cpu(paquete, pcb_a_enviar);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void agregar_pcb_a_paquete_para_cpu(t_paquete* paquete, PCB* pcb) {
    agregar_registros_a_paquete(paquete, pcb->registrosCpu);
    agregar_int_a_paquete(paquete, pcb->id_proceso);
    agregar_int_a_paquete(paquete, pcb->estado);
    agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);
    agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
    agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
    // agregar_lista_archivos_a_paquete(paquete, pcb->lista_archivos_abiertos);
    agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
    agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
}
/*
void agregar_registros_a_paquete_cpu(t_paquete* paquete, registros_cpu* registrosCpu) {
     agregar_int_a_paquete(paquete, registrosCpu->AX);
     agregar_int_a_paquete(paquete, registrosCpu->BX);
     agregar_int_a_paquete(paquete, registrosCpu->CX);
     agregar_int_a_paquete(paquete, registrosCpu->DX);
     agregar_long_a_paquete(paquete, registrosCpu->EAX);
     agregar_long_a_paquete(paquete, registrosCpu->EBX);
     agregar_long_a_paquete(paquete, registrosCpu->ECX);
     agregar_long_a_paquete(paquete, registrosCpu->EDX);
     agregar_longlong_a_paquete(paquete, registrosCpu->RAX);
     agregar_longlong_a_paquete(paquete, registrosCpu->RBX);
     agregar_longlong_a_paquete(paquete, registrosCpu->RCX);
     agregar_longlong_a_paquete(paquete, registrosCpu->RDX);
}
*/

void agregar_lista_archivos_a_paquete(t_paquete* paquete, t_list* lista) {
    int tamanio = list_size(lista);
    agregar_int_a_paquete(paquete, tamanio);

    for(int i = 0; i < tamanio; i++) {
        t_archivo_abierto* archivo = list_get(lista, i);

        char* nombreArchivo = (char*)archivo->nombreArchivo;
        strtok(nombreArchivo, "$"); // Removemos el salto de linea
        log_debug(kernelLogger, "Agregando nombreArchivo: %s, tamanio %zu", nombreArchivo, strlen(nombreArchivo));
        agregar_a_paquete(paquete, nombreArchivo, strlen(nombreArchivo));

        log_debug(kernelLogger, "Agregando puntero: %d, tamanio %zu", archivo->puntero, sizeof(uint32_t));
        agregar_a_paquete(paquete, (void*)(int)archivo->puntero, sizeof(int));
    }
}

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista) {
    int tamanio = list_size(lista);
    agregar_int_a_paquete(paquete, tamanio);

    for(int i = 0; i < tamanio; i++) {
        void* elemento = list_get(lista, i);
        char* palabra = (char*)elemento;
        strtok(palabra, "$"); // Removemos el salto de linea
        log_debug(kernelLogger, "Agregando instruccion: %s, tamanio %zu", palabra, strlen(palabra));
        agregar_a_paquete(paquete, palabra, strlen(palabra));
    }

}

void agregar_int_a_paquete(t_paquete* paquete, int valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}


void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}

void agregar_registros_a_paquete(t_paquete* paquete, registros_cpu* registrosCpu) {

    agregar_registro4bytes_a_paquete(paquete, registrosCpu->AX);
    //log_error(kernelLogger, "REGISTRO AX: %d", registrosCpu->AX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->BX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->CX);
    agregar_registro4bytes_a_paquete(paquete, registrosCpu->DX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EAX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EBX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->ECX);
    agregar_registro8bytes_a_paquete(paquete, registrosCpu->EDX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RAX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RBX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RCX);
    agregar_registro16bytes_a_paquete(paquete, registrosCpu->RDX);
}
void agregar_registro4bytes_a_paquete(t_paquete* paquete, char valor[4]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}
void agregar_registro8bytes_a_paquete(t_paquete* paquete, char valor[8]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}
void agregar_registro16bytes_a_paquete(t_paquete* paquete, char valor[16]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long)*2);
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long)*2);
    paquete->buffer->size += sizeof(long)*2;
}


void agregar_long_a_paquete(t_paquete* paquete, long valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}

void agregar_longlong_a_paquete(t_paquete* paquete, long long valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long long));
    paquete->buffer->size += sizeof(long long);
}

void agregar_a_lista(PCB* pcb, t_list* lista, sem_t m_sem) {
    sem_wait(&m_sem);
    list_add(lista, pcb);
    sem_post(&m_sem);
}

void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb) {
    agregar_registros_a_paquete(paquete, pcb->registrosCpu);
    agregar_int_a_paquete(paquete, pcb->id_proceso);
    agregar_int_a_paquete(paquete, pcb->estado);
    agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);
    agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
    agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
    agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
    agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
    agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
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
