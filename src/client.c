#include <stdio.h>

#include "udp_socket.h"
#include "client.h"
#include <string.h>

void client( const char *address, const char *port )
{ 
    char send[] = "helloWorld!";
    int i;

    printf("Run Client\n");
    printf(" addr: %s\n", address );
    printf(" port: %s\n", port );

    UDP_ClientInit( port, address );

    struct REQUEST_PACKET rp;
    rp.opcode = REQUEST_OPCODE;
    rp.id = 0;

    char buff[ sizeof( rp ) * 10 ];

    unsigned int ptrOffset = 0;

    for( i = 0; i < 10; ++i )
    {
      rp.id += 1;
      memcpy( buff + ptrOffset, &rp, sizeof( rp ) );
      ptrOffset += sizeof( rp );
    }

    BetterUDP_send( buff, ( sizeof( rp ) * 10 ) );
    //UDP_send( buff, sizeof( rp ) );
    //UDP_send( send, sizeof( send ) );
    
    UDP_close();

}
