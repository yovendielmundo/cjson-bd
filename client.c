/*
 * client.c
 *
 *  Created on: 04/07/2012
 */


#include "client.h"

int main (int argc, char *argv[])
{

	char my_ip[STRING_IP_SIZE];
	int my_port = MY_PORT;
	char host_ip[STRING_IP_SIZE];
	int host_port;
	char *resource = NULL, *response;
	socket_t *socket_tcp;
	unsigned short command_op, op, len;
	struct prompt_t prompt;
	int client_error = 0;


	/* Obtener los argumentos */
	if ( retrieve_arguments(argc, argv, &host_port, host_ip, &resource) )
	{
		printf("(cliente) el uso debe ser --> cliente -l SLR -p PSLR -r REC\n");
		return -1;
	}

	/* Obtiene la ip del host local  */
	if( get_my_ip(my_ip) )
	{
		printf("(cliente) no se puede obtener ip local.\n");
		return -1;
	}

	/* Localizacion del recurso en el SLR */
	if ( (locate_resource_from_slr( my_port, my_ip, &host_port, host_ip, resource )) )
	{
		printf("(cliente) no se ha localizado el recurso en el slr.\n");
		return -1;
	}

	// strncpy(host_ip, "192.168.1.34", 15);
	// host_port = 7000;
	printf("(cliente) recurso localizado en servidor SPR [con IP: %s y puerto remoto: %d].\n", host_ip, host_port);

	socket_tcp = socket_create(my_port, my_ip, host_port, host_ip, SOCK_STREAM);
	if ( socket_connect(socket_tcp ) )
	{
		printf("(cliente) connect error.\n");
		return -1;
	}
	/* Establece el timeout de respuestas */
	socket_setrcvtimeout(socket_tcp, TIMEOUT);

	printf ("(cliente) conexion establecida con servidor SPR [con IP: %s y puerto remoto: %d].\n", host_ip, host_port);
	printf ("(cliente) enviando solicitud de conexion.\n");
	if ( conection_request( socket_tcp ) )
	{
		printf("(cliente) solicitud de conexion rechazada.\n");
		return -1;
	}

	prompt_init();

	do
	{
		prompt_read_command(&prompt);
		command_op = cmdtous(prompt.command);
		if ( command_op == OP_UNKNOW )
		{
			printf ("(cliente) orden desconocida.\n");
			continue;
		}

		if( !(client_error = operation_request(socket_tcp, command_op, prompt.parameters)) )
		{
			op = OP_RESPONSE( socket_readushort(socket_tcp) );
			if ( socket_tcp->expire_timeout ) break;
			len = socket_readushort(socket_tcp);
			if ( socket_tcp->expire_timeout ) break;
			response = socket_readnchar(socket_tcp, len);
			if ( socket_tcp->expire_timeout ) break;


			if (op != command_op && op != ERROR)		
			{
				printf ("(cliente) recibida respuesta desconocida.\n");
				client_error = 1;
			}
			else
				prompt_print(NULL, response);

			free(response);
		}
	} 
	while( command_op != OP_SALIR && !client_error );

	prompt_destroy();	
	/* Cierra el socket */
	socket_destroy(socket_tcp);

	printf ("(cliente) conexion cerrada con servidor SPR.\n");

	return(0);
}



int retrieve_arguments(int argc, char *argv[], int *port, char *ip, char **resource)
{
	char *p,*l;

	if (   (p = get_arg(argc, argv, "-p"))
		&& (l = get_arg(argc, argv, "-l"))
		&& (*resource = get_arg(argc, argv, "-r")) )
	{
		sscanf( p, "%d", port );
		sscanf( l, "%s", ip );

		return 0;
	}
	return -1;
}



int conection_request(socket_t * sock)
{
	unsigned short op, len;
	char *data;

	if (operation_request(sock, OP_SOLICITUD_CONEXION, NULL))
		return -1;

    op = socket_readushort(sock);
	len = socket_readushort(sock);
	data = socket_readnchar(sock, len);
	if (op == ERROR || op != OP_CONCESION_CONEXION)
	{
		printf("(cliente) no se ha recibido concesion de conexion [%s]\n", data);
		free(data);
		return -1;
	}
    printf ("%s\n", data);
	free(data);

	return 0;
}

int operation_request(socket_t * sock, unsigned short op, char * data)
{
	struct paquete_men * paquete;
    size_t paquete_men_len;

	paquete = paquete_men_create(op, data);
	paquete_men_len = sizeof(struct paquete_men) + ntohs(paquete->len) - 1;
	if ( socket_write( sock, (void *)paquete, paquete_men_len) == -1 )
    {
        printf("(cliente) no se ha enviado paquete.\n");
        return -1;
    }
    free(paquete);

	return 0;
}

