#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "udp_socket.h"
#include "client.h"
#include <string.h>

char* log_domain;
char* file_name;

int readFile( const char* fn, char** buff )
{
    FILE* fd = fopen( fn, "rb" );
    if( fd == NULL )
        g_log( log_domain,
               G_LOG_LEVEL_ERROR,
               "Error opening file: %s",
               file_name );       

    fseek( fd, 0L, SEEK_END );
    int size = ftell( fd );
    rewind( fd );

    *buff = (char*)malloc( (size+1)*sizeof(char) );
    
    if( 1 != fread( *buff, size, 1, fd ))
        g_log( log_domain,
               G_LOG_LEVEL_ERROR,
               "Error reading file: %s",
               file_name );

    fclose( fd );
}

void client( const char *address, const char *port )
{
    log_domain = g_strdup( "CLIENT:" );
    file_name  = g_strdup( "christmas_carol.txt" );

    char* buffer = 0;
    int length = readFile( file_name, &buffer );
 
    // initialize the udp socket
    UDP_ClientInit( port, address );

    struct REQUEST_PACKET rp;
    rp.opcode = REQUEST_OPCODE;
    rp.id = 0;

    char buff[ sizeof( rp ) * 10 ];

    unsigned int ptrOffset = 0;

    int i;
    for( i = 0; i < 10; ++i )
    {
      rp.id += 1;
      memcpy( buff + ptrOffset, &rp, sizeof( rp ) );
      ptrOffset += sizeof( rp );
    }

    BetterUDP_send( buff, ( sizeof( rp ) * 10 ) );
    
    UDP_close();

}
