/*
 * common.h
 *
 *  Created on: 04/07/2012
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <math.h>

#define STRING_IP_SIZE 15
#define STRING_DNI_SIZE 9
// /* declaración de constantes */

// #define BACKLOG 		10     		/* numero maximo de conexiones pendientes en cola */


#define PACKAGE_HEADER_SIZE		(sizeof(unsigned short)+sizeof(unsigned int))		/* longitud de la cabecera del paquete d, 6 bytes */


#define DNI1			"23285364A"	/* dni del alumno 1 */
#define DNI2			"00000000X"	/* dni del alumno 2 */


/* codigos de operacion */
#define OP_PETICION_ID		0x0001
#define OP_PETICION_REG		0x0002
#define OP_CONCESION_ID		0x0003
#define OP_CONCESION_REG	0x0004
#define OP_PETICION_REC		0x0005
#define OP_LOCALIZADOR_REC	0x0006

#define OP_SOLICITUD_CONEXION	0x0007
#define OP_CONCESION_CONEXION	0x0008

#define OP_CONECTAR		0x0009
#define OP_BLOQUEAR		0x000B
#define OP_BORRAR		0x000C
#define OP_BUSCAR		0x000D
#define OP_CAMBIARCLAVE		0x000E
#define OP_CREAR 		0x0010
#define OP_DESBLOQUEAR	0x0011
#define OP_LISTAR		0x0012
#define OP_SALIR		0x0013
#define OP_UNKNOW		0x0014
#define OP_COMMIT		0x0015
#define OP_ROLLBACK		0x0016
#define OP_MASK_XOR		0x8000
#define OP_RESPONSE(x)	((x) ^ (OP_MASK_XOR))	

#define DF 				0x0A	
#define	ERROR			0xFFFF


// int bloqueo_acceso;
// pthread_mutex_t mutex_bloqueo, mutex_fichero;
// Lista *lista;

struct paquete_datos
{
	unsigned short op;		/* codigo de operacion, 2 bytes */
	unsigned int id;		/* identificador, 4 bytes */
	char data;				/* datos, 1 < data < max bytes */
}__attribute__((packed));

struct paquete_men
{
	unsigned short op;		/* codigo de operacion, 2 bytes */
	unsigned short len;		/* identificador, 4 bytes */
	char data;				/* datos, 1 < data < max bytes */
}__attribute__((packed));


struct paquete_id
{
	unsigned short op;				/* codigo de operacion, 2 bytes */
	char d1[STRING_DNI_SIZE];		/* dni, 9 bytes */
	char d2[STRING_DNI_SIZE];		/* dni, 9 bytes */
	char ip[STRING_IP_SIZE];		/* ip, 15 bytes */
	char data;						/* datos, 1 < data < max bytes */
}__attribute__((packed));

struct paquete_recurso
{
	unsigned short op;				/* codigo de operacion, 2 bytes */
	unsigned int id;
	char ip[STRING_IP_SIZE];		/* ip, 15 bytes */
	unsigned short port;
	char data;						/* datos, 1 < data < max bytes */
}__attribute__((packed));

char *get_arg(int argc, char *argv[], const char *name);

int get_my_ip (char *host);

char *get_date ();

struct paquete_datos *paquete_datos_create(unsigned short op, unsigned int id, char *data, size_t *package_len);

struct paquete_id *paquete_id_create(char *my_ip, size_t *package_len);

struct paquete_recurso *paquete_recurso_create(unsigned int id, char *my_ip, unsigned short my_port,char *resource, size_t *package_len);

struct paquete_men *paquete_men_create( unsigned short op, char * data);

unsigned short cmdtous(char * str);


#endif /* COMMON_H_ */
