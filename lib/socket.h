/*
 * socket.h
 *
 *  Created on: 04/07/2012
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>  /* Linux, FreeBSD */
#include "common.h"


/* PUBLIC */

/**
 *
 */
typedef int socket_type;
typedef struct {
	int fd;
	socket_type type;
	int flag;
	unsigned int nbytes;
	int expire_timeout;
	struct sockaddr_in *my_addr;
	struct sockaddr_in *their_addr;
	void *buffer;
}socket_t;

/**
 * Crea un struct socket_t, inicializa sus campos, crea un socket y hace bind del socket.
 */
socket_t * socket_create( int my_port, char *my_ip, int their_port, char *their_ip, socket_type type);

socket_t * socket_init();

int socket_connect(socket_t *sock);

void socket_setfd( socket_t *sock, int fd );

void socket_settype( socket_t *sock, socket_type type );

int socket_getbytes( socket_t *sock );

int socket_getfd( socket_t *sock );

void socket_setrcvtimeout(socket_t *sock, int seconds );

unsigned short socket_readushort( socket_t *sock );

char * socket_readnchar( socket_t *sock, int n );
/**
 *
 */
int socket_read( socket_t *sock);
/**
 *
 */
int socket_readn( socket_t *sock, size_t nbytes );
/**
 *
 */
int socket_write( socket_t *sock, const void *vptr, size_t nbytes );
/**
 *
 */
int socket_request(socket_t *sock, void * vptr, size_t size);
/**
 *
 */
int socket_requestn(socket_t *sock, void *vptr, size_t size, size_t nbytes);
/**
 *
 */
void socket_destroy(socket_t * sock);

#endif /* SOCKET_H_ */
