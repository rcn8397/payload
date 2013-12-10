#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>

#include "udp_socket.h"
#include "better_udp_socket.h"
#include "packets.h"
#include "hamming.h"

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

  // We have 3 ecc messages for every 4 data messages.  
  // CARSON TODO: if we have less than 4 data packets, how many ecc packets do we need?
  int maxSeqNum = totalMsgs + (totalMsgs/4)*3;
  
  while( sequenceNum <= maxSeqNum )
  {
    char* data = buff + ( i * DATA_SIZE );

    if( !eccAddMsg( &data, DATA_SIZE ) )
      i++;

    char ecc_flag = eccInnerFlag( data );

    /* copy the seqence number into buffer */
    memcpy( sendBuff + SEQ_NUM_OFFSET, &sequenceNum, SEQ_NUM_SIZE ); 

    /* copy the total number of sequences into the buffer */
    memcpy( sendBuff + SEQ_TOTAL_NUM_OFFSET, &maxSeqNum, SEQ_TOTAL_NUM_SIZE ); 

    /* copy the msg data */
    memcpy( sendBuff + DATA_NUM_OFFSET, data, DATA_SIZE ); 

    // TODO: simulate packet loss here
    // TODO: simulate out of order packets here
    // TODO: implement flow control
    usleep( 100 );
    if( totalSize >= DATA_SIZE )
      UDP_send( sendBuff, MSG_CHUNK_SIZE );
    else
      UDP_send( sendBuff, ( totalSize + DATA_NUM_OFFSET ) );

    totalSize -= DATA_SIZE;
    sequenceNum++;

    memset( sendBuff, 0x0, MSG_CHUNK_SIZE );
  }

  printf( "Sent %i UDP packets.\n", sequenceNum-1 );

  free( sendBuff );
  return 1; 
}

int BetterUDP_receive( char** receive_buffer )
{
    char recvBuff[ MAXBUFFERLEN ];
    char* reassembleBuff = *receive_buffer;

    int size, i, error = 0;
    unsigned int seqNum = 0, totalSeq = 1, 
                 dataSize = DATA_SIZE,
                 reassembleBuffSize = 0, 
                 expectedSeq = 1,
                 eccFlag = 0,
                 maxSeqNum = 0;

    eccStartReceive();

    char buff[DATA_SIZE];
    int  buff_len;
    while( eccGetMsg( &buff, &buff_len, &seqNum ) )
    {
      /* process a message from the ecc receiver */
      if( buff_len > 0 )
      {
        //printf( "processing packet: %i\n", seqNum );
        memcpy( reassembleBuff + seqNum*DATA_SIZE, buff, buff_len );
        //printf( "   data[%i] = %i,%i\n", seqNum*DATA_SIZE, buff[0], (char)(*(reassembleBuff + seqNum*DATA_SIZE)) ); 
        maxSeqNum = seqNum > maxSeqNum ? seqNum : maxSeqNum ;
      }

      /* receive a new msg from the net*/
      if( !eccEOM() )
        error = UDP_recv( recvBuff, &size );
      else
        size = 0;

      if( size > 0 )
      {
        /* grab the seq num and total seq */
        memcpy( &seqNum,   recvBuff + SEQ_NUM_OFFSET,       SEQ_NUM_SIZE );
        memcpy( &totalSeq, recvBuff + SEQ_TOTAL_NUM_OFFSET, SEQ_TOTAL_NUM_SIZE );

        /* tailor the data size for msgs that are smaller than the max DATA size*/
        dataSize = size - DATA_NUM_OFFSET;
        if( dataSize > DATA_SIZE )
        {
            printf( "Error receiving data (seq num = %i)! "
                    "Received data in packet is larger than maximum BUDP data packet.\n",
                    seqNum );
            continue;
        }

        // pass the received packet to the ecc receiver where it will record it and do the following:
        //   1) reorder out of order packets
        //   2) recreate missing packets if possible
        char* ptr = recvBuff + DATA_NUM_OFFSET;
        //printf( "from net(%i): %i\n", seqNum-1, (char)(*(recvBuff + DATA_NUM_OFFSET)) );
        eccReceiveMsg( seqNum, totalSeq, eccFlag, &ptr, dataSize );

        //printf( "received sequence number %i\n", seqNum );

        // TODO: check that totalSeq hasn't changed from packet to packet

      }
    }

    reassembleBuffSize = maxSeqNum * MSG_CHUNK_SIZE;

    printf( "Received %i UDP packets\n", maxSeqNum );
  
    return reassembleBuffSize;
}
