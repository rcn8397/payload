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
#include "better_udp_socket.h"

int BetterUDP_send( char* buff, unsigned int msg_size )
{
  char* sendBuff = NULL;
  unsigned int sequenceNum = 1;
  unsigned int totalMsgs = 1, totalSize = 0;
  int i = 0;

  if( msg_size <= 0 )
  {
    fprintf( stdout, "ERROR: Message size was %d in betterUDP_send()\n", msg_size );
    fflush ( stdout );
    return 0;
  }

  /* calculate the number of chunks to return */
  if( msg_size > DATA_SIZE )
     totalMsgs = ( ( msg_size / DATA_SIZE ) + ( ( msg_size % DATA_SIZE ) ? 1 : 0 ) );

  sendBuff = ( char* )malloc( sizeof( char ) * MSG_CHUNK_SIZE );

  totalSize = msg_size;

  int ecc_flag = 0; // BILL TO DO: call calculate inner ecc function here!!!

  while( i < totalMsgs )
  {
    char* data = buff + ( i * DATA_SIZE );

    // BILL TO DO: pass current data for packet to outer ecc function here!!!
    //     Thought here is to pass current data to outer ecc calculator, if its
    //     time to send a ecc packet instead of a data packet, eccAddMsg() will 
    //     modify the data pointer with the ecc data and return true.  Returning
    //     false means all you did was record data and not change data.  If false then
    //     we want to increment our i so we move to the next piece of actual data.
    //if( !eccAddMsg( &data, DATA_SIZE ) )
      i++;

    /* copy the seqence number into buffer */
    memcpy( sendBuff + SEQ_NUM_OFFSET, &sequenceNum, sizeof( int ) ); 

    /* copy the total number of sequences into the buffer */
    memcpy( sendBuff + SEQ_TOTAL_NUM_OFFSET, &totalMsgs, sizeof( int ) ); 

    /* copy the ecc flag into the buff */
    memcpy( sendBuff + ECC_FLAG_NUM_OFFSET, &ecc_flag, sizeof( int ) );

    /* copy the msg data */
    memcpy( sendBuff + DATA_NUM_OFFSET, data, DATA_SIZE ); 

    // TODO: simulate packet loss here
    // TODO: simulate out of order packets here
    if( totalSize >= DATA_SIZE )
      UDP_send( sendBuff, MSG_CHUNK_SIZE );
    else
      UDP_send( sendBuff, ( totalSize + DATA_NUM_OFFSET ) );

    totalSize -= DATA_SIZE;
    sequenceNum++;

    memset( sendBuff, 0x0, MSG_CHUNK_SIZE );
  }

  free( sendBuff );
  return 1; 
}

#if 0
/* FIXME: UNUSED FOR NOW */

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
#endif
