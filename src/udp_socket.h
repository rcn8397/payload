#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#define MAXBUFFERLEN 1024

/* data chunk size + sequence number + total sequences */
#define SEQ_EXTRAS_SIZE      ( sizeof( int ) * 2 )
#define DATA_SIZE            ( 32 )
#define MSG_CHUNK_SIZE       ( DATA_SIZE + SEQ_EXTRAS_SIZE )
#define SEQ_NUM_OFFSET       ( sizeof( int ) )
#define TOTAL_SEQ_NUM_OFFSET ( SEQ_NUM_OFFSET * 2 )

int UDP_ClientInit( const char *port, const char *ip );
int UDP_send( char *buff, int size );
int UDP_ServerInit( const char *port, const char *ip );
int UDP_recv( char *buff, int *size );
void UDP_close( void );



#endif /* _UDP_SOCKET_H_ */
