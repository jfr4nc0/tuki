#include "estructuras.h"

t_superbloque* superbloque;

void inicializar_estructuras(t_config* config)
{
    cargar_config(config);
    superbloque = crear_superbloque(configFileSystem->PATH_SUPERBLOQUE);

    abrir_bitmap(configFileSystem->PATH_BITMAP, superbloque->block_count);

    crear_archivo_de_bloques(configFileSystem->PATH_BLOQUES, superbloque->block_count, superbloque->block_size);

    abrir_fcbs(configFileSystem->PATH_FCB);
}


void cargar_config(t_config* config) {
    configFileSystem = malloc(sizeof(t_config_file_system));
   	configFileSystem->IP_MEMORIA = extraer_string_de_config(config, "IP_MEMORIA", loggerFileSystem);
	configFileSystem->PATH_SUPERBLOQUE = extraer_string_de_config(config, "PATH_SUPERBLOQUE", loggerFileSystem);
	configFileSystem->PATH_BITMAP = extraer_string_de_config(config, "PATH_BITMAP", loggerFileSystem);
	configFileSystem->PATH_BLOQUES = extraer_string_de_config(config, "PATH_BLOQUES", loggerFileSystem);
	configFileSystem->PATH_FCB = extraer_string_de_config(config, "PATH_FCB", loggerFileSystem);

	configFileSystem->RETARDO_ACCESO_BLOQUE = extraer_int_de_config(config, "RETARDO_ACCESO_BLOQUE", loggerFileSystem);
	configFileSystem->PUERTO_MEMORIA = extraer_int_de_config(config, "PUERTO_MEMORIA", loggerFileSystem);
	configFileSystem->PUERTO_ESCUCHA = extraer_int_de_config(config, "PUERTO_ESCUCHA", loggerFileSystem);

    return;
}


t_superbloque* crear_superbloque(char *pathSuperbloque) {
    t_superbloque *superbloque = malloc(sizeof(*superbloque));

    t_config* config = config_create(configFileSystem->PATH_SUPERBLOQUE);

    superbloque->block_size = (uint32_t) config_get_int_value(config, "BLOCK_SIZE");
    superbloque->block_count = (uint32_t) config_get_int_value(config, "BLOCK_COUNT");

    config_destroy(config);
    return superbloque;
}



void abrir_bitmap(char* pathBitmap, uint32_t blockCount) {
    bitmap = malloc(sizeof(t_bitmap));

    uint32_t fileDescriptor = open(pathBitmap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(fileDescriptor == -1) {
        log_error(loggerFileSystem,"Error al abrir el archivo Bitmap");
    }

    bitmap->tamanio =(blockCount / 8);
    if(ftruncate(fileDescriptor, bitmap->tamanio) == -1) {
        log_error(loggerFileSystem,"Error al truncar el archivo Bitmap");
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor,0);
    if(bitmap->direccion == MAP_FAILED) {
        log_error(loggerFileSystem,"Error al mapear el Bitmap");
    }

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->tamanio, LSB_FIRST);

    close(fileDescriptor);
}

void destruir_bitmap() {
    munmap(bitmap->direccion, bitmap->tamanio);
    bitarray_destroy(bitmap->bitarray);
}


FILE* abrir_archivo_de_bloques(char* pathArchivoDeBloques) {
    archivoDeBloques = fopen(pathArchivoDeBloques, "r+b");

    if(archivoDeBloques == NULL) {
        log_error(loggerFileSystem, "No se pudo abrir el archivo.");
    }
    return archivoDeBloques;
}

void crear_archivo_de_bloques(char* pathArchivoDeBloques, uint32_t blockCount, uint32_t blockSize) {
    uint32_t fileDescriptor = open(pathArchivoDeBloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(fileDescriptor == -1) {
        log_error(loggerFileSystem,"Error al abrir el Archivo de Bloques");
    }

    uint32_t tamanioArchivoDeBloques = blockCount * blockSize;
    if(ftruncate(fileDescriptor, tamanioArchivoDeBloques) == -1) {
        log_error(loggerFileSystem,"Error al truncar el Archivo de Bloques");
    }

    close(fileDescriptor);
}


void abrir_fcbs(char* path_fcbs) {

}
