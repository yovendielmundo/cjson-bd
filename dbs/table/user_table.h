
/*
 *  user_table.h
 *
 *  Created on: 04/08/2012
 */

#ifndef USER_TABLE_H_
#define USER_TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <regex.h>

typedef struct user {
	unsigned int uid;
	char *user;
	char *name;
	char *email;
	char *pass;
	char *type;
	struct user *next;
}user_t;

typedef struct user_table {
		unsigned int rows;
		unsigned int last_uid;
		user_t *first;
		user_t *last;
}user_table_t;

int 		user_table_init ();
user_t * 	user_table_insert ( char *user, char *name, char *email, char *pass, char *type );
user_t * 	user_table_create_user ( unsigned int uid,  char *user, char *name, char *email, char *pass, char *type );
int 		user_table_delete ( unsigned int uid) ;
int 		user_table_update ( user_t * user );
user_t * 	user_table_getbyuid ( unsigned int uid );
user_t * 	user_table_getbyemail ( char * email );
user_t * 	user_table_getbyuser ( char * user );
user_t ** 	user_table_findall (int * size);
user_t ** 	user_table_findbyfield (char * field, char * pattern, int * size);
int 		user_table_commit ();
int 		user_table_rollback ();
void 		user_table_free ();


#endif /* USER_TABLE_H_ */