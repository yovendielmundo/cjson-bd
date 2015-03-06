/*
 * slr.h
 *
 *  Created on: 04/07/2012
 */

#ifndef SLR_H_
#define SLR_H_

#include "common.h"
#include "socket.h"


int locate_resource_from_slr( int my_port, char *my_ip,  int *host_port, char *host_ip, char *resource );

int register_resource_at_slr( int my_port, char *my_ip,  int host_port, char *host_ip, char *resource );


#endif /* SLR_H_ */
