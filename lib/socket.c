/*
 * socket.c
 *
 *  Created on: 04/07/2012
 */


#include "socket.h"

/* PRIVATE */
#define DF 0x0A        /* delimitador de fin de paquete, 1 byte */
#define STRING_IP_SIZE 15
#define MAX_BUFFER_DATA_SIZE 1024


/**
 * lectura para socket 
 */
int __read(socket_t *sock);

/**
 * lectura de nbytes para socket
 */
int __readn(socket_t *sock, size_t n);


/**
 * escritrua para socket
 */
int __write(socket_t *sock, const void *vptr, size_t n);

/**
 * Crea e inicializa un struct sockaddr_in
 */
struct sockaddr_in *__create_sockaddr(int port, char * ip);




/* ==============================================================================================================================  */

socket_t *socket_create(int my_port, char *my_ip, int their_port, char *their_ip, socket_type type)
{
    socket_t *sock;
    sock = (socket_t *) malloc( sizeof(socket_t) );

    sock->type = type;
    sock->flag = 1;
    sock->nbytes = 0;


    /* crea mi addr en el socket_t */
    sock->my_addr = __create_sockaddr(my_port, my_ip);

    /* crea el addr del destino en el socket_t */
    sock->their_addr = __create_sockaddr(their_port, their_ip);

    sock->buffer = (void *) calloc( sizeof(char), MAX_BUFFER_DATA_SIZE );

    /* crea el socket */
    if ((sock->fd = socket(AF_INET, type, 0)) == -1)
    {
        perror ("[ERROR lib/socket.c socket]:");
        return NULL;
    }

    /* Modifica la opcion del socket para evitar "bind: address already in use" */
    if ( setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &sock->flag, sizeof(int)) )
    {
        perror("[ERROR lib/socket.c setsockopt]:");
        return NULL;
    }

    /* asigna el socket datagrama a un puerto local */
    if ( (bind (sock->fd, (void *)sock->my_addr, sizeof (struct sockaddr))) == -1 )
    {
        perror ("[ERROR lib/socket.c bind]:");
        return NULL;
    }

    return sock;

}

socket_t *socket_init()
{
    socket_t *sock;
    sock = (socket_t *) malloc( sizeof(socket_t) );

    sock->flag = 1;
    sock->nbytes = 0;
    sock->expire_timeout = 0;

    sock->buffer = (void *) calloc( sizeof(char), MAX_BUFFER_DATA_SIZE );

    return sock;

}

int socket_connect(socket_t *sock)
{
    if (connect(sock->fd, (struct sockaddr *)sock->their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("[ERROR lib/socket.c connect]");
        return -1;
    }
    return 0;
}

void socket_setfd( socket_t *sock, int fd )
{
    sock->fd = fd;
}

void socket_settype( socket_t *sock, socket_type type )
{
    sock->type = type;
}

int socket_getbytes( socket_t *sock )
{
    return sock->nbytes;
}

int socket_getfd( socket_t *sock )
{
    return sock->fd;
}

unsigned short socket_readushort( socket_t *sock )
{
    unsigned short * ptr;
    if ( (__readn(sock, sizeof(unsigned short))) <= 0 && errno == EAGAIN )
    {
        sock->expire_timeout = 1;
    }
    ptr = (unsigned short *)sock->buffer;
    return ntohs(*ptr);
}

char * socket_readnchar( socket_t *sock, int n )
{
    char * ptr = NULL;
    size_t len = sizeof(char) * n;

    if( 0 < len && len < MAX_BUFFER_DATA_SIZE ){
        ptr = calloc( sizeof(char), n );
        if ( (__readn(sock, len)) <= 0 && errno == EAGAIN )
        {
            sock->expire_timeout = 1;
        }
        memcpy(ptr, sock->buffer, len);
    }

    return ptr;
}

void socket_setrcvtimeout(socket_t *sock, int seconds )
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int socket_read( socket_t *sock )
{
    return __read(sock);
}


int socket_readn( socket_t *sock, size_t nbytes )
{
    return __readn(sock, nbytes);
}

int socket_write( socket_t *sock, const void *vptr, size_t nbytes )
{
    return __write(sock, vptr, nbytes);
}

int socket_request(socket_t *sock, void *vptr, size_t size)
{

    if ( socket_write( sock, vptr, size ) == -1 )
    {
        printf("[ERROR lib/socket.c]: no se ha enviado paquete.\n");
        return -1;
    }

    if ( socket_read( sock ) == -1 )
    {
        printf("[ERROR lib/socket.c]: no se ha recibido paquete.\n");
        return -1;
    }

    return 0;
}


int socket_requestn(socket_t *sock, void *vptr, size_t size, size_t nbytes)
{

    if ( socket_write( sock, vptr, size ) == -1 )
    {
        printf("[ERROR lib/socket.c]: no se ha enviado paquete.\n");
        return -1;
    }

    if ( socket_readn( sock, nbytes ) == -1 )
    {
        printf("[ERROR lib/socket.c]: no se ha recibido paquete.\n");
        return -1;
    }

    return 0;
}

void socket_destroy(socket_t *sock)
{
    close(sock->fd);
    free(sock->their_addr);
    free(sock->my_addr);
    free(sock->buffer);
    free(sock);
}




int __read(socket_t *sock)
{
    size_t nleft;
    ssize_t nread;
    socklen_t addrlen;
    char *ptr;

    memset(sock->buffer, '\0', MAX_BUFFER_DATA_SIZE);
    ptr = sock->buffer;
    addrlen = sizeof(struct sockaddr_in);
    nleft = MAX_BUFFER_DATA_SIZE;
    nread = 0;
    do
    {
        if (sock->type == SOCK_DGRAM)
            nread = recvfrom(sock->fd, ptr, nleft, 0, (void *)sock->their_addr, &addrlen);
        else if(sock->type == SOCK_STREAM)
            nread = read(sock->fd, ptr, nleft);

        if ( nread < 0)
        {
            sock->nbytes = -1;
            return (-1);
        }
        else if ( nread == 0 )
            break;

        nleft -= nread;
        ptr += nread;

    }while ((nleft > 0) && (ptr[-1] != DF));

    sock->nbytes = MAX_BUFFER_DATA_SIZE - nleft;

    return 0;
}


int __readn(socket_t *sock, size_t n)
{
    size_t nleft;
    socklen_t addrlen;
    ssize_t nread;
    char *ptr;

    if(n > MAX_BUFFER_DATA_SIZE){
        printf("[ERROR lib/socket.c]: imposible leer %d bytes.\n",(int) n);
        return -1;
    }
    memset(sock->buffer, '\0', MAX_BUFFER_DATA_SIZE);
    ptr = sock->buffer;
    addrlen = sizeof(struct sockaddr_in);
    nread = 0;
    nleft = n;
    while (nleft > 0)
    {
        if (sock->type == SOCK_DGRAM)
            nread = recvfrom(sock->fd, ptr, nleft, 0, (void *)sock->their_addr, &addrlen);
        else if(sock->type == SOCK_STREAM)
            nread = read(sock->fd, ptr, nleft);

        if ( nread < 0)
        {
            sock->nbytes = -1;
            return (-1);
        }
        else if ( nread == 0 )
            break;

        nleft -= nread;
        ptr += nread;
    }

    
    return (n - nleft);
}



int __write(socket_t *sock, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten = 0;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0)
    {
        if (sock->type == SOCK_DGRAM)
        {
            nwritten = sendto(sock->fd, ptr, nleft, 0, (void *)sock->their_addr, sizeof(struct sockaddr_in));
        }
        else if(sock->type == SOCK_STREAM)
        {
            nwritten = write(sock->fd, ptr, nleft);
        }

        if ( nwritten < 0)
            return (-1);
        else if ( nwritten == 0 )
            break;

        nleft -= nwritten;
        ptr += nwritten;
    }
    sock->nbytes = n;
    return 0;
}

struct sockaddr_in *__create_sockaddr(int port, char * ip)
{
    struct sockaddr_in *addr;
    addr = (struct sockaddr_in *) malloc( sizeof(struct sockaddr_in) );
    addr->sin_family = AF_INET;                               /* Familia: ordenación de bytes de la máquina */
    addr->sin_port = htons( port );                     /* Puerto: ordenación de bytes de la red */
    addr->sin_addr.s_addr = inet_addr( ip );            /* IP: ordenación de bytes de la red */
    memset (&(addr->sin_zero), '\0', 8);

    return addr;
}
