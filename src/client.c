#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "udp_socket.h"
#include "client.h"

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

    return size;
}

void client( const char *address, const char *port, const char *data )
{
    log_domain = g_strdup( "CLIENT:" );

    if( data )
        file_name = ( char *) data;
    else
        file_name  = g_strdup( "./data/christmas_carol.txt" );

    g_log( log_domain, 
           G_LOG_LEVEL_MESSAGE,
           "Opening %s for tx",
           file_name );
   
    char* buffer = 0;
    int length = readFile( file_name, &buffer );
 
    // initialize the udp socket
    UDP_ClientInit( port, address );

    BetterUDP_send( buffer, length );
    
    UDP_close();

}
