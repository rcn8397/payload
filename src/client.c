#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>

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

void client( const char *address, const char *port, const char *data, int udp )
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

    printf( "Sending %i bytes\n", strlen( buffer ));

    if( udp )
    {
      printf( "sending with udp\n" );
   
      char* ptr = buffer;
      while( ptr < ( buffer + length ) )
      {
        
        if ((ptr-buffer)%4<3)
          UDP_send( ptr, 1 );
        ptr += 1;
        usleep( 1 );
      }
      char end = 0;
      UDP_send( &end, 1 );
    }
    else
      BetterUDP_send( buffer, length-1 );
    
    UDP_close();

}
