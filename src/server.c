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

    char* buffer = (char*)malloc( sizeof(char) * MAXBUFFERLEN );
    int length = BetterUDP_receive( &buffer );

    // print the first line of the received txt
    //  NOTE: the ebook text uses the DOS EOL format!!!
    printf( "Received %i characters\n", length );
    printf( "First Line:\n\'%.*s\'\n", strcspn( buffer, "\r\n"), buffer );

    printf( "length = %i\n", strlen( buffer ));

    FILE* fd = fopen( "out.txt", "w" );
    fwrite( buffer, sizeof(char), length, fd );
    fclose( fd );

    UDP_close();
}
