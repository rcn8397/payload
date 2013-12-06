#include <stdio.h>

#include "udp_socket.h"
#include "server.h"

void server( const char *address, const char *port )
{
    printf("Run Server\n");
    printf(" addr: %s\n", address );
    printf(" port: %s\n", port );

    UDP_ServerInit( port, address );
    
    char buff[ MAXBUFFERLEN ];
    int size ;
    UDP_recv( buff, &size );

    UDP_close();

}
