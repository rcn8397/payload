#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <errno.h>

int sockfd;
struct addrinfo *p;

int UDPinit( int port, char *ip )
{
    int status;
    
    struct addrinfo hints;
    struct addrinfo *addrs; //< results list

    ///< Data setup
    memset( &hints, 0, sizeof( hints ) );

    hints.ai_family   = AF_UNSPEC;  ///< Don't care whether ip 4 or 6
    hints.ai_socktype = SOCK_DGRAM; ///< UDP

    if( ( status = getaddrinfo( ip, port, &hints, &addrs ) ) != 0 )
    {
        perror( "getaddrinfo error: %s\n", gai_strerror( status ) );
        return -1;
    }
    
    ///< Find a socket in the results
    for( p = addrs; p != NULL; p = p->ai_next )
    {
        if( ( sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
        {
            perror( "Client: socket" );
            continue;
        }
        break;
    }
    
    if( p == NULL )
    {
        perror( "Could not bind socket\n" );
        return -1;
    }

        
 
}


int UDP_send( char *buff, int size )
{
    int sent;

    if( p == NULL )
    {
        perror( "No socket bound" );
        return -1;
    }

    sent = sendto( sockfd,
                   buff,
                   size,
                   0, 
                   p->ai_addr,
                   p->ai_addrlen )
            
    return sent;
}

void UDP_close( void )
{
    if( addrs == NULL )
    {
        perror( "No socket bound" );
        return -1;
    }

    freeaddrinfo( addrs );
    close( sockfd );
}


int UDP_ServerInit( int port, char *ip, int type )
{
    int status;
    struct addrinfo hints;
    struct addrinfo *addrs; //< results list

    ///< Data setup
    memset( &hints, 0, sizeof( hints ) );

    hints.ai_family   = AF_UNSPEC;  ///< Don't care whether ip 4 or 6
    hints.ai_socktype = SOCK_DGRAM; ///< UDP

    hints.ai_flags    = AI_PASSIVE; ///< Uses current IP
    
    if( ( status = getaddrinfo( NULL, port, &hints, &addrs ) ) != 0 )
    {
        perror( "getaddrinfo error: %s\n", gai_strerror( status ) );
    }
    break;

    ///< Find a socket in the results
    for( p = addrs; p != NULL; p = p->ai_next )
    {
        if( ( sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
        {
            perror( "Server: socket" );
            continue;
        }
    
        if( bind ( sockfd, p->ai_addr, p->ai_addrlen ) == -1 )
        {
            close( sockfd );
            perror( "Server: bind" );
            continue;
        }
        break;
    }
    
    
    if( p == NULL )
    {
        perror( "Could not bind socket\n" );
        return -1;
    }
}
