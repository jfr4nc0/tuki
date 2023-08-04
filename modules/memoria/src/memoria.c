#include "memoria.h"
#include "administrarMemoria.h"

t_memoria_config* memoriaConfig;

int main(int argc, char** argv) {
    loggerMemoria = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* configInicial = iniciar_config(argv[1], loggerMemoria);
    cargar_config_memoria(configInicial);

    inicializar_memoria(memoriaConfig->TAM_MEMORIA, memoriaConfig->TAM_SEGMENTO_0,memoriaConfig->ALGORITMO_ASIGNACION);

    int servidorMemoria = iniciar_servidor(configInicial, loggerMemoria);

    atender_conexiones(servidorMemoria);

    //pthread_mutex_init(&mutex_memoria_ocupada,NULL);

    terminar_programa_memoria(servidorMemoria, loggerMemoria, configInicial);

	return 0;
}

void atender_conexiones(int socket_servidor){
    log_info(loggerMemoria, "Esperando conexiones de modulos...");

    pthread_t hilo_kernel, hilo_cpu, hilo_fs;
    int conexion_cpu, conexion_fs, conexion_kernel, cliente;
    codigo_operacion codigo;

    //for (int i = 0; i < 3; i++)
    for (int i = 0; i < 2; i++){

    	cliente = esperar_cliente(socket_servidor, loggerMemoria);
    	codigo = recibir_operacion(cliente);
        switch (codigo){
        /*case AUX_SOY_FILE_SYSTEM:
            log_info(loggerMemoria, "Se conecto el file system");
            conexion_fs = cliente;
            pthread_create(&hilo_fs, NULL, (void *)ejecutar_filesystem_pedido, &conexion_fs);
            pthread_detach(hilo_fs);
            break;*/
        case AUX_SOY_KERNEL:
            log_info(loggerMemoria, "Se conecto el kernel");
            conexion_kernel = cliente;
            codigo = recibir_operacion(cliente);
            pthread_create(&hilo_kernel, NULL, (void *)ejecutar_kernel_pedido, (void*)(intptr_t)conexion_kernel);
            pthread_join(hilo_kernel, NULL);
            break;
        case AUX_SOY_CPU:
            log_info(loggerMemoria, "Se conecto el cpu");
            conexion_cpu = cliente;
            codigo = recibir_operacion(cliente);
            pthread_create(&hilo_cpu, NULL, (void *)ejecutar_cpu_pedido, (void*)(intptr_t)conexion_cpu);
            pthread_detach(hilo_cpu);
            break;
        }
    }
}

// --------------------------PEDIDOS FILESYSTEM--------------------------
void ejecutar_filesystem_pedido(int *socket_modulo){
    while (1)
    {
        int cod_op = recibir_operacion(*socket_modulo);
        switch (cod_op){

        case I_F_READ:
            int PID = -1;
            t_parametros_variables *parametros = NULL;
            recibir_acceso(&parametros, &PID, *socket_modulo);
            escribir_valor_direccion_fisica(parametros->parametros[0], strtol(parametros->parametros[1], NULL, 10), PID, "FS");
            enviar_mensaje_memoria("OK", *socket_modulo);
            liberar_parametros_desalojo(parametros);

            break;

        case I_F_WRITE:
            recibir_acceso(&parametros, &PID, *socket_modulo);
            char *valor_leido = leer_valor_direccion_fisica(strtol(parametros->parametros[0], NULL, 10), atoi(parametros->parametros[1]), PID, "FS");
            enviar_mensaje_memoria(valor_leido, *socket_modulo);
            free(valor_leido);
            liberar_parametros_desalojo(parametros);

            break;

        case -1:
            log_info(loggerMemoria, "Se desconecto un modulo");
            return;

        default:
            log_error(loggerMemoria, "Operacion desconocida");
            return;
        }
    }
}
//Recibir acceso
void recibir_acceso(t_parametros_variables **parametros, int *PID, int socket){
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    *PID = leer_int_memoria(buffer, desplazamiento);
    *parametros = deserealizar_motivos_desalojo(buffer, desplazamiento);

    free(buffer);
    free(desplazamiento);
}
int leer_int_memoria(void *buffer, int *desplazamiento){
    int valor = *(int *)(buffer + *desplazamiento);
    *desplazamiento += sizeof(int);
    return valor;
}
t_parametros_variables* deserealizar_motivos_desalojo(void *buffer, int*desplazamiento){
	t_parametros_variables *motivos_desalojo = malloc(sizeof(t_parametros_variables));
	memcpy(&motivos_desalojo->cantidad_parametros, buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	motivos_desalojo->parametros = malloc(sizeof(char *) * motivos_desalojo->cantidad_parametros);
	for (int i = 0; i < motivos_desalojo->cantidad_parametros; i++)
	{
		int tamanio_parametro;
		memcpy(&tamanio_parametro, buffer + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);

		motivos_desalojo->parametros[i] = malloc(tamanio_parametro);
		memcpy(motivos_desalojo->parametros[i], buffer + *desplazamiento, tamanio_parametro);
		*desplazamiento += tamanio_parametro;
	}
	return motivos_desalojo;
}

void escribir_espacio_usuario(void* direccion, size_t size, void* valor, int demora) {
    // simular_tiempo_acceso(demora);

    memcpy(direccion, valor, size);

    //log_debug(loggerMemoria, "Valor escrito en memoria: %s", (char*)direccion);
    return;
}

//Escribir/leer valor de direccion fisica
char *leer_valor_direccion_fisica(long direccion_fisica, int tamanio, int pid, char *origen){
    sleep(memoriaConfig->RETARDO_MEMORIA / 500);
    char* valor = malloc(tamanio);
    memcpy((void*)valor, memoria_principal + direccion_fisica, tamanio);
    //memcpy((void*)valor, (void*)direccion_fisica, tamanio);
    log_info(loggerMemoria, "PID: <%d> - Acción: <LEER> - Dirección física: <%p> - Tamaño: <%d> - Origen: <%s>", pid, (void *)memoria_principal + direccion_fisica, tamanio, origen);
    log_info(loggerMemoria, "el valor es %s", valor);
    return valor;
}

void escribir_valor_en_memoria(long dirFisica, void* bytesRecibidos, uint32_t tamanio, int pid, char *origen)
{
	// bytesRecibidos = malloc(tamanio);
	char* valor = (char*)bytesRecibidos;
    memcpy(memoria_principal+dirFisica, bytesRecibidos, tamanio); // cambiar
    //log_debug(loggerMemoria, "Escribió %s en memoria", valor);
    log_info(loggerMemoria, "PID: <%d> - Acción: <ESCRIBIR> - Dirección física: <%p> - Tamaño: <%d> - Origen: <%s>", pid, dirFisica, tamanio, origen);

    return;
}

void escribir_valor_direccion_fisica(char *valor, long direccion_fisica, int pid, char *origen){

    sleep(memoriaConfig->RETARDO_MEMORIA / 500);
    void *direccion = (void *)direccion_fisica;
    int tamanio = strlen(valor) + 1;
    memcpy(direccion, valor, tamanio);
    log_info(loggerMemoria, "PID: <%d> - Acción: <ESCRIBIR> - Dirección física: <%p> - Tamaño: <%d> - Origen: <%s>", pid, direccion, tamanio, origen);
}

//Enviar mensaje
void enviar_mensaje_memoria(char* mensaje, int socket_cliente){
	t_paquete *paquete = malloc(sizeof(t_paquete));
	int codigo = atoi(mensaje);
	paquete->codigoOperacion = codigo;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}
//Liberar parametros desalojo
void liberar_parametros_desalojo(t_parametros_variables *parametros_variables){
    vaciar_parametros_desalojo(parametros_variables);
    free(parametros_variables->parametros);
    free(parametros_variables);
}
void vaciar_parametros_desalojo(t_parametros_variables *parametros){
    for (int i = 0; i < parametros->cantidad_parametros; i++)    {
        free(parametros->parametros[i]);
    }
    parametros->cantidad_parametros = 0;
}

// --------------------------PEDIDOS CPU--------------------------
void ejecutar_cpu_pedido(void* socket){
	while (1){
		//log_warning(loggerMemoria, "entra al while(1)");
		int socket_modulo = (int)(intptr_t)socket;
		codigo_operacion cod_op1 = recibir_operacion(socket_modulo);
		//log_warning(loggerMemoria, "el cod op recibido de cpu es %d", cod_op1);
	    switch (cod_op1){
	    	case I_MOV_IN:{
	    		//log_warning(loggerMemoria, "entra a MOV_IN");

	            char* buffer;
	            int tamanio = 0;
	            int desplazamiento = 0;

	            //recibir_operacion(socket_modulo);
	            buffer = recibir_buffer(&tamanio, socket_modulo);

	            int pid = leer_int(buffer, &desplazamiento);
	            long direccion_fisica = (long)leer_int(buffer, &desplazamiento);
	            int tamanio_registro = leer_int(buffer, &desplazamiento);

	            //log_debug(loggerMemoria, "%d %d %d", pid, direccion_fisica, tamanio_registro);

	            char *valor_leido = leer_valor_direccion_fisica(direccion_fisica, tamanio_registro, pid, "CPU");

				t_paquete* paquete = crear_paquete(AUX_OK);

				agregar_registro_a_paquete(paquete, valor_leido, tamanio_registro);
				enviar_paquete(paquete, socket_modulo);

	            free(buffer);
	            break;
	    	}
	        case I_MOV_OUT:{
	        	//log_warning(loggerMemoria, "entra a MOV_OUT");

	        	char* buffer;
	        	int tamanio = 0;
	        	int desplazamiento = 0;

	        	//recibir_operacion(socket_modulo);
	        	buffer = recibir_buffer(&tamanio, socket_modulo);

	        	int pid = leer_int(buffer, &desplazamiento);
	        	long direccion_fisica = (long)leer_int(buffer, &desplazamiento);
	        	int tamanioRegistro = leer_int(buffer, &desplazamiento);
//				char* valor_registro = leer_string(buffer, &desplazamiento);
	        	char* valor_registro = leer_registro_de_buffer(buffer, desplazamiento);

				escribir_valor_en_memoria(direccion_fisica, valor_registro, tamanioRegistro, pid, "CPU");
//	        	log_debug(loggerMemoria, "%d %d %s", pid, direccion_fisica, (char*)valor_registro);

//	        	escribir_valor_direccion_fisica(valor_registro, direccion_fisica, pid, "CPU");
	        	//enviar_mensaje_memoria(valor_leido, socket_modulo);
	        	enviar_operacion(socket_modulo, AUX_OK, 0, 0);

	        	free(buffer);
	            break;
	        }
	        case -1:
	            log_info(loggerMemoria, "Se desconecto un modulo");
	            return;

	        default:
	            log_error(loggerMemoria, "Operacion desconocida");
	            return;
	        }
	    }
}

// --------------------------PEDIDOS KERNEL--------------------------
void ejecutar_kernel_pedido(void* socket){
	while (1){
		int socket_modulo = (int)(intptr_t)socket;
		int cod_op = recibir_operacion(socket_modulo);
		//log_warning(loggerMemoria, "el cod op recibido de kernel es %d", cod_op);
		t_paquete *paquete;
		switch (cod_op){
			case AUX_CREATE_PCB:{
				// recibe
				void* buffer;
				int tamanio = 0;
				int desplazamiento = 0;

				buffer = recibir_buffer(&tamanio, socket_modulo);

				int pid = leer_int(buffer, &desplazamiento);
				//log_warning(loggerMemoria, "EL PID %d FUE RECIBIDO DE KERNEL", pid);
	            // crea
	            t_tabla_segmentos *tabla_segmentos = malloc(sizeof(t_tabla_segmentos));
	            tabla_segmentos->PID = pid;
	            tabla_segmentos->segmentos = crear_tabla_segmentos();
	            list_add(tabla_segmentos_global, tabla_segmentos);
	            log_info(loggerMemoria, "Creación de Proceso PID: <%d>", pid);

	            // envia
	            paquete = crear_paquete(AUX_CREATE_PCB);
	            serializar_tabla_segmentos(tabla_segmentos->segmentos, paquete);
	            enviar_paquete(paquete, socket_modulo);

	            // libera
	            eliminar_paquete(paquete);

	            break;
			}
	        case AUX_FINALIZAR_PROCESO:{
	            /*
	        	// recibe
	            int size;
	            void *buffer = recibir_buffer(&size, *socket_modulo);

	            int *desplazamiento = malloc(sizeof(int));
	            *desplazamiento = 0;

	            int pid = leer_int_memoria(buffer, desplazamiento);
	            t_list* tabla_de_segmentos = deserializar_tabla_segmentos(buffer, desplazamiento);

	            // eliminar
	            free(buffer);
	            free(desplazamiento);
	            finalizar_proceso(tabla_de_segmentos, pid);

	            log_info(loggerMemoria, "Eliminacion de Proceso PID: <%d>", pid);
	            enviar_mensaje_memoria("OK", *socket_modulo);
	            */
	            break;
	        }
			case COMPACTACION: {
				compactar();
				paquete = crear_paquete(AUX_OK);
				serializar_todas_las_tablas_segmentos(tabla_segmentos_global, paquete);
				enviar_paquete(paquete, socket_modulo);
				eliminar_paquete(paquete);
			}
	        case I_CREATE_SEGMENT:{

				PCB* pcb = recibir_pcb(socket_modulo);

				recibir_operacion(socket_modulo);
				void* buffer;
				int tamanio = 0;
				int desplazamiento = 0;
	        	buffer = recibir_buffer(&tamanio, socket_modulo);

	        	int id_segmento = leer_int(buffer, &desplazamiento);
	        	int tamanio_segmento = leer_int(buffer, &desplazamiento);

	        	log_warning(loggerMemoria, "%d %d %d", pcb->id_proceso, id_segmento, tamanio_segmento);

				t_paquete* paquete = crear_segmento(id_segmento, tamanio_segmento, pcb);

				enviar_paquete(paquete, socket_modulo);

				eliminar_paquete(paquete);



	        	/*
	            // recibe
	            t_ctx *ctx = recibir_contexto(*socket_modulo);

	            // crea
	            paquete = crear_segmento(atoi(ctx->motivos_desalojo->parametros[0]), atoi(ctx->motivos_desalojo->parametros[1]), ctx);

	            // envia
	            enviar_paquete(paquete, *socket_modulo);

	            // libera
	            eliminar_paquete(paquete);
	            liberar_contexto(ctx);
	            */
	            break;
	        }
	        case I_DELETE_SEGMENT:{
	        	/*
	            // recibe
	            ctx = recibir_contexto(*socket_modulo); // Porque no está inicializada??

	            // elimina
	            eliminar_segmento(ctx->tabla_segmentos, atoi(ctx->motivos_desalojo->parametros[0]), pcb->id_proceso);

	            // envia
	            paquete = crear_paquete(DELETE_SEGMENT);
	            serializar_tabla_segmentos(ctx->tabla_segmentos, paquete);
	            enviar_paquete(paquete, *socket_modulo);

	            // libera
	            eliminar_paquete(paquete);
	            liberar_contexto(ctx);
	            */
	            break;
	        }
	        case -1:
	            log_info(loggerMemoria, "Se desconecto un modulo");
	            return;

	        default:
	            //log_error(loggerMemoria, "Operacion desconocida");
	            break;
	        }
	    }
}

void serializar_todas_las_tablas_segmentos(t_list* tablas_segmentos, t_paquete* paquete){
	agregar_a_paquete_dato_serializado(paquete, &tablas_segmentos->elements_count, sizeof(int));
	for(int i = 0; i < list_size(tablas_segmentos); i++){
		t_tabla_segmentos* tabla_segmentos = list_get(tablas_segmentos, i);
		agregar_a_paquete_dato_serializado(paquete, &(tabla_segmentos->PID), sizeof(int));
		serializar_tabla_segmentos(tabla_segmentos->segmentos, paquete);
	}
}

void compactar()
{
    log_info(loggerMemoria, "Se solicita compactacion");
    usleep(memoriaConfig->RETARDO_COMPACTACION * 250);
    int nuevo_tamanio = 0;
    void *base_del_primer_hueco = NULL;

    for (int i = 1; i < list_size(lista_huecos); i++)
    {
        t_hueco *hueco = list_get(lista_huecos, i);

        if (base_del_primer_hueco == hueco->base)
        {
            hueco->tamanio = nuevo_tamanio;
            // eliminar los huecos que estan a la derecha del hueco que se esta compactando
            for (int j = i + 1; j < list_size(lista_huecos); j++)
            {
                list_remove(lista_huecos, j);
            }
            break;
        }

        if (hueco->libre)
        {
            // agregarlo al final de la lista de huecos
            list_remove(lista_huecos, i);
            list_add(lista_huecos, hueco);
            nuevo_tamanio += hueco->tamanio;
            if (!base_del_primer_hueco)
                base_del_primer_hueco = hueco->base;
            i--;
        }
    }

    // modificar las bases de los huecos en relacion a su tamanio (solo los ocupados)
    t_hueco *hueco = list_get(lista_huecos, 0);
    void *base_actual = hueco->base;
    int tamanio_actual = hueco->tamanio;

    for (int i = 1; i < list_size(lista_huecos); i++)
    {
        t_hueco *hueco = list_get(lista_huecos, i);
        if (!hueco->libre)
        {
            // modificar la tabla de segmentos
            // buscar el segmento en todas las tablas de segmentos
            for (int j = 0; j < list_size(tabla_segmentos_global); j++)
            {
                t_tabla_segmentos *tabla_segmentos = list_get(tabla_segmentos_global, j);
                for (int k = 1; k < list_size(tabla_segmentos->segmentos); k++)
                {
                    t_segmento *segmento = list_get(tabla_segmentos->segmentos, k);
                    if (segmento->direccionBase == hueco->base)
                    {
                        segmento->direccionBase = base_actual + tamanio_actual;
                        log_info(loggerMemoria, "PID: <%d> - Segmento: <%d> - Base: <%p> - TAMAÑO: <%zu>", tabla_segmentos->PID, k, segmento->direccionBase, segmento->size);
                        break;
                    }
                }
            }

            hueco->base = base_actual + tamanio_actual;
            base_actual = hueco->base;
            tamanio_actual = hueco->tamanio;
        }
        else
        {
            hueco->base = base_actual + tamanio_actual;
            tamanio_actual = hueco->tamanio;
        }
    }

}

//Recibir int
int recibir_int(int socket){
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    int valor = leer_int_memoria(buffer, desplazamiento);
    free(buffer);
    free(desplazamiento);
    return valor;
}
void serializar_tabla_segmentos(t_list *tabla_segmentos, t_paquete *paquete){
    agregar_a_paquete_dato_serializado(paquete, &(tabla_segmentos->elements_count), sizeof(int));
    for (int i = 0; i < tabla_segmentos->elements_count; i++)
    {
        segmento_t *segmento = list_get(tabla_segmentos, i);
		// agregar_a_paquete
        agregar_a_paquete_dato_serializado(paquete, &(segmento->id), sizeof(int));
        agregar_a_paquete_dato_serializado(paquete, &(segmento->direccion_base), sizeof(void*));
        agregar_a_paquete_dato_serializado(paquete, &(segmento->tamanio_segmento), sizeof(int));

    }
}
void agregar_a_paquete_dato_serializado(t_paquete *paquete, void *valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);

	// Agrega el valor
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);

	paquete->buffer->size += tamanio;
}

t_list* deserializar_tabla_segmentos(void* buffer, int* desplazamiento){
	t_list* tabla_segmentos = list_create();

	int cansegmento_ts;
    memcpy(&cansegmento_ts, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

	for (int i = 0; i < cansegmento_ts; i++) {
        segmento_t* segmento = malloc(sizeof(segmento_t));

	    memcpy(&segmento->id, buffer + *desplazamiento, sizeof(int));
        *desplazamiento += sizeof(int);

	    memcpy(&segmento->direccion_base, buffer + *desplazamiento, sizeof(void*));
        *desplazamiento += sizeof(void*);

	    memcpy(&segmento->tamanio_segmento, buffer + *desplazamiento, sizeof(int));
        *desplazamiento += sizeof(int);

	    list_add(tabla_segmentos, segmento);
    }
    return tabla_segmentos;
}
void finalizar_proceso(t_list *tabla_segmentos, int PID){
    free(list_remove(tabla_segmentos, 0));
    for (int i = 0; i < list_size(tabla_segmentos); i++)
    {
        segmento_t *segmento = list_get(tabla_segmentos, i);
        if (segmento->direccion_base != NULL)
        {
            // buscar hueco que tenga la misma base que el segmento y marcarlo como libre
            int index_hueco = 0;
            for (int i = 0; i < list_size(lista_huecos); i++)
            {
                t_hueco *hueco = list_get(lista_huecos, i);
                if (hueco->base == segmento->direccion_base)
                {
                    hueco->libre = true;
                    index_hueco = i;
                    break;
                }
            }
            comprobar_consolidacion_huecos_aledanios(index_hueco);
        }
        list_remove_and_destroy_element(tabla_segmentos, i, (void *)liberar_segmentoo);
        i--;
    }

    for (int i = 0; i < tabla_segmentos_global->elements_count; i++){
        t_tabla_segmentos* ts = list_get(tabla_segmentos_global, i);
        if (ts->PID == PID){
            list_remove_and_destroy_element(tabla_segmentos_global, i, (void *)liberar_tabla_segmentos);
            break;
        }
    }
}
void liberar_segmentoo(segmento_t *segmento){
    free(segmento);
}
void liberar_tabla_segmentos(t_tabla_segmentos *ts){
    list_destroy_and_destroy_elements(ts->segmentos, (void *)liberar_segmentoo);
    free(ts);
}
void comprobar_consolidacion_huecos_aledanios(int index_hueco) {
    t_hueco* hueco_actual = list_get(lista_huecos, index_hueco);
    t_hueco* hueco_anterior = NULL;
    t_hueco* hueco_siguiente = NULL;

    if (index_hueco > 1) {
        hueco_anterior = list_get(lista_huecos, index_hueco - 1);
    }

    if (index_hueco < list_size(lista_huecos) - 1) {
        hueco_siguiente = list_get(lista_huecos, index_hueco + 1);
    }

    if (hueco_anterior && hueco_anterior->libre) {
        hueco_anterior->tamanio += hueco_actual->tamanio;
        list_remove(lista_huecos, index_hueco);

        // copio el contenido del hueco actual al anterior
        memcpy(hueco_actual, hueco_anterior, sizeof(t_hueco));
        hueco_actual = (void*) hueco_anterior;
        hueco_anterior = NULL;

        index_hueco--;
        memset(hueco_actual->base, 0, hueco_actual->tamanio);
    }

    if (hueco_siguiente && hueco_siguiente->libre) {
        hueco_actual->tamanio += hueco_siguiente->tamanio;
        list_remove(lista_huecos, index_hueco + 1);
        memset(hueco_actual->base, 0, hueco_actual->tamanio);
        free(hueco_siguiente);
    }
}

void mostrar_tabla_global(){
    for (int i = 0; i < tabla_segmentos_global->elements_count; i++){
        t_tabla_segmentos* ts = list_get(tabla_segmentos_global, i);
        log_info(loggerMemoria, "PID: %d", ts->PID);
        for (int j = 0; j < ts->segmentos->elements_count; j++){
            t_segmento* s = list_get(ts->segmentos, j);
            log_info(loggerMemoria, "Base: %p, Tamanio: %zu, ID: %d", s->direccionBase, s->size, s->id);
        }
    }
}

/*
t_ctx *recibir_contexto(int socket){
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    t_ctx *ctx = deserializar_contexto(buffer, desplazamiento);

    free(desplazamiento);
    free(buffer);
    return ctx;
}

t_ctx *deserializar_contexto(void *buffer, int *desplazamiento){
	t_ctx *ctx = malloc(sizeof(t_ctx));

	// Deserializo PID, PC y cant_instrucciones

	// Aveces ocurre Segment Fault cuando cierras mal o algo así los módulos
	memcpy(&pcb->id_proceso, buffer + *desplazamiento, sizeof(pcb->id_proceso));
	*desplazamiento += sizeof(pcb->id_proceso);
	memcpy(&ctx->program_counter, buffer + *desplazamiento, sizeof(ctx->program_counter));
	*desplazamiento += sizeof(ctx->program_counter);
	memcpy(&ctx->cant_instrucciones, buffer + *desplazamiento, sizeof(ctx->cant_instrucciones));
	*desplazamiento += sizeof(ctx->cant_instrucciones);

	// Deserializo Instrucciones
	ctx->instrucciones = list_create();

	for (int i = 0; i < ctx->cant_instrucciones; i++)
	{
		list_add(ctx->instrucciones, deserealizar_instruccion(buffer, desplazamiento));

	}

	// Deserializo Registros
	ctx->registros = deserealizar_registros(buffer, desplazamiento);


	// deserializo motivos de desalojo
	ctx->motivos_desalojo = deserealizar_motivos_desalojo(buffer, desplazamiento);

	// Deserializo tabla de segmentos
	ctx->tabla_segmentos = deserializar_tabla_segmentos(buffer, desplazamiento);

	return ctx;
}*/
/*
void crear_segmento(PCB *proceso){

    // enviar a memoria CREATE_SEGMENT con sus 2 parametros (id del segmento y tamanio)
    // se solicita la creacion del segmento
    pthread_mutex_lock(&m_memoria);
    t_paquete *paquete = crear_paquete(I_CREATE_SEGMENT);
    //serializar_contexto(proceso->contexto, paquete);
    enviar_pcb(conexionCPU, proceso, I_CREATE_SEGMENT, loggerMemoria);
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);

    int cod_op = recibir_operacion(SOCKET_MEMORIA);

    switch (cod_op){
    case CREATE_SEGMENT:
        int size;
        void *buffer = recibir_buffer(&size, SOCKET_MEMORIA);

        segmento_t *segmento = list_get(proceso->contexto->tabla_segmentos, atoi(proceso->contexto->motivos_desalojo->parametros[0]));

        memcpy(&(segmento->base), buffer, sizeof(segmento->base));
        segmento->tamanio = atoi(proceso->contexto->motivos_desalojo->parametros[1]);
        free(buffer);

        pthread_mutex_unlock(&m_memoria);
        log_info(LOGGER_KERNEL, "PID: <%d> - Crear Segmento - Id: <%d> - Tamaño: <%d>", proceso->contexto->PID, segmento->id_segmento, segmento->tamanio);
        break;
    case COMPACTAR:

        log_info(LOGGER_KERNEL, "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>");
        pthread_mutex_lock(&SOLICITUD_FS);
        log_info(LOGGER_KERNEL, "Inicia la compactacion");

        enviar_compactacion();
        recibir_operacion(SOCKET_MEMORIA);
        recibir_operacion(SOCKET_MEMORIA);


        t_list *tablas_de_segmentos_actualizadas = recibir_todas_las_tablas_segmentos(SOCKET_MEMORIA);
        actualizar_todas_las_tablas_de_segmentos(tablas_de_segmentos_actualizadas);

        log_info(loggerMemoria, "Se finalizó el proceso de compactación");
        pthread_mutex_unlock(&SOLICITUD_FS);
        pthread_mutex_unlock(&m_memoria);
        crear_segmento(proceso);
        break;
    case OUT_OF_MEMORY:
        pthread_mutex_unlock(&m_memoria);
        terminar_proceso(proceso, OUT_OF_MEMORY);
        break;

}
}*/

t_hueco* get_hueco_con_best_fit(int tamanio){
    t_hueco* hueco = NULL;
    int tamanio_hueco = memoriaConfig->TAM_MEMORIA;
    for (int i = 0; i < list_size(lista_huecos); i++) {
        t_hueco* hueco_actual = list_get(lista_huecos, i);
        if (hueco_actual->libre && hueco_actual->tamanio >= tamanio && hueco_actual->tamanio < tamanio_hueco) {
            hueco = hueco_actual;
            tamanio_hueco = hueco_actual->tamanio;
        }
    }
    return hueco;
}

t_hueco* get_hueco_con_worst_fit(int tamanio){
    t_hueco* hueco = NULL;
    int tamanio_hueco = 0;
    for (int i = 0; i < list_size(lista_huecos); i++) {
        t_hueco* hueco_actual = list_get(lista_huecos, i);
        if (hueco_actual->libre && hueco_actual->tamanio >= tamanio && hueco_actual->tamanio > tamanio_hueco) {
            hueco = hueco_actual;
            tamanio_hueco = hueco_actual->tamanio;
        }
    }
    return hueco;
}

bool comprobar_compactacion(int tamanio){
    // sumar el tamaño de todos los hueccos libres
    int tamanio_huecos_libres = 0;
    for (int i = 0; i < list_size(lista_huecos); i++) {
        t_hueco* hueco_actual = list_get(lista_huecos, i);
        if (hueco_actual->libre) {
            tamanio_huecos_libres += hueco_actual->tamanio;
        }
    }

    return tamanio_huecos_libres >= tamanio;
}

t_hueco* get_hueco_con_first_fit(int tamanio){
    t_hueco* hueco = NULL;
    // devuelve el primer hueco que encuentre que este libre
    for (int i = 0; i < list_size(lista_huecos); i++) {
        t_hueco* hueco_actual = list_get(lista_huecos, i);
        if (hueco_actual->libre && hueco_actual->tamanio >= tamanio) {
            hueco = hueco_actual;
            break;
        }
    }
    return hueco;
}

int obtener_index_tabla_segmentos(int PID){
    int index = -1;
    for (int i = 0; i < tabla_segmentos_global->elements_count; i++){
        t_tabla_segmentos* ts = list_get(tabla_segmentos_global, i);
        if (ts->PID == PID){
            index = i;
            break;
        }
    }
    return index;
}

t_paquete* crear_segmento(int id_segmento, int tamanio, PCB* pcb) {
t_hueco *hueco = NULL;
    if (strcmp(memoriaConfig->ALGORITMO_ASIGNACION, "FIRST") == 0)
    {
        hueco = get_hueco_con_first_fit(tamanio);
    }
    else if (strcmp(memoriaConfig->ALGORITMO_ASIGNACION, "BEST") == 0)
    {
        hueco = get_hueco_con_best_fit(tamanio);
    }
    else if (strcmp(memoriaConfig->ALGORITMO_ASIGNACION, "WORST") == 0)
    {
        hueco = get_hueco_con_worst_fit(tamanio);
    }
    else
    {
        log_error(loggerMemoria, "Algoritmo de asignacion no valido");
        return NULL;
    }

    if (!hueco && comprobar_compactacion(tamanio))
    {
        return crear_paquete(COMPACTACION);
    }
    else if (!hueco)
    {
        log_error(loggerMemoria, "No hay hueco disponible para crear el segmento");
        return crear_paquete(OUT_OF_MEMORY);
    }

    modificar_lista_huecos(hueco, tamanio);

    t_segmento *segmento = list_get(pcb->lista_segmentos, id_segmento);

    memcpy(&(segmento->direccionBase), &hueco->base, sizeof(void *));
    memcpy(&(segmento->size), &tamanio, sizeof(int));

    t_tabla_segmentos *ts = malloc(sizeof(t_tabla_segmentos));
    ts->PID = pcb->id_proceso;
    ts->segmentos = pcb->lista_segmentos;

    int index = obtener_index_tabla_segmentos(pcb->id_proceso);

    list_replace_and_destroy_element(tabla_segmentos_global, index, ts, (void *)liberar_tabla_segmentos);
    t_paquete *paquete = crear_paquete(AUX_OK);
    agregar_a_paquete_dato_serializado(paquete, &(segmento->direccionBase), sizeof(segmento->direccionBase));
    log_info(loggerMemoria, "PID: <%d> - Crear Segmento: <%d> - Base: <%p> - TAMAÑO: <%d>", pcb->id_proceso, id_segmento, hueco->base, tamanio);
    return paquete;
}

int obtener_indice_de_lista_huecos(t_hueco* hueco) {
    int index = -1;
    for (int i = 0; i < list_size(lista_huecos); i++) {
        t_hueco* hueco_actual = list_get(lista_huecos, i);
        if (hueco_actual->base == hueco->base) {
            index = i;
            break;
        }
    }

    return index;
}

void modificar_lista_huecos(t_hueco* hueco, int tamanio) {
    // dividir el hueco en 2, uno para el segmento y otro para el hueco restante
    t_hueco* hueco_restante = malloc(sizeof(t_hueco));
    hueco_restante->base = hueco->base + tamanio;
    hueco_restante->tamanio = hueco->tamanio - tamanio;
    hueco_restante->libre = true;

    hueco->tamanio = tamanio;
    hueco->libre = false;

    // usar el espacio en MEMORIA_PRINCIPAL
    memset(hueco->base, 0, tamanio);
    memset(hueco_restante->base, 0, hueco_restante->tamanio);

    // agregar el hueco restante a la lista de huecos despues del hueco actual
    int index_hueco = obtener_indice_de_lista_huecos(hueco);
    list_add_in_index(lista_huecos, index_hueco + 1, hueco_restante);
}

void terminar_proceso(PCB* pcb_para_finalizar, codigo_operacion motivo_finalizacion){

	//log_info(loggerMemoria, "Finaliza el proceso con PID %d - Motivo: %s", pcb_para_finalizar->id_proceso, obtener_motivo(motivo_finalizacion));

	//destruir_pcb(pcb_para_finalizar);

}









// CREACION DE CONFIG Y ESTRUCTURAS
void cargar_config_memoria(t_config* configInicial) {
    memoriaConfig = malloc(sizeof(t_memoria_config));
    memoriaConfig->PUERTO_ESCUCHA = extraer_int_de_config(configInicial, "PUERTO_ESCUCHA", loggerMemoria);
    memoriaConfig->TAM_MEMORIA = extraer_int_de_config(configInicial, "TAM_MEMORIA", loggerMemoria);
    memoriaConfig->TAM_SEGMENTO_0 = extraer_int_de_config(configInicial, "TAM_SEGMENTO_0", loggerMemoria);
    memoriaConfig->CANT_SEGMENTOS = extraer_int_de_config(configInicial, "CANT_SEGMENTOS", loggerMemoria);
    memoriaConfig->RETARDO_MEMORIA = extraer_int_de_config(configInicial, "RETARDO_MEMORIA", loggerMemoria);
    memoriaConfig->RETARDO_COMPACTACION = extraer_int_de_config(configInicial, "RETARDO_COMPACTACION", loggerMemoria);
    memoriaConfig->ALGORITMO_ASIGNACION = extraer_string_de_config(configInicial, "ALGORITMO_ASIGNACION", loggerMemoria);

}

void inicializar_memoria(int tamanio_memoria, int tamanio_segmento_0, char* algoritmo){
	memoria_principal = malloc(memoriaConfig->TAM_MEMORIA);
	tabla_segmentos_global = list_create();
	lista_huecos = list_create();

	t_hueco* hueco_segmento_0 = malloc(sizeof(t_hueco));
	hueco_segmento_0->base = memoria_principal;
	hueco_segmento_0->tamanio = tamanio_segmento_0;
	hueco_segmento_0->libre = false;
	list_add(lista_huecos, hueco_segmento_0);
	log_info(loggerMemoria, "Se creo el hueco del segmento 0 de tamanio %d en la direccion %p", hueco_segmento_0->tamanio, hueco_segmento_0->base);

	t_hueco* hueco_libre_inicial = malloc(sizeof(t_hueco));
	hueco_libre_inicial->base = memoria_principal + tamanio_segmento_0;
	hueco_libre_inicial->tamanio = tamanio_memoria - tamanio_segmento_0;
	hueco_libre_inicial->libre = true;
	list_add(lista_huecos, hueco_libre_inicial);
	log_info(loggerMemoria, "Se creo el hueco libre inicial de tamanio %d en la direccion %p", hueco_libre_inicial->tamanio, hueco_libre_inicial->base);

	return;
}

t_list* crear_tabla_segmentos(){
    t_list* tabla_segmentos = list_create();
    //se me orccure algo, hacer el malloc afuera

    for (int i = 0; i < memoriaConfig->CANT_SEGMENTOS; i++) {
        segmento_t* segmento = malloc(sizeof(segmento_t));
        segmento->id = i;
        segmento->direccion_base = NULL;
        segmento->tamanio_segmento = 0;
        list_add(tabla_segmentos, segmento);
    }

    agregar_segmento_0(tabla_segmentos);
    return tabla_segmentos;
}

void agregar_segmento_0(t_list* tabla_segmentos){
    segmento_t* segmento_0 = malloc(sizeof(segmento_t));
    segmento_0 = list_get(tabla_segmentos, 0);
    segmento_0->direccion_base = memoria_principal;
    segmento_0->tamanio_segmento = memoriaConfig->TAM_SEGMENTO_0;
}

void terminar_programa_memoria(int conexion, t_log* logger, t_config* config){ //TODO: faltaria liberar la conexion
	if (logger) {
		log_destroy(logger);
	}

	if (config){
		config_destroy(config);
	}
}
