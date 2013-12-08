#include <stdio.h>

#include "udp_socket.h"
#include "server.h"
#include <string.h>
#include <stdlib.h>

void server( const char *address, const char *port )
{
    printf("Run Server\n");

    /* initialize the UDP connection */
    UDP_ServerInit( port, address );

    BetterUDP_receive();

    UDP_close();
}
