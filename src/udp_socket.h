#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#define MAXBUFFERLEN 100

int UDP_ClientInit( char *port, char *ip );
int UDP_send( char *buff, int size );
int UDP_ServerInit( char *port, char *ip );
int UDP_recv( char *buff, int size );
void UDP_close( void );



#endif /* _UDP_SOCKET_H_ */
