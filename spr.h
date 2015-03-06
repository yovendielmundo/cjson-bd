/*
 * spr.h
 *
 *  Created on: 04/07/2012
 *      Author: luis
 */


#ifndef SPR_H_
#define SPR_H_

#include "lib/common.h"
#include "lib/socket.h"
#include "lib/slr.h"
#include "dbs/dbs.h"


#define BACKLOG 10 /* numero maximo de conexiones pendientes en cola */
#define TIMEOUT 300 /* segundos de espera para recibir un mensaje del cliente */
#define MAX_TOKEN_PARAMS 5 

#define SERVER_VERSION 			"Versión del servidor: 1.0"
#define INVALID_OPERATION 		"Operacion invalida."
#define NOT_SESSION 			"No ha iniciado session."



/* parametros que pasa el servidor al hilo */
struct thread_args{
	int fd;
	char ip[STRING_IP_SIZE];
	unsigned short port;
};

/**
 *
 */
int conection_response(socket_t *sock);
/**
 *
 */
int operation_response(socket_t *sock, unsigned short op, char * data);



#endif /* SPR_H_ */
