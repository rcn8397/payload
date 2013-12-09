#ifndef _BETTER_UDP_SOCKET_H_
#define _BETTER_UDP_SOCKET_H_

/* sequence number size */
#define SEQ_NUM_SIZE ( sizeof( int ) )

/* sequence total number size */
#define SEQ_TOTAL_NUM_SIZE ( sizeof( int ) )

#define DATA_SIZE            ( sizeof( char )*100 )

/* BUDP only size ( not UDP header )*/
#define MSG_CHUNK_SIZE       ( SEQ_NUM_SIZE + SEQ_TOTAL_NUM_SIZE + DATA_SIZE )

/* constant for offsetting into buffer for writing sequence number into */
#define SEQ_NUM_OFFSET       ( 0 )

/* constant for offsetting into buffer for writing total sequence number into */
#define SEQ_TOTAL_NUM_OFFSET ( SEQ_NUM_SIZE )

/*  constant for offsetting writing of data into the buffer */
#define DATA_NUM_OFFSET      ( SEQ_TOTAL_NUM_OFFSET + SEQ_TOTAL_NUM_SIZE ) 

int BetterUDP_send( char* buff, unsigned int msg_size );
char* BetterUDP_sendAll( char* buff, unsigned int msg_size );

#endif /* _UDP_SOCKET_H_ */
