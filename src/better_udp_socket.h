#ifndef _BETTER_UDP_SOCKET_H_
#define _BETTER_UDP_SOCKET_H_

#define MAXBUFFERLEN 1024

/* seqence number + total sequence */
#define SEQ_EXTRAS_SIZE      ( sizeof( int ) * 2 )
/* arbitrary data size */
#define DATA_SIZE            ( 32 )
/* data chunk size + sequence number + total sequences */
#define MSG_CHUNK_SIZE       ( DATA_SIZE + SEQ_EXTRAS_SIZE )
/* constant for offsetting into buffer after writing sequence number in */
#define SEQ_NUM_OFFSET       ( sizeof( int ) )
/* constant for offsetting into buffer after writing sequence number and total sequences in */
#define TOTAL_SEQ_NUM_OFFSET ( SEQ_NUM_OFFSET * 2 )

int BetterUDP_send( char* buff, unsigned int msg_size );
char* BetterUDP_sendAll( char* buff, unsigned int msg_size );

#endif /* _UDP_SOCKET_H_ */
