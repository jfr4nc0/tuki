/*
 * constructor.h
 *
 *  Created on: Apr 27, 2023
 *      Author: utnso
 */

#ifndef CONSTRUCTOR_H_
#define CONSTRUCTOR_H_
#include <stdlib.h>
#define NUEVO(var, type) type *var = malloc(sizeof(type));
#define FREE(var) free(var);

#endif /* CONSTRUCTOR_H_ */
