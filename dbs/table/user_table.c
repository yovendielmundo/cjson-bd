/*
 *  user_table.c
 *
 *  Created on: 04/08/2012
 */


#include "user_table.h"

#define MAX_ROWS 		1000
 #define TABLE_FILE		"./dbs/table/user_table.db"

user_table_t user_table;
pthread_mutex_t mutex_user_table;

int _load_table ()
{
    FILE * fd;
    char line[256];
    char * ptrs[6];
    user_t * u;

    if((fd = fopen (TABLE_FILE, "r")) == NULL){
    	perror("(dbs) fopen\n");
    	return -1;
    }
        
    
    if (fscanf(fd,"%[^\n]%*c",line) != EOF)
    {
        ptrs[0] = strtok(line, "|");
        ptrs[1] = strtok(NULL, "|");
        ptrs[2] = strtok(NULL, "|");
        ptrs[3] = strtok(NULL, "|");
        ptrs[4] = strtok(NULL, "|");
        ptrs[5] = strtok(NULL, "|");

		u = user_table_create_user((unsigned int)atoi(ptrs[0]), ptrs[1], ptrs[2], ptrs[3], ptrs[4], ptrs[5]);	
		user_table.first = u;
		user_table.last = u;
		user_table.rows = 1;
	}

    while (fscanf(fd,"%[^\n]%*c",line) != EOF)
    {
        ptrs[0] = strtok(line, "|");
        ptrs[1] = strtok(NULL, "|");
        ptrs[2] = strtok(NULL, "|");
        ptrs[3] = strtok(NULL, "|");
        ptrs[4] = strtok(NULL, "|");
        ptrs[5] = strtok(NULL, "|");
	
		u = user_table_create_user((unsigned int)atoi(ptrs[0]), ptrs[1], ptrs[2], ptrs[3], ptrs[4], ptrs[5]);	
		user_table.last->next = u;
		user_table.last = u;
		user_table.rows ++;
	}

	if ( ptrs[0] != NULL )
		user_table.last_uid = (unsigned int) atoi(ptrs[0]);
    
    fclose(fd);

    return 0;
}



int _save_table ()
{
    FILE * fd;
    user_t * u;

    if((fd = fopen (TABLE_FILE, "w")) == NULL)
    {
    	perror("(dbs) fopen\n");
    	return -1;
    }

	u = user_table.first;
	while( u != NULL )
	{
		fprintf(fd, "%u|%s|%s|%s|%s|%s\n", u->uid, u->user, u->name, u->email, u->pass, u->type);
		u = u->next;
	}
		
    fclose(fd);

    return 0;
}

char * _user_field_value(char * field, user_t * user)
{
	char *str = NULL;

	if (strcmp("login", field) == 0)
	{
		str = user->user;
	}
	else if(strcmp("nombre", field) == 0)
	{
		str = user->name;
	}
	else if(strcmp("email", field) == 0)
	{
		str = user->email;
	}

	return str;
}

int user_table_init ()
{
	user_table.rows = 0;
	user_table.last_uid = 1000;
	user_table.first = NULL;
	user_table.last = NULL;

	if( _load_table() )
		user_table_insert("admin", "Administrador", "admin@email.com", "admin", "administrador");

	pthread_mutex_init(&mutex_user_table, NULL);

	return 0;
}

user_t * _user_create( char *user, char *name, char *email, char *pass, char *type)
{
	user_t * u;

	u = (user_t * ) malloc( sizeof(user_t) );
	u->user = (char *) malloc (strlen(user) * sizeof(char));
	u->name = (char *) malloc (strlen(name) * sizeof(char));
	u->email = (char *) malloc (strlen(email) * sizeof(char));
	u->pass = (char *) malloc (strlen(pass) * sizeof(char));
	u->type = (char *) malloc (strlen(type) * sizeof(char));
	
	strcpy(u->user, user);
	strcpy(u->name, name);
	strcpy(u->email, email);
	strcpy(u->pass, pass);
	strcpy(u->type, type);
	u->next = NULL;

	return u;
}

user_t * user_table_create_user ( unsigned int uid, char *user, char *name, char *email, char *pass, char *type )
{
	user_t * u;

	u = _user_create(user, name, email, pass, type);
	u->uid = uid;
	return u;
}

user_t * user_table_insert ( char *user, char *name, char *email, char *pass, char *type )
{
	user_t * u;
	u = _user_create(user, name, email, pass, type);	
	
	pthread_mutex_lock(&mutex_user_table);
	if ( user_table.rows == 0 )
	{
		user_table.first = u;
		user_table.last = u;		
	}
	else
	{
		user_table.last->next = u;
		user_table.last = u;		
	}
	user_table.last_uid++;
	user_table.rows++;
	u->uid = user_table.last_uid;
	pthread_mutex_unlock(&mutex_user_table);

	return u;
}

int user_table_delete ( unsigned int uid ) 
{
	int i, ret = 0;
	user_t * user = NULL, *ptr, *ptrant;

	pthread_mutex_lock(&mutex_user_table);
	if (user_table.rows == 1)
	{
		if ( uid == user_table.first->uid )
		{
			free(user_table.first);
			user_table.first = user_table.last = NULL;
		}
	}
	else if ( user_table.rows > 1 )
	{
		ptrant = user_table.first;
		ptr = user_table.first->next;
		for (i = 1; i < user_table.rows; i++)
		{
			if ( uid == ptr->uid )
			{
				user = ptr;
				break;
			}	
			ptr = ptr->next;
			ptrant = ptr;
		}
		if (user != NULL)
		{
			if ( user_table.last->uid == user->uid )
				user_table.last = ptrant;
			ptrant->next = user->next;
			user_table.rows--;
			free(user);
			ret = 1;
		}
	}
	pthread_mutex_unlock(&mutex_user_table);

	return ret;
}

int user_table_update ( user_t * newuser)
{
	int i, ret = 0;
	user_t * user = NULL, *ptr, *ptrant;

	pthread_mutex_lock(&mutex_user_table);
	if (user_table.rows == 1)
	{
		if ( newuser->uid == user_table.first->uid )
		{
			free(user_table.first);
			user_table.first = user_table.last = newuser;
			ret = 1;
		}
	}
	else if ( user_table.rows > 1 )
	{
		ptrant = user_table.first;
		ptr = user_table.first->next;
		for (i = 1; i < user_table.rows; i++)
		{
			if ( newuser->uid == ptr->uid )
			{
				user = ptr;
				break;
			}	
			ptr = ptr->next;
			ptrant = ptr;
		}
		if (user != NULL)
		{
			newuser->next = user->next;
			ptrant->next = newuser;
			if ( user_table.last->uid == user->uid )
				user_table.last = newuser;
			free(user);
			ret = 1;
		}
	}
	pthread_mutex_unlock(&mutex_user_table);

	return ret;
}

user_t * user_table_getbyuid ( unsigned int uid )
{
	int i;
	user_t * user = NULL, *ptr;

	pthread_mutex_lock(&mutex_user_table);
	ptr = user_table.first;
	for (i = 0; i < user_table.rows; i++)
	{
		if ( uid == ptr->uid )
		{
			user = ptr;
			break;
		}	
		ptr = ptr->next;
	}
	pthread_mutex_unlock(&mutex_user_table);

	return user;
}

user_t * user_table_getbyemail ( char * email )
{
	int i;
	user_t * user = NULL, *ptr;

	pthread_mutex_lock(&mutex_user_table);
	ptr = user_table.first;
	for (i = 0; i < user_table.rows; i++)
	{
		if ( strcmp(email, ptr->email) == 0 )
		{
			user = ptr;
			break;
		}	
		ptr = ptr->next;
	}
	pthread_mutex_unlock(&mutex_user_table);

	return user;
}

user_t * user_table_getbyuser ( char * username )
{
	int i;
	user_t * user = NULL, *ptr;

	pthread_mutex_lock(&mutex_user_table);
	ptr = user_table.first;
	for (i = 0; i < user_table.rows; i++)
	{
		if ( strcmp(username, ptr->user) == 0 )
		{
			user = ptr;
			break;
		}	
		ptr = ptr->next;
	}
	pthread_mutex_unlock(&mutex_user_table);

	return user;
}


user_t ** user_table_findall (int * size)
{
	int i, limit;
	user_t ** users;
	user_t * user;

	users = NULL;
	*size = 0;

	pthread_mutex_lock(&mutex_user_table);
	limit = user_table.rows > MAX_ROWS ? MAX_ROWS : user_table.rows;
	if (limit > 0)
	{
		users = (user_t ** ) malloc( sizeof(user_t *) * limit );		
		user = user_table.first;
		for (i = 0; i < limit; i++)
		{
			users[i] = user;
			user = user->next;
		}
		*size = limit;
	}
	pthread_mutex_unlock(&mutex_user_table);

	return users;
}


user_t ** user_table_findbyfield (char * field, char * pattern, int * size)
{
	int i, limit;
	user_t ** users;
	user_t * user;
	regex_t re;

	users = NULL;
	*size = 0;

	if(regcomp(&re, pattern, REG_EXTENDED) != 0)
	{
		return NULL;
	}

	pthread_mutex_lock(&mutex_user_table);
	limit = user_table.rows > MAX_ROWS ? MAX_ROWS : user_table.rows;
	if (limit > 0)
	{
		users = (user_t ** ) malloc( sizeof(user_t *) * limit );		
		user = user_table.first;
		i = 0;
		if (strcmp("login", field) == 0)
		{
			while (user != NULL && i < limit)
			{
				if(regexec(&re, user->user, 0, NULL, 0) == 0)
				{
					users[i++] = user;
					*size = *size + 1;
				}
				user = user->next;
			}
		}
		else if(strcmp("nombre", field) == 0)
		{
			while (user != NULL && i < limit)
			{
				if(regexec(&re, user->name, 0, NULL, 0) == 0)
				{
					users[i++] = user;
					*size = *size + 1;
				}
				user = user->next;
			}
		}
		else if(strcmp("email", field) == 0)
		{
			while (user != NULL && i < limit)
			{
				if(regexec(&re, user->email, 0, NULL, 0) == 0)
				{
					users[i++] = user;
					*size = *size + 1;
				}
				user = user->next;
			}
		}
		else if(strcmp("permisos", field) == 0)
		{
			while (user != NULL && i < limit)
			{
				if(regexec(&re, user->type, 0, NULL, 0) == 0)
				{
					users[i++] = user;
					*size = *size + 1;
				}
				user = user->next;
			}
		}
		else
		{
			free(users);
			users = NULL;
			*size = -1;
		}
	}
	pthread_mutex_unlock(&mutex_user_table);
	
	regfree(&re);

	return users;
}
int user_table_commit ()
{
	int ret;
	pthread_mutex_lock(&mutex_user_table);
	ret = _save_table();
	pthread_mutex_unlock(&mutex_user_table);

	return ret;
}

int user_table_rollback ()
{
	int ret;
	pthread_mutex_lock(&mutex_user_table);
	ret = _load_table();
	pthread_mutex_unlock(&mutex_user_table);

	return ret;
}

void user_table_free ()
{
	_save_table();
	pthread_mutex_destroy(&mutex_user_table);
}


