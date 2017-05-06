/*
 *  File name: clientAPI.h
 *  
 *  Description: This file describes the interface 
 *  API to be used by client programs in order to comunicate and 
 *  interact with distributed server system (Psis Project)
 *
 */

/* Prevent multiple inclusions                                      */
#ifndef clientAPIHeader
#define clientAPIHeader

#include <netinet/in.h>  /* definition of in_port_t data type */


/*
 *  Function:
 *    gallery_connect
 *
 *  Description:
 *    connects client to one Peer. Must be called before attempting other 
 *    client-server interactions.
 *
 *  Arguments:
 *    char *host: gateway address string
 *    in_port_t port: gateway port
 *
 *  Return value:
 *    socket descriptor in case of success.
 *    -1 if gateway cannot be accessed
 *    0 if no servers are available
 *    
 */
int gallery_connect(char *host, in_port_t port);

#endif
