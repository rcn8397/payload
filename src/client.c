#include <stdio.h>

#include "udp_socket.h"
#include "client.h"

void client( const char *address, const char *port )
{
    char send[] = "helloWorld!";

    printf("Run Client\n");
    printf(" addr: %s\n", address );
    printf(" port: %s\n", port );

    UDP_ClientInit( port, address );

    
    UDP_send( send, sizeof( send ) );
    
    UDP_close();

}
