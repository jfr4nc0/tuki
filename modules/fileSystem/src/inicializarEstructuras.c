#include "inicializarEstructuras.h"
#include <unistd.h>

t_superbloque* superbloque;
t_config_file_system* configFileSystem;
t_bitmap* bitmap;
t_dictionary* dictionaryFcbs;
uint32_t SIZE_BLOQUE;
uint32_t CANTIDAD_BLOQUES;

/*
* Funciona como un main, usa todas las funciones que estan debajo
*/
void inicializar_estructuras(t_config* config) {
    configFileSystem = cargar_config(config);
    superbloque = crear_superbloque(configFileSystem->PATH_SUPERBLOQUE);

    SIZE_BLOQUE = superbloque->block_size;
    CANTIDAD_BLOQUES = superbloque->block_count;

    abrir_bitmap(configFileSystem->PATH_BITMAP, superbloque->block_count);

    crear_archivo_de_bloques(configFileSystem->PATH_BLOQUES, superbloque->block_count, superbloque->block_size);

    abrir_fcbs(configFileSystem->PATH_FCB);

    return;
}

t_config_file_system* cargar_config(t_config* config) {
    t_config_file_system* configPuntero = malloc(sizeof(t_config_file_system));
   	configPuntero->IP_MEMORIA = extraer_string_de_config(config, "IP_MEMORIA", loggerFileSystem);
	configPuntero->PATH_SUPERBLOQUE = extraer_string_de_config(config, "PATH_SUPERBLOQUE", loggerFileSystem);
    // configPuntero->PATH_SUPERBLOQUE = "./tuki-pruebas/prueba-filesystem/superbloque.dat";
	configPuntero->PATH_BITMAP = extraer_string_de_config(config, "PATH_BITMAP", loggerFileSystem);
	configPuntero->PATH_BLOQUES = extraer_string_de_config(config, "PATH_BLOQUES", loggerFileSystem);
	configPuntero->PATH_FCB = extraer_string_de_config(config, "PATH_FCB", loggerFileSystem);

	configPuntero->RETARDO_ACCESO_BLOQUE = extraer_int_de_config(config, "RETARDO_ACCESO_BLOQUE", loggerFileSystem) / 1000;
	configPuntero->PUERTO_MEMORIA = extraer_int_de_config(config, "PUERTO_MEMORIA", loggerFileSystem);
	configPuntero->PUERTO_ESCUCHA = extraer_int_de_config(config, "PUERTO_ESCUCHA", loggerFileSystem);

    return configPuntero;
}

t_superbloque* crear_superbloque(char *pathSuperbloque) {
    t_superbloque *superbloque = malloc(sizeof(*superbloque));

    t_config* config = config_create(pathSuperbloque);

    superbloque->block_size = (uint32_t) extraer_int_de_config(config, "BLOCK_SIZE", loggerFileSystem);
    superbloque->block_count = (uint32_t) extraer_int_de_config(config, "BLOCK_COUNT", loggerFileSystem);

    config_destroy(config);

    return superbloque;
}


void abrir_bitmap(char* pathBitmap, uint32_t blockCount) {
    bitmap = malloc(sizeof(t_bitmap));

    uint32_t fileDescriptor = open(pathBitmap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(fileDescriptor == -1) {
        log_error(loggerFileSystem, "Error al abrir el archivo Bitmap");
    }

    bitmap->size =(blockCount / 8);
    if(ftruncate(fileDescriptor, bitmap->size) == -1) {
        log_error(loggerFileSystem, "Error al truncar el archivo Bitmap");
    }

    bitmap->direccion = mmap(NULL, bitmap->size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor,0);
    if(bitmap->direccion == MAP_FAILED) {
        log_error(loggerFileSystem, "Error al mapear el Bitmap");
    }

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->size, LSB_FIRST);

    close(fileDescriptor);

    log_debug(loggerFileSystem, "bitmap inicializado");

    return;
}

void destruir_bitmap() {
    munmap(bitmap->direccion, bitmap->size);
    bitarray_destroy(bitmap->bitarray);

    return;
}


FILE* abrir_archivo_de_bloques(char* pathArchivoDeBloques) {
    FILE* archivo = fopen(pathArchivoDeBloques, "r+b");

    if(archivo == NULL) {
        log_error(loggerFileSystem, "No se pudo abrir el archivo.");
        return NULL;
    }

    return archivo;
}

void crear_archivo_de_bloques(char* pathArchivoDeBloques, uint32_t blockCount, uint32_t blockSize) {
    uint32_t fileDescriptor = open(pathArchivoDeBloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(fileDescriptor == -1) {
        log_error(loggerFileSystem, "Error al abrir el Archivo de Bloques");
        return;
    }

    uint32_t tamanioArchivoDeBloques = blockCount * blockSize;
    if(ftruncate(fileDescriptor, tamanioArchivoDeBloques) == -1) {
        log_error(loggerFileSystem, "Error al truncar el Archivo de Bloques");
        return;
    }

    close(fileDescriptor);
    log_debug(loggerFileSystem, "Archivo de bloques inicializado");
    return;
}


void abrir_fcbs(char* path_fcbs) {
	DIR *dir;
	struct dirent *ent;
	char rutaFcb[PATH_MAX];
	t_fcb* fcb_temp;
	char *nombreTemp;

	dir = opendir(path_fcbs);
	if (dir == NULL) {
		log_error(loggerFileSystem , ERROR_ABRIR_ARCHIVO, path_fcbs);
		return;
	}

	dictionaryFcbs = dictionary_create();
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_type == DT_REG) {
            // Ruta completa del archivo es el nombre del archivo + el path hacia la ruta de los fcb
			snprintf(rutaFcb, sizeof(rutaFcb), "%s/%s", path_fcbs, ent->d_name);
			fcb_temp = cargar_fcb(rutaFcb);
			dictionary_put(dictionaryFcbs, fcb_temp->nombre_archivo, (void*)fcb_temp);
		} else {
			log_warning(loggerFileSystem, "FCB: EL ent->d_type (%d) no es %d", ent->d_type, DT_REG);
		}
	}
	closedir(dir);
	log_debug(loggerFileSystem, "Lista de fcbs inicializada.");
}

t_fcb* cargar_fcb(char *pathFcb) {
    t_config* config_fcb = config_create(pathFcb);
    t_fcb* fcb = malloc(sizeof(t_fcb));

    fcb->nombre_archivo = extraer_string_de_config(config_fcb, "NOMBRE_ARCHIVO", loggerFileSystem);
    fcb->tamanio_archivo = (uint32_t) extraer_int_de_config(config_fcb, "TAMANIO_ARCHIVO", loggerFileSystem);
    fcb->puntero_directo = (uint32_t) extraer_int_de_config(config_fcb, "PUNTERO_DIRECTO", loggerFileSystem);
    fcb->puntero_indirecto = (uint32_t) extraer_int_de_config(config_fcb, "PUNTERO_INDIRECTO", loggerFileSystem);
    fcb->cantidad_bloques_asignados = dividir_por_size_bloque_y_redondear_hacia(fcb->tamanio_archivo, ARRIBA);

    return fcb;
}

uint32_t dividir_por_size_bloque_y_redondear_hacia(uint32_t nuevoSize, codigo_redondear ABAJO_ARRIBA) {
    uint32_t cantidadBLoques = nuevoSize / SIZE_BLOQUE;

    cantidadBLoques = (nuevoSize%SIZE_BLOQUE == 0) ? cantidadBLoques : cantidadBLoques + ABAJO_ARRIBA;

    return cantidadBLoques;
}
