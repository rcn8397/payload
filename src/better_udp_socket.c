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

