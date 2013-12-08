#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>

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

int UDP_ClientInit( const char *port, const char *ip )
{
    int status;
    
    struct addrinfo hints;
    
    log_domain = g_strdup( "CLIENT_SOCKET" );
    printf(" addr: %s\n", ip );
    printf(" port: %s\n", port );


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
               "UDP_send: No socket bound" );
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
    freeaddrinfo( addrs );
    close( sockfd );
}


int UDP_ServerInit( const char *port, const char *ip )
{
    int status;
    struct addrinfo hints;
    struct addrinfo *addrs; //< results list

    log_domain = g_strdup( "SERVER_SOCKET" );
    printf(" addr: %s\n", ip );
    printf(" port: %s\n", port );


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


int UDP_recv( char *buff, int *size )
{
    int status = 0;
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    char client_name[ INET6_ADDRSTRLEN ];
    const char *ip;
    int done = FALSE;

    if( p == NULL )
    {
        g_log( log_domain, 
               G_LOG_LEVEL_CRITICAL,
               "UDP_rec: No socket bound" );
        return -1;
    }
    
    if( !sockfd )
        return -1;

    addr_len = sizeof( client_addr );

    while( 1 )
    {
        status = 0;
        errno = 0;
        /*
        *size = recvfrom( sockfd,
                          buff,
                          *size,
                          MAXBUFFERLEN - 1,
                          ( struct sockaddr *) &client_addr, 
                          &addr_len );
         */

        *size = recvfrom( sockfd,
                          buff,
                          MAXBUFFERLEN - 1,
                          0,
                          ( struct sockaddr *) &client_addr, 
                          &addr_len );
        
        status = errno;
        //printf("%s\n", strerror( status ) );
    
        ///< Debug
        ip = inet_ntop( client_addr.ss_family, 
                        get_in_addr( ( struct sockaddr * ) &client_addr ),
                        client_name, 
                        sizeof (client_name ) );

        if( ip && (*size > 0 ) )
        {
            buff[ *size ] = '\0';
            printf( "UDP_recv: got packet from %s\n", ip );
            printf( "UDP_recv: %d bytes received\n", *size );
            printf( "UDP_recv: msg: %s \n", buff );
            return 0;
        }

       
    }
    
        
    return 1;
}

int BetterUDP_send( char* buff, unsigned int msg_size )
{
  char* returnBuff = NULL;
  unsigned int sequenceNum = 1;
  unsigned int totalMsgs = 1, totalSize = 0;
  int i = 0;

  if( msg_size > 0 )
  {
    /* calculate the number of chunks to return */
    if( msg_size > MSG_CHUNK_SIZE )
       totalMsgs = ( ( msg_size / DATA_SIZE ) + ( ( msg_size % DATA_SIZE ) ? 1 : 0 ) );

    returnBuff = ( char* )malloc( sizeof( char ) * MSG_CHUNK_SIZE );

    totalSize = msg_size;

    for( i = 0; i < totalMsgs; ++i, ++sequenceNum )
    {
      /* copy the seqence number into buffer */
      memcpy( returnBuff, &sequenceNum, sizeof( int ) ); 
      /* copy the total number of sequences into the buffer */
      memcpy( returnBuff + SEQ_NUM_OFFSET, &totalMsgs, sizeof( int ) ); 
      /* copy the msg data */
      memcpy( returnBuff + TOTAL_SEQ_NUM_OFFSET, buff + (i * DATA_SIZE), DATA_SIZE ); 

      if( totalSize > DATA_SIZE )
        UDP_send( returnBuff, MSG_CHUNK_SIZE );
      else
        UDP_send( returnBuff, ( totalSize + TOTAL_SEQ_NUM_OFFSET ) );

      totalSize -= DATA_SIZE;

      memset( returnBuff, 0x0, MSG_CHUNK_SIZE );
    } 
  }
  else
  {
    fprintf( stdout, "ERROR: Message size was %d in betterUDP_send()\n", msg_size );
    fflush ( stdout );
    return 0;
  }

  free( returnBuff );
  return 1; 
}

char* BetterUDP_sendAll( char* buff, unsigned int msg_size )
{
  char* returnBuff = NULL;
  unsigned int sequenceNum = 1;
  unsigned int totalMsgs = 1;
  int i = 0;

  if( msg_size > 0 )
  {
    /* calculate the number of chunks to return */
    if( msg_size > MSG_CHUNK_SIZE )
       totalMsgs = ( ( msg_size / DATA_SIZE ) + ( ( msg_size % DATA_SIZE ) ? 1 : 0 ) );

    returnBuff = ( char* )malloc( sizeof( char ) * ( MSG_CHUNK_SIZE * totalMsgs ) );

    for( i = 0; i < totalMsgs; ++i, ++sequenceNum )
    {
      /* copy the seqence number into buffer */
      memcpy( returnBuff, &sequenceNum, sizeof( int ) ); 
      /* copy the total number of sequences into the buffer */
      memcpy( returnBuff + SEQ_NUM_OFFSET, &totalMsgs, sizeof( int ) ); 
      /* copy the msg data */
      memcpy( returnBuff + TOTAL_SEQ_NUM_OFFSET, buff + (i * DATA_SIZE), DATA_SIZE ); 
    } 
  }
  else
  {
    fprintf( stdout, "ERROR: Message size was %d in betterUDP_send()\n", msg_size );
    fflush ( stdout );
  }

  return returnBuff; 
}

