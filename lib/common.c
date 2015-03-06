/*
 * common.c
 *
 *  Created on: 04/07/2012
 */

#include "common.h"


char *get_arg(int argc, char *argv[], const char *name)
{
    int i;
    for (i = 1; i < argc; ++i)
        if ( strcmp(argv[i], name) == 0 )
            return argv[i+1];

    return NULL;
}

/* Devuelve la fecha/hora actual en formato texto */
char *get_date()
{
    time_t timestamp;
    time(&timestamp);
    return ctime(&timestamp);
}

/**
 *
 * btiene la IP del host local
 */
int get_my_ip(char *my_ip)
{
    struct ifaddrs* ifa;
    struct ifaddrs* ifa_tmp;
    int num_interfaz = 0;

    if (getifaddrs (&ifa))
    {
        return -1;
    }

    ifa_tmp = ifa;
    do
    {
    /* Bridges have no addr. */
        if (ifa_tmp->ifa_addr && ifa_tmp->ifa_addr->sa_family == AF_INET)
        {
            num_interfaz ++;
            if (num_interfaz == 2)
            {
                strcpy(my_ip,inet_ntoa( ((struct sockaddr_in*)(ifa_tmp->ifa_addr))->sin_addr));
                break;
            }
        }
        ifa_tmp = ifa_tmp->ifa_next;
    }
    while (ifa_tmp);

    /* free memory */
    freeifaddrs (ifa);


    return 0;
}



struct paquete_datos *paquete_datos_create(unsigned short op, unsigned int id, char *data, size_t *package_len)
{

    size_t paquete_datos_len, data_len;
    struct paquete_datos *package_ptr;
    char * ptr;

    data_len = strlen(data);
    paquete_datos_len = sizeof(struct paquete_datos);
    *package_len = paquete_datos_len + data_len;
    
    if ( (package_ptr = (struct paquete_datos *) malloc(*package_len)) == NULL )
    {
        return NULL;
    }

    package_ptr->op = htons(op);
    package_ptr->id = htonl(id);
    memcpy((void *) &package_ptr->data, (void *)data, data_len);
    ptr = (char *) &package_ptr->data;
    ptr[data_len] = DF;

    return package_ptr;
}

/**
 *
 * 
 */
struct paquete_id * paquete_id_create(char *my_ip, size_t *package_len)
{
    char *timestamp;
    size_t timestamp_len, peticion_id_len;
    struct paquete_id *peticion_id;

    timestamp = get_date();
    timestamp_len = strlen(timestamp);
    peticion_id_len = sizeof(struct paquete_id);
    *package_len = peticion_id_len + timestamp_len;
    
    if ( (peticion_id = (struct paquete_id *) malloc(*package_len)) == NULL )
        return NULL;

    peticion_id->op = htons(OP_PETICION_ID);
    strncpy(peticion_id->d1, DNI1, STRING_DNI_SIZE);
    strncpy(peticion_id->d2, DNI2, STRING_DNI_SIZE);
    strncpy(peticion_id->ip, my_ip, STRING_IP_SIZE);
    memcpy((void *) &peticion_id->data, (void *)timestamp, timestamp_len);

    return peticion_id;
}

struct paquete_recurso *paquete_recurso_create( unsigned int id, char *my_ip, unsigned short my_port, char *resource, size_t *package_len)
{
    size_t resource_len, peticion_reg_len;
    struct paquete_recurso *peticion_reg;


    resource_len = strlen(resource);
    peticion_reg_len = sizeof(struct paquete_id);
    *package_len = peticion_reg_len + resource_len;
    
    if ( (peticion_reg = (struct paquete_recurso *) malloc(*package_len)) == NULL )
        return NULL;

    peticion_reg->op = htons(OP_PETICION_REG);
    peticion_reg->id = htonl(id);
    strncpy(peticion_reg->ip, my_ip, STRING_IP_SIZE);
    peticion_reg->port = htons(my_port);

    memcpy((void *) &peticion_reg->data, (void *)resource, resource_len);
    ((char *)&peticion_reg->data)[resource_len] = DF;

    return peticion_reg;
}

struct paquete_men *paquete_men_create( unsigned short op, char * data)
{
    struct paquete_men * paquete;
    size_t paquete_men_len, data_len;

    if(data == NULL)
        data_len = 0;
    else
        data_len = strlen(data);
    
    paquete_men_len = sizeof(struct paquete_men) + data_len - 1;

    paquete = (struct paquete_men *)malloc( paquete_men_len );
    paquete->op = htons(op);
    paquete->len = htons(data_len);
    memcpy((void *) &paquete->data, (void *)data, data_len);
    
    return paquete;
}

unsigned short cmdtous(char * str)
{

    unsigned short op = OP_UNKNOW;
    int c;

    c = strcmp(str, "conecta");
    if (c == 0){
        op = OP_CONECTAR;
    } else if ( c < 0) {
        c = strcmp(str, "commit");
        if (c == 0){
            op = OP_COMMIT;
        } else if ( c < 0) {        
            c = strcmp(str, "cambiaclave");
            if ( c == 0) {
                op = OP_CAMBIARCLAVE;
            } else if ( c < 0) {            
                c = strcmp(str, "buscar");
                if ( c == 0) {
                    op = OP_BUSCAR;
                } else if ( c < 0) {
                    c = strcmp(str, "borra");
                    if ( c == 0) {
                        op = OP_BORRAR;
                    } else if ( c < 0) {
                        c = strcmp(str, "bloquea");
                        if ( c == 0) {
                            op = OP_BLOQUEAR;
                        }
                    }
                }
            }
        }

    } else if ( c > 0) {
        c = strcmp(str, "crea");
        if ( c == 0) {
            op = OP_CREAR;
        } else if ( c > 0) {
             c = strcmp(str, "desbloquea");
            if ( c == 0) {
                op = OP_DESBLOQUEAR;
            } else if ( c > 0) {
                c = strcmp(str, "listado");
                if ( c == 0) {
                    op = OP_LISTAR;
                } else if ( c > 0) {
                    c = strcmp(str, "rollback");
                    if ( c == 0) {
                        op = OP_ROLLBACK;
                    } else if ( c > 0) {
                        c = strcmp(str, "salir");
                        if ( c == 0) {
                            op = OP_SALIR;
                        }
                    }                  
                }                
            }           
        }
    }

    return op;

}