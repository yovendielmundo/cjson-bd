
#include "slr.h"


int __slr_request(socket_t *sock, void *request_packet, size_t request_packet_len, unsigned short op_response)
{
	struct paquete_datos *response;

	if ( socket_request( sock, request_packet, request_packet_len )  )
	{
		printf("[ERROR slr.c]:\t\t no se ha realizado peticion.\n");
		return -1;
	}

	response = (struct paquete_datos *) sock->buffer;
	response->op = ntohs(response->op), response->id = ntohl(response->id);

	if (response->op == ERROR)
	{
		printf("[AVISO slr.c]:\t\t mensage de error: %s", (char *)&response->data);
		return -1;
	}
	else if ( (sock->nbytes < sizeof(struct paquete_datos)) || response->op != op_response )
	{
		printf("[ERROR slr.c]:\t\t se ha recibido paquete corrupto.\n");
		return -1;
	} 

	return 0;
}

/**
 * El cliente localiza el recurso en el slr.
 */
int locate_resource_from_slr( int my_port, char *my_ip,  int *host_port, char *host_ip, char *resource )
{
	socket_t * socket_udp;
	struct paquete_id *peticion_id;
	struct paquete_datos *peticion_rec;
	struct paquete_datos *response;
	struct paquete_recurso *response_rec;
	size_t request_packet_len;

	/*
	 * Peticion de id
	 */
	if ( (socket_udp = socket_create(my_port, my_ip, *host_port, host_ip, SOCK_DGRAM)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el socket datagrama.\n");
		return -1;
	}



	if ( (peticion_id = paquete_id_create(my_ip, &request_packet_len)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el paquete peticion id.\n");
		return -1;
	}

	if ( __slr_request(socket_udp, (void *)peticion_id, request_packet_len, OP_CONCESION_ID) )
	{
		printf("[ERROR slr.c]:\t\t no se ha realizado la concesión id.\n");
		return -1;
	}

	response = (struct paquete_datos *) socket_udp->buffer;
	printf("[AVISO slr.c]:\t\t peticion de id realizada op:%d\tid:%d\tmen:%s", response->op, response->id, (char *)&response->data);

	

	/*
	 * Peticion de recurso
	 */
	if ( (peticion_rec = paquete_datos_create(OP_PETICION_REC, response->id, resource, &request_packet_len)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el paquete peticion de recurso.\n");
		return -1;
	}

	if ( __slr_request(socket_udp, (void *)peticion_rec, request_packet_len, OP_LOCALIZADOR_REC) )
	{
		response = (struct paquete_datos *) socket_udp->buffer;
		if (response->op != ERROR)
		{
			printf("[ERROR slr.c]:\t\t no se ha realizado localizacion recurso.\n");
		}
		return -1;

	}

	response_rec = (struct paquete_recurso *) socket_udp->buffer;

	*host_port = ntohs(response_rec->port);
	strncpy(host_ip, response_rec->ip, STRING_IP_SIZE);

	socket_destroy( socket_udp );
	return 0;
}
/**
 * El spr registra un recurso en el slr.
 */
int register_resource_at_slr( int my_port, char *my_ip,  int host_port, char *host_ip, char *resource )
{
	socket_t * socket_udp;
	struct paquete_id *peticion_id;
	struct paquete_recurso *peticion_reg;
	struct paquete_datos *response;
	size_t request_packet_len;
	int udp_port = 3490;

	

	/*
	 * Peticion de id
	 */
	if ( (socket_udp = socket_create(udp_port, my_ip, host_port, host_ip, SOCK_DGRAM)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el socket datagrama.\n");
		return -1;
	}


	if ( (peticion_id = paquete_id_create(my_ip, &request_packet_len)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el paquete peticion id.\n");
		return -1;
	}

	if ( __slr_request(socket_udp, (void *)peticion_id, request_packet_len, OP_CONCESION_ID) )
	{
		printf("[ERROR slr.c]:\t\t no se ha realizado la concesión id.\n");
		return -1;
	}

	response = (struct paquete_datos *) socket_udp->buffer;
	printf("[AVISO slr.c]:\t\t peticion de id realizada op:%d\tid:%d\tmen:%s", response->op, response->id, (char *)&response->data);



	/*
	 * Peticion de registro
	 */
	if ( (peticion_reg = paquete_recurso_create(response->id, my_ip, my_port, resource, &request_packet_len)) == NULL )
	{
		printf("[ERROR slr.c]:\t\t no se ha creado el paquete registro de recurso.\n");
		return -1;
	}

	if ( __slr_request(socket_udp, (void *)peticion_reg, request_packet_len, OP_CONCESION_REG) )
	{
		response = (struct paquete_datos *) socket_udp->buffer;
		if (response->op != ERROR)
		{
			printf("[ERROR slr.c]:\t\t no se ha realizado el registro del recurso.\n");
			return -1;
		}

	}

	socket_destroy( socket_udp );
	return 0;
}
