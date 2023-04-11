#ifndef KERNEL_H_
	#define KERNEL_H_

	#include<stdio.h>
	#include<stdlib.h>
	#include<string.h>
	#include<sys/socket.h>
	#include<unistd.h>
	#include<netdb.h>
	#include<commons/log.h>
	#include<commons/collections/list.h>
	#include<assert.h>

	#include "utils.h"
	#include "../../shared/src/funcionesServidor.c"

	void iterator(char* value);

	extern t_log* logger;

#endif /* SERVER_H_ */
