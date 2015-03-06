/*
 *  dbs.h
 *
 *  Created on: 04/08/2012
 */

#ifndef DBS_H_
#define DBS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "table/user_table.h"


typedef struct {
	unsigned int con;
	unsigned int nupdates;
	unsigned int noperations;
	unsigned short user_type;
	double processtime;
	char * result;
	char * buffer;
} dbs_connection_t;

int 	dbs_start();
dbs_connection_t * dbs_create_connection ();
int 	dbs_connect ( dbs_connection_t *con, char *user, char *pass);
int 	dbs_create ( dbs_connection_t *con, char *user, char *email, char *pass, char *user_type, char *name );
int 	dbs_search ( dbs_connection_t *con, char *field, char *pattern );
int 	dbs_delete ( dbs_connection_t *con, char * user );
int 	dbs_changepass ( dbs_connection_t *con, char * user, char *old_pass, char * new_pass );
int 	dbs_list ( dbs_connection_t *con );
int 	dbs_block ( dbs_connection_t *con );
int 	dbs_unblock ( dbs_connection_t *con );
int 	dbs_commit ( dbs_connection_t *con );
int 	dbs_rollback ( dbs_connection_t *con );
int 	dbs_exit ( dbs_connection_t *con );
void 	dbs_free_connection ( dbs_connection_t *con );
void 	dbs_stop();

#define DBS_VERSION			"DBS Ver: 0.1"
#define INVALID_CONNECTION	"Conexion invalida."
#define ALREADY_CONNECT		"Ya se ha echo la conexion."
#define WRONG_USER_PASS		"Usuario o la contraseña incorrectos."
#define NOT_CONNECT			"No se ha echo la conexion."
#define SUCCESS_CONNECT 	"Bienvenido %s."
#define INVALID_PARAMS 		"Parametros incorrectos."
#define WRONG_OPERATION 	"Operación incorrecta."
#define IS_BLOCKED 			"El sistema esta bloqueado por otro usuario."
#define FAIL_CREATE 		"No se ha creado el usuario."
#define FAIL_DELETE			"No se ha borrado el usuario."
#define FAIL_CHANGEPASS		"No se ha cambiado el password."
#define FAIL_COMMIT			"No se ha echo commit."
#define FAIL_ROLLBACK		"No se ha echo rollback."
#define FAIL_SEARCH			"No se ha podido realizar la busqueda."
#define FAIL_FIELD_SEARCH			"Debe indicar un campo valido para la busqueda."
#define FAIL_PATTERN_SEARCH			"Debe indicar una expresion regular valida para la busqueda."
#define USER_EXIST	 		"El login ya esta registrado."
#define USER_NOT_EXIST	 	"El login no esta registrado."
#define EMAIL_EXIST	 		"El email ya esta registrado."
#define INVALID_USERTYPE	"Tipo de usuario incorrecto."
#define NOT_ADMIN_PRIVILEGES	"Se necesitan permisos de administrador."
#define NOT_PRIVILEGES			"Se necesitan permisos."
#define SUCCESS_BLOCKED 	"Sistema bloqueado."
#define SUCCESS_UNBLOCKED 	"Sistema desbloqueado."
#define SUCCESS_CLOSE 		"Tiempo de procesamiento total: %lf segundos\nNúmero total de operaciones: %d\nNúmero total de actualizaciones: %d"
#define OPERATION_OK_UID_SECONDS 	"OK uid: %u\n%.2lf segundos"
#define OPERATION_OK_SECONDS 		"OK\n%.2lf segundos"
#define OPERATION_SECONDS 			"%.2lf segundos"
#define LIST_HEADER		 			"uid|login|nombre|email|clave|permisos\n"
#define LIST_ROW		 			"%u|%s|%s|%s|%s|%s\n"

#define TYPE_ADMIN 		1
#define TYPE_OPERATOR 	2
#define TYPE_USER		3

#define ADMINISTRADOR 	"administrador"
#define OPERADOR 		"operador"
#define USUARIO			"usuario"

#define DBS_CONNECTION_BUFFER_SIZE 4096


#endif /* DBS_H_ */
