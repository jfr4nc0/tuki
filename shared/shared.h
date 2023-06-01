#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
//#include <commons/bitarray.h>
#include <commons/config.h>

#include <sys/stat.h>
#include <pthread.h> 	// Para crear hilos! :D
#include <semaphore.h>	// Para crear semaforos! :D
#include <time.h>
#include <math.h>

/*---------------------------------- INSTRUCTIONS ----------------------------------*/

#define BADKEY -1
#define I_SET 1
#define I_MOV_IN 2
#define I_MOV_OUT 3
#define I_IO 4
#define I_F_OPEN 5
#define I_F_CLOSE 6
#define I_F_SEEK 7
#define I_F_READ 8
#define I_F_WRITE 9
#define I_TRUNCATE 10
#define I_WAIT 11
#define I_SIGNAL 12
#define I_CREATE_SEGMENT 13
#define I_DELETE_SEGMENT 14
#define I_YIELD 15
#define I_EXIT 16

int keyfromstring(char *key);

