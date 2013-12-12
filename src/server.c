#include <stdio.h>

#include "udp_socket.h"
#include "server.h"
#include <string.h>
#include <stdlib.h>

void server( const char *address, const char *port, int udp )
{
    printf("Run Server\n");
      
    UDP_ServerInit( port, address );

    char* buffer = (char*)malloc( sizeof(char) * MAXBUFFERLEN );

    /* initialize the UDP connection */
    int length = 0;
    int total  = 0;
    if( udp )
    {
      // HACK, first three bytes are bad, ignore them
      int i;
      for( i=0; i<3; i++ )
        UDP_recv( buffer, &length );
      length = 0;
      do
      {
        total += length;
        UDP_recv( &((buffer+total)[0]), &length );
        //printf( "%c", (buffer+total)[0] );
      } while( (buffer+total)[0] != 0 );
      length = total;
    }
    else
      length = BetterUDP_receive( &buffer );

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
