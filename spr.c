

#include "spr.h"

void thread_exit(socket_t *sock, dbs_connection_t *dbscon, void *value)
{
	printf ("(servidor)[%d] conexion cerrada con cliente\n\n", sock->fd);
	dbs_free_connection(dbscon);
	socket_destroy(sock);	
	free(value);
}

int retrieve_arguments(int argc, char *argv[], int *spr_port, char **file, int *slr_port, char *slr_ip, char **resource)
{
	char *p, *l, *b;

	if (   (p = get_arg(argc, argv, "-p"))
		&& (l = get_arg(argc, argv, "-l"))
		&& (*resource = get_arg(argc, argv, "-r"))
		&& (*file = get_arg(argc, argv, "-b"))
		&& (b = get_arg(argc, argv, "-q"))
	)
	{
		sscanf( p, "%d", slr_port );
		sscanf( l, "%s", slr_ip );
		sscanf( b, "%d", spr_port);

		return 0;
	}
	return -1;
}

int check_ip(struct sockaddr_in *addr){
	
	char fichero[] = "allowed_ips.txt";
	char linea[STRING_IP_SIZE];	
	/* abrir el fichero de direcciones ip */
	FILE * fich;				
	int ret = 0;

	fich = fopen(fichero,"r");
	if (!fich)
	{
		fprintf(stderr,"No se puede abrir el fichero  '%s'!\n",fichero);
	}
	else
	{
		while(fscanf(fich,"%s\n",linea)!=EOF)				/* mientras no se llegue al final del fichero ... */
		{
			if (strcmp(linea, inet_ntoa (addr->sin_addr))==0)
			{
				ret = 1;
			} 
		}

	}

	fclose(fich);
	return ret;
}

void requesttoparams(char * str, int nparams, char ** params)
{
	int i, x;
	char c;
	for (i = nparams; i < MAX_TOKEN_PARAMS; i++) params[i] = NULL;
	
	if (str != NULL)
	{
		i = 0; x = 0;
		params[x] = str;
		nparams -= 1;
		while( (c = str[i]) != '\0' && x < nparams)
		{
			if (c == ' ' || c == '\n' || c == '\t' )
			{
				str[i] = '\0';
				if ( str[i + 1] != '\0' )
					params[++x] = str + i + 1;
			}
			i++;
		}
	}
	
}

int conection_response(socket_t *sock)
{
	unsigned short op, len;

	op = socket_readushort(sock);
	len = socket_readushort(sock);
	if ( op != OP_SOLICITUD_CONEXION || len > 0 )
    {
        printf("(servidor)[%d] no se ha recibido solicitud de conexion.\n", sock->fd);
        return -1;
    }

    if ( operation_response(sock, OP_CONCESION_CONEXION, SERVER_VERSION) )
    	return -1;

    return 0;
}

int operation_response(socket_t *sock, unsigned short op, char * data)
{
	struct paquete_men * paquete;
    size_t paquete_men_len;

	paquete = paquete_men_create(op, data);
	paquete_men_len = sizeof(struct paquete_men) + ntohs(paquete->len) - 1;

	if ( socket_write( sock, (void *)paquete, paquete_men_len) == -1 )
    {
        printf("(servidor)[%d] no se ha enviado paquete.\n", sock->fd);
        return -1;
    }
    free(paquete);

    return 0;
}

/* Codigo ejecutado por cada hilo */
void *procesar(void *value)
{

	socket_t *socktcp;
	struct thread_args *targs;
	unsigned short op, len;
	char *request, *response;
	int in_session = 0, fatal_error = 0;
	dbs_connection_t *dbscon;
	char *params[MAX_TOKEN_PARAMS];

	pthread_detach(pthread_self());

	targs = (struct thread_args *)value;
    socktcp = socket_init();
    socket_setfd( socktcp, targs->fd );
    socket_settype( socktcp, SOCK_STREAM );
	socket_setrcvtimeout(socktcp, TIMEOUT);
	
	dbscon = dbs_create_connection();

	if ( conection_response( socktcp ) == -1 )
    {
        printf("(servidor)[%d] no se ha concedido conexion.\n", socktcp->fd);
        thread_exit(socktcp, dbscon, value);
		return NULL;
    }
    printf ("(servidor)[%d] concedida solicitud de conexion con cliente [IP %s puerto remoto %d]\n", socktcp->fd, targs->ip, targs->port);
    
    
    do 
    {
	    op = socket_readushort(socktcp);
	    if ( socktcp->expire_timeout )
	    {
	    	thread_exit(socktcp, dbscon, value);
	    	return NULL;
	    }
	    len = socket_readushort(socktcp);
	    if ( socktcp->expire_timeout )
	    {
	    	thread_exit(socktcp, dbscon, value);
	    	return NULL;
	    }
		request = socket_readnchar(socktcp, len);
	    if ( socktcp->expire_timeout )
	    {
	    	thread_exit(socktcp, dbscon, value);
	    	return NULL;
	    }


		printf ("(servidor)[%d] recibida operación [0x%X]: %s\n", socktcp->fd, op, request);

	    if ( !in_session && op != OP_CONECTAR && op != OP_SALIR )
	    {
			printf ("(servidor)[%d] no ha iniciado sesion\n", socktcp->fd);
			response = NOT_SESSION;
	    }
	    else
	    {
			switch(op)
			{
				case OP_CONECTAR:
					if(in_session)
						response = INVALID_OPERATION;
					else
					{
						requesttoparams(request, 2, params);
						if ( (fatal_error = dbs_connect(dbscon, params[0], params[1])) == 0 )
							in_session = 1;								
						else if( fatal_error > 0 )
							fatal_error = 0;

						response = dbscon->result;				
					}
					break;
				case OP_CREAR:
					requesttoparams(request, 5, params);
					if ( (fatal_error = dbs_create(dbscon, params[0], params[1], params[2]
													, params[3], params[4])) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_CAMBIARCLAVE:
					requesttoparams(request, 3, params);
					if ( (fatal_error = dbs_changepass(dbscon, params[0], params[1], params[2])) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_BORRAR:
					requesttoparams(request, 1, params);
					if ( (fatal_error = dbs_delete(dbscon, params[0])) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_BUSCAR:
					requesttoparams(request, 2, params);
					if ( (fatal_error = dbs_search(dbscon, params[0], params[1])) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_LISTAR:
					if ( (fatal_error = dbs_list(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_BLOQUEAR:
					if ( (fatal_error = dbs_block(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_DESBLOQUEAR:
					if ( (fatal_error = dbs_unblock(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_COMMIT:
					if ( (fatal_error = dbs_commit(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_ROLLBACK:
					if ( (fatal_error = dbs_rollback(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				case OP_SALIR:
					if ( (fatal_error = dbs_exit(dbscon)) > 0 )
						fatal_error = 0;

					response = dbscon->result;
					break;
				default:
					printf ("(servidor)[%d] operacion invalida\n", socktcp->fd);
					response = INVALID_OPERATION;
					op = ERROR;
					break;
			}
	    }

		free(request);

		fatal_error = operation_response( socktcp, OP_RESPONSE(op), response );
    }
    while ( op != OP_SALIR && !fatal_error );

	thread_exit(socktcp, dbscon, value);

	return NULL;
}

int main (int argc, char* argv[ ])
{
	int sockfd, flag, my_port, host_port;
	struct thread_args * targs;
	struct sockaddr_in my_addr, their_addr;
	pthread_t hilo;
	socklen_t sin_size;
	char my_ip[STRING_IP_SIZE], host_ip[STRING_IP_SIZE];
	char * resource = NULL, * file = NULL;


	/* Obtine los argumentos */
	if ( retrieve_arguments(argc, argv, &my_port, &file, &host_port, host_ip, &resource) )
	{
		printf("(servidor) el uso debe ser --> spr -q PSPR -b FICHHERO -l SLR -p PSLR -r REC\n");
		return -1;
	}


	/* Obtiene la ip del host local  */
	if( get_my_ip(my_ip) )
	{
		printf("(servidor) no se puede obtener ip local.\n");
		return -1;
	}


	/* El spr registra el recurso en el SLR */
	if ( (register_resource_at_slr( my_port, my_ip, host_port, host_ip, resource )) )
	{
		printf("(servidor) no se ha registrado el recurso en el slr.\n");
		return -1;
	}

	printf("(servidor) recurso %s registrado en el SLR.\n", resource);

	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror ("(servidor) socket");
		exit (1);
	}

	my_addr.sin_family = AF_INET; 			/* Familia: ordenacion de bytes de la maquina */
	my_addr.sin_port = htons (my_port);		/* Puerto: ordenacion de bytes de la red */
	my_addr.sin_addr.s_addr = INADDR_ANY;	/* IP: ordenacion de bytes de la red */
	memset (&(my_addr.sin_zero), '\0', 8);

    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) )
    {
        perror("(servidor) setsockopt");
        exit (1);
    }

	if (bind (sockfd, (void*)&my_addr, sizeof (struct sockaddr_in)) == -1)
	{
		perror ("(servidor) bind");
		exit (1);
	}
	/* escucha peticiones de conexion */
	if (listen (sockfd, BACKLOG) == -1)
	{
		perror ("(servidor) listen");
		exit (1);
	}

	/* Arranco el sevicio de base de datos */
	dbs_start();

	while (1)
	{
		printf ("(servidor) escuchando peticiones de conexion [puerto local %d]\n", ntohs (my_addr.sin_port));

		/* acepta peticion de conexion de un cliente */
		sin_size = sizeof (struct sockaddr_in);
		targs = (struct thread_args *) malloc(sizeof(struct thread_args));
		if ((targs->fd = accept (sockfd, (void*)&their_addr, &sin_size)) == -1)
		{
			free(targs);
			perror ("accept");
			continue;
		}


		memcpy(targs->ip, inet_ntoa (their_addr.sin_addr), STRING_IP_SIZE);
		targs->port = ntohs (their_addr.sin_port);
		printf ("(servidor) conexion establecida desde cliente [IP %s puerto remoto %d]\n",
				inet_ntoa (their_addr.sin_addr), ntohs (their_addr.sin_port));


		/* crea un nuevo thread para atender esta peticion */
		if (pthread_create(&hilo, NULL, procesar, (void*)targs) != 0)
		{
			fprintf(stderr, "(servidor) error creando hilo\n");
			exit(1);
		}

		printf ("(servidor) hilo ejecutado para atender peticion [IP %s puerto remoto %d]\n",
				inet_ntoa (their_addr.sin_addr), ntohs (their_addr.sin_port));

	}

	dbs_stop();
	/* cierra socket (no se ejecuta nunca) */
	close(sockfd);
	return 0;
}







