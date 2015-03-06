/*
 *  dbs.c
 *
 *  Created on: 04/08/2012
 */


#include "dbs.h"

/* PRIVATE */

pthread_mutex_t mutex_dbs;
unsigned int connection_lock = 0;


void _logupdate(dbs_connection_t *connection);
void _logoperation(dbs_connection_t *connection, time_t start, time_t end, int is_update);
int _check_preconditions(dbs_connection_t *connection);
unsigned short _stype_to_utype(char * user_type);
char * _utype_to_stype(unsigned short user_type);

int dbs_start()
{

    pthread_mutex_init(&mutex_dbs, NULL);
    user_table_init();
    return 0;
}


dbs_connection_t * dbs_create_connection ()
{
    dbs_connection_t * connection;
    connection = (dbs_connection_t *) malloc( sizeof(dbs_connection_t) );
    connection->con = 0;
    connection->nupdates = 0;
    connection->noperations = 0;
    connection->user_type = 0;
    connection->processtime = 0;
    connection->buffer = (char *) malloc( DBS_CONNECTION_BUFFER_SIZE );

    return connection;
}

int dbs_connect (dbs_connection_t *connection, char *user, char *pass)
{
    user_t * u;

    if (connection == NULL){
        connection->result = INVALID_CONNECTION;
        return -1;
    }

    if ( connection_lock != 0 && connection_lock != connection->con)
    {
        connection->result = IS_BLOCKED;
        return 1;      
    }

    if (connection->con > 0)
    {
        connection->result = ALREADY_CONNECT;
        return 1;       
    }

    if(user == NULL || pass == NULL)
    {
        connection->result = INVALID_PARAMS;
        return 1;      
    }

    u = user_table_getbyuser(user);

    if ( u == NULL || strcmp(u->pass, pass) != 0 )
    {
        connection->result = WRONG_USER_PASS;
        return 1;         
    }

    connection->con = u->uid;
    connection->user_type = _stype_to_utype(u->type);
    sprintf(connection->buffer, SUCCESS_CONNECT, user);
    connection->result = connection->buffer;

    return 0;
}

int dbs_create (dbs_connection_t *connection, char *user, char *email, char *pass, char *user_type, char *name)
{
    int ret, i;
    time_t start, end;
    user_t *u;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    if(user == NULL || pass == NULL || email == NULL || pass == NULL || user_type == NULL || name == NULL)
    {
        connection->result = INVALID_PARAMS;
        return 1;
    }

    if ( _stype_to_utype(user_type) == 0 )
    {
        connection->result = INVALID_USERTYPE;
        return 1;
    }

    if ( user_table_getbyuser(user) )
    {
        connection->result = USER_EXIST;
        return 1;
    }

    if ( user_table_getbyemail(email) )
    {
        connection->result = EMAIL_EXIST;
        return 1;
    }

    if ( connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_ADMIN_PRIVILEGES;
        return 1;
    }

    if ( !(u = user_table_insert(user, name, email, pass, user_type)) )
    {
        connection->result = FAIL_CREATE;
        return 1;
    }

    end = time(NULL);
    i = sprintf(connection->buffer, OPERATION_OK_UID_SECONDS, u->uid, difftime(end, start));
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;

    _logoperation(connection, start, end, 1);
	return 0;
}

int dbs_search (dbs_connection_t *connection, char *field, char *pattern)
{
    int ret, users_size, i;
    time_t start,end;
    user_t ** users;
    char * ptr;


    start = time(0);

    if ( (ret = _check_preconditions(connection)) )
        return ret;

    if(field == NULL || pattern == NULL)
    {
        connection->result = INVALID_PARAMS;
        return 1;
    }

    if ( connection->user_type != TYPE_OPERATOR && connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_PRIVILEGES;
        return 1;
    }

    if ( (users = user_table_findbyfield(field, pattern, &users_size)) == NULL)
    {
        connection->result = FAIL_SEARCH;
        if (users_size == 0)
        {
            connection->result = FAIL_PATTERN_SEARCH;
        }
        else if (users_size == -1)
        {
            connection->result = FAIL_FIELD_SEARCH;
        }
        return 1;
    }

    ptr = connection->buffer;
    ptr += sprintf(ptr, LIST_HEADER);

    for (i = 0; i < users_size; i++)
    {
        ptr += sprintf(ptr, LIST_ROW, users[i]->uid, users[i]->user
            , users[i]->name, users[i]->email, users[i]->pass, users[i]->type);
    }

    end = time(NULL);
    ptr += sprintf(ptr, OPERATION_SECONDS, difftime(end, start));
    *ptr = '\0';

    connection->result = connection->buffer;
    free(users);
    _logoperation(connection, start, end, 0);   
	return 0;
}

int dbs_delete (dbs_connection_t *connection, char * username)
{
    int ret, i;
    time_t start, end;
    user_t *user;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    if(username == NULL)
    {
        connection->result = INVALID_PARAMS;
        return 1;
    }

    if ( connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_ADMIN_PRIVILEGES;
        return 1;
    }

    if ( (user = user_table_getbyuser(username)) == NULL )
    {
        connection->result = USER_NOT_EXIST;
        return 1;
    }

    if ( user_table_delete(user->uid) == 0)
    {
        connection->result = FAIL_DELETE;
        return 1;
    }

    end = time(NULL);
    i = sprintf(connection->buffer, OPERATION_OK_SECONDS, difftime(end, start));
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;

    _logoperation(connection, start, end, 1);
	return 0;
}

int dbs_changepass (dbs_connection_t *connection, char * username, char *old_pass, char * new_pass)
{
    int ret, i;
    time_t start, end;
    user_t *user, *newuser;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    if(username == NULL || old_pass == NULL || new_pass == NULL)
    {
        connection->result = INVALID_PARAMS;
        return 1;
    }

    if ( (user = user_table_getbyuser(username)) == NULL )
    {
        connection->result = USER_NOT_EXIST;
        return 1;
    }

    if ( user->uid != connection->con && connection->user_type != TYPE_OPERATOR && connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_PRIVILEGES;
        return 1;
    }

    if ( strcmp(old_pass, new_pass) == 0 )
    {
        connection->result = INVALID_PARAMS;
        return 1;
    }

    newuser = user_table_create_user(user->uid, user->user, user->name, user->email, new_pass, user->type);

    if ( user_table_update(newuser) == 0)
    {
        connection->result = FAIL_CHANGEPASS;
        return 1;
    }

    end = time(NULL);
    i = sprintf(connection->buffer, OPERATION_OK_SECONDS, difftime(end, start));
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;

    _logoperation(connection, start, end, 1);
	return 0;
}


int dbs_list (dbs_connection_t *connection)
{
    int ret, users_size, i;
    time_t start, end;
    user_t ** users;
    char * ptr;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    ptr = connection->buffer;
    ptr += sprintf(ptr, LIST_HEADER);

    users = user_table_findall(&users_size);  
    for (i = 0; i < users_size; i++)
    {
        ptr += sprintf(ptr, LIST_ROW, users[i]->uid, users[i]->user
            , users[i]->name, users[i]->email, users[i]->pass, users[i]->type);
    }

    end = time(NULL);
    ptr += sprintf(ptr, OPERATION_SECONDS, difftime(end, start));
    *ptr = '\0';

    connection->result = connection->buffer;
    free(users);
    _logoperation(connection, start, end, 0);

    return 0;
}

int dbs_block (dbs_connection_t *connection)
{
    int ret;
    time_t start;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    pthread_mutex_lock(&mutex_dbs);
    if ( connection_lock == connection->con)
    {
        connection->result = WRONG_OPERATION;
        pthread_mutex_unlock(&mutex_dbs);
        return 1;
    }
    connection_lock = connection->con;
    pthread_mutex_unlock(&mutex_dbs);

    connection->result = SUCCESS_BLOCKED;
    _logoperation(connection, start, time(NULL), 0);
    return 0;
}

int dbs_unblock (dbs_connection_t *connection)
{
    int ret;
    time_t start;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;


    pthread_mutex_lock(&mutex_dbs);
    if ( connection_lock == 0)
    {
        connection->result = WRONG_OPERATION;
        pthread_mutex_unlock(&mutex_dbs);
        return 1;
    }    
    connection_lock = 0;
    pthread_mutex_unlock(&mutex_dbs);

    connection->result = SUCCESS_UNBLOCKED;
    _logoperation(connection, start, time(NULL), 0);

    return 0;
}

int dbs_commit (dbs_connection_t *connection)
{
    int ret, i;
    time_t start, end;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    if ( connection->user_type != TYPE_OPERATOR && connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_PRIVILEGES;
        return 1;
    }

    if ( user_table_commit() )
    {
        connection->result = FAIL_COMMIT;
        return 1;
    }

    end = time(NULL);
    i = sprintf(connection->buffer, OPERATION_OK_SECONDS, difftime(end, start));
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;

    _logoperation(connection, start, end, 0);
    return 0;
}

int dbs_rollback (dbs_connection_t *connection)
{
    int ret, i;
    time_t start, end;

    start = time(0);

    if ( (ret = _check_preconditions(connection))  )
        return ret;

    if ( connection->user_type != TYPE_OPERATOR && connection->user_type != TYPE_ADMIN )
    {
        connection->result = NOT_PRIVILEGES;
        return 1;
    }

    if ( user_table_rollback() )
    {
        connection->result = FAIL_ROLLBACK;
        return 1;
    }

    end = time(NULL);
    i = sprintf(connection->buffer, OPERATION_OK_SECONDS, difftime(end, start));
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;

    _logoperation(connection, start, end, 0);
    return 0;
}

int dbs_exit (dbs_connection_t *connection)
{
    int i;
 
    if (connection == NULL)
    {
        connection->result = INVALID_CONNECTION;
        return -1;
    }

    if (connection->con == 0)
    {
        connection->result = NOT_CONNECT;
        return 1;       
    }

    i = sprintf(connection->buffer, SUCCESS_CLOSE, connection->processtime, connection->noperations, connection->nupdates);
    connection->buffer[i] = '\0';
    connection->result = connection->buffer;
    return 0;
}

void dbs_free_connection(dbs_connection_t *connection)
{
    free(connection->buffer);
    free(connection);
}

void dbs_stop()
{
    user_table_free();
    pthread_mutex_destroy(&mutex_dbs);
}



void _logoperation(dbs_connection_t *connection, time_t start, time_t end, int is_update)
{
    connection->processtime = connection->processtime + difftime( end, start); 
    connection->noperations ++;
    if (is_update)
        connection->nupdates ++;
}

int _check_preconditions(dbs_connection_t *connection)
{

    if (connection == NULL)
    {
        connection->result = INVALID_CONNECTION;
        return -1;
    }

    pthread_mutex_lock(&mutex_dbs);
    if ( connection_lock != 0 && connection_lock != connection->con)
    {
        connection->result = IS_BLOCKED;
        pthread_mutex_unlock(&mutex_dbs);
        return 1;      
    }
    pthread_mutex_unlock(&mutex_dbs);
    if (connection->con == 0)
    {
        connection->result = NOT_CONNECT;
        return 1;       
    }

    return 0;
}

unsigned short _stype_to_utype(char * user_type)
{
    if ( user_type != NULL )
    {
        if ( strcmp(user_type, ADMINISTRADOR) == 0 )
            return TYPE_ADMIN;

        if ( strcmp(user_type, OPERADOR) == 0 )
            return TYPE_OPERATOR;
        if ( strcmp(user_type, USUARIO) == 0 )
            return TYPE_USER;
    }
    
    return 0;
}

char * _utype_to_stype(unsigned short user_type)
{
    if ( user_type != 0 )
    {
        if ( user_type == TYPE_ADMIN )
            return ADMINISTRADOR;

        if ( user_type == TYPE_OPERATOR )
            return OPERADOR;
        
        if ( user_type == TYPE_USER )
            return USUARIO;
    }
    
    return NULL;
}

