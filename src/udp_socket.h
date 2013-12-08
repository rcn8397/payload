#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#define MAXBUFFERLEN 1024*1024

int UDP_ClientInit( const char *port, const char *ip );
int UDP_send( char *buff, int size );
int UDP_ServerInit( const char *port, const char *ip );
int UDP_recv( char *buff, int *size );
void UDP_close( void );

#endif /* _UDP_SOCKET_H_ */
