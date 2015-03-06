/*
 * client.h
 *
 *  Created on: 04/07/2012
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "lib/common.h"
#include "lib/socket.h"
#include "lib/slr.h"
#include "lib/prompt.h"



#define MY_PORT		3490
#define TIMEOUT 	30 /* segundos de espera para recibir una respuesta del servidor */

int retrieve_arguments(int argc, char *argv[], int *port, char *ip, char **resource);
/**
 *
 */
int conection_request(socket_t * sock);
/**
 *
 */
int operation_request(socket_t * sock, unsigned short op, char * data);


#endif /* CLIENT_H_ */
