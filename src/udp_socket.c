#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "udp_socket.h"

char *log_domain;
int sockfd;
struct addrinfo *p;
struct addrinfo *addrs; //< results list

void *get_in_addr( struct sockaddr *sa )
{
    if( sa->sa_family == AF_INET )
    {
        return &( ( ( struct sockaddr_in * ) sa )->sin_addr );
    }

    return &( ( ( struct sockaddr_in6 * ) sa )->sin6_addr );
}

int UDP_ClientInit( char *port, char *ip )
{
    int status;
    
    struct addrinfo hints;
    
    log_domain = g_strdup( "CLIENT_SOCKET" );

    ///< Data setup
    memset( &hints, 0, sizeof( hints ) );

    hints.ai_family   = AF_UNSPEC;  ///< Don't care whether ip 4 or 6
    hints.ai_socktype = SOCK_DGRAM; ///< UDP

    if( ( status = getaddrinfo( ip, port, &hints, &addrs ) ) != 0 )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "getaddrinfo error: %s",
               gai_strerror( status ) );
        return -1;
    }
    
    ///< Find a socket in the results
    for( p = addrs; p != NULL; p = p->ai_next )
    {
        if( ( sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
        { 
            perror("Client: socket ");
            continue;
        }
        break;
    }
    
    if( p == NULL )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "Could not bind socket" );
        return -1;
    }
}


int UDP_send( char *buff, int size )
{
    int sent;

    if( p == NULL )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "No socket bound" );
        return -1;
    }

    sent = sendto( sockfd,
                   buff,
                   size,
                   0, 
                   p->ai_addr,
                   p->ai_addrlen );
            
    return sent;
}

void UDP_close( void )
{
    if( addrs == NULL )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "No socket bound" );
    }

    freeaddrinfo( addrs );
    close( sockfd );
}


int UDP_ServerInit( char *port, char *ip )
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
         g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "getaddrinfo error: %s",
               gai_strerror( status ) );
         return -1;
    }
    
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
         g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "No socket bound" );
        return -1;
    }
}


int UDP_recv( char *buff, int size )
{
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    char client_name[ INET6_ADDRSTRLEN ];
    const char *ip;

    if( p == NULL )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "No socket bound" );
        return -1;
    }

    addr_len = sizeof( client_addr );

    size = recvfrom( sockfd,
                     buff,
                     size,
                     MAXBUFFERLEN - 1,
                     ( struct sockaddr *) &client_addr, 
                     &addr_len );

    ///< Debug
    ip = inet_ntop( client_addr.ss_family, 
                    get_in_addr( ( struct sockaddr * ) &client_addr ),
                    client_name, 
                    sizeof (client_name ) );
    if( ip )
    {
        printf( "UDP_recv: got packet from %s\n", ip );
    }

    printf( "UDP_recv: %d bytes received\n", size );
    
        
    return 1;
}
