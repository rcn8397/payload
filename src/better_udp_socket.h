#ifndef _BETTER_UDP_SOCKET_H_
#define _BETTER_UDP_SOCKET_H_

/* sequence number size */
#define SEQ_NUM_SIZE ( sizeof( int ) )

/* sequence total number size */
#define SEQ_TOTAL_NUM_SIZE ( sizeof( int ) )

/* ecc size ( currently really only 1 bit stuffed in 1 byte ) */
#define ECC_FLAG_SIZE ( sizeof( char ) )

/* ( FIXME: this should get set to: 
         MTU - IP header - UDP header - BetterUDP stuff
     Issue is we can't really assume 1500 as MTU, can assume we are using
     IP, but if we are IP can have options so 20 can't be assume.  All we
     know is a UDP header is 20 and our better udp header is 2 )

   For now this is 1 byte ( actually 1 bit contained in 1 byte )
*/
#define DATA_SIZE            ( sizeof( char ) )

/* BUDP only size ( not UDP header )*/
#define MSG_CHUNK_SIZE       ( SEQ_NUM_SIZE + SEQ_TOTAL_NUM_SIZE + ECC_FLAG_SIZE + DATA_SIZE )

/* constant for offsetting into buffer for writing sequence number into */
#define SEQ_NUM_OFFSET       ( 0 )

/* constant for offsetting into buffer for writing total sequence number into */
#define SEQ_TOTAL_NUM_OFFSET ( SEQ_NUM_SIZE )

/* number of error correcting bytes ( actually 1 bit ecc contained in 1 byte ) */
#define ECC_FLAG_NUM_OFFSET  ( SEQ_TOTAL_NUM_OFFSET + SEQ_TOTAL_NUM_SIZE )

/*  constant for offsetting writing of data into the buffer */
#define DATA_NUM_OFFSET      ( ECC_FLAG_NUM_OFFSET + ECC_FLAG_SIZE )

int BetterUDP_send( char* buff, unsigned int msg_size );
char* BetterUDP_sendAll( char* buff, unsigned int msg_size );

#endif /* _UDP_SOCKET_H_ */
