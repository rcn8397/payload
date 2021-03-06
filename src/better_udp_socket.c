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

int BetterUDP_send( byte* buff, unsigned int msg_size )
{
  byte* sendBuff = NULL;
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

  sendBuff = ( byte* )malloc( sizeof( byte ) * MSG_CHUNK_SIZE );

  totalSize = msg_size + (totalMsgs/4)*3*MSG_CHUNK_SIZE;

  // We have 3 ecc messages for every 4 data messages.  
  // CARSON TODO: if we have less than 4 data packets, how many ecc packets do we need?
  int maxSeqNum = totalMsgs + (totalMsgs/4)*3;
  
  while( sequenceNum <= maxSeqNum )
  {
    byte* data = buff + ( i * DATA_SIZE );

    if( !eccSendMsg( &data, DATA_SIZE ) )
      i++;

    /* copy the seqence number into buffer */
    memcpy( sendBuff + SEQ_NUM_OFFSET, &sequenceNum, SEQ_NUM_SIZE ); 

    /* copy the total number of sequences into the buffer */
    memcpy( sendBuff + SEQ_TOTAL_NUM_OFFSET, &maxSeqNum, SEQ_TOTAL_NUM_SIZE ); 

    /* copy the msg data */
    memcpy( sendBuff + DATA_NUM_OFFSET, data, DATA_SIZE ); 

    // Simple flow control implementation, assuming 1GBps network
    usleep( 1 );

    int sn = sequenceNum-1;
    // simulate a packet loss from every block
    if( sn % 7 != 3 )
    {
      //if( sn % 7 < 4 )
        //printf( "Sending: %c\n", (sendBuff + DATA_NUM_OFFSET)[0] );
      //else
        //printf( "Sending ECC: %x\n", (sendBuff + DATA_NUM_OFFSET)[0] );

      if( totalSize >= DATA_SIZE )
        UDP_send( sendBuff, MSG_CHUNK_SIZE );
      else
        UDP_send( sendBuff, ( totalSize + DATA_NUM_OFFSET ) );
    }
    //else
      //printf( "Not sending: %c\n", (sendBuff + DATA_NUM_OFFSET)[0] );

    totalSize -= DATA_SIZE;
    sequenceNum++;

    memset( sendBuff, 0x0, MSG_CHUNK_SIZE );
  }

  printf( "Sent %i UDP packets.\n", sequenceNum-1 );

  free( sendBuff );
  return 1; 
}

int BetterUDP_receive( byte** receive_buffer )
{
    byte recvBuff[ MAXBUFFERLEN ];
    byte* reassembleBuff = *receive_buffer;

    int size, i, error = 0;
    unsigned int seqNum = 0, totalSeq = 1, 
                 dataSize = DATA_SIZE,
                 reassembleBuffSize = 0, 
                 expectedSeq = 1,
                 maxSeqNum = 0;

    eccStartReceive();

    byte buff[DATA_SIZE];
    int  buff_len;
    while( eccGetMsg( &buff, &buff_len, &seqNum ) )
    {
      /* copy returned message from ecc processor into the reassemble buffer */ 
      if( buff_len > 0 )
      {
        //printf( "processing packet: %i\n", seqNum );
        memcpy( reassembleBuff + seqNum*DATA_SIZE, buff, buff_len );
        //printf( "   data[%i] = %c (%i,%i)\n", seqNum*DATA_SIZE, buff[0], buff[0], (byte)(*(reassembleBuff + seqNum*DATA_SIZE)) ); 
        maxSeqNum = seqNum > maxSeqNum ? seqNum : maxSeqNum ;
        reassembleBuffSize += buff_len ;
      }

      /* receive a new msg from the net*/
      //printf("a\n");
      if( !eccEOM() )
        error = UDP_recv( recvBuff, &size );
      else
        size = 0;
      //printf("b\n");

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
        byte* ptr = recvBuff + DATA_NUM_OFFSET;
        //if( (seqNum-1)%7 < 4 )
        //  printf( "from net(%i): %c\n", seqNum, (recvBuff + DATA_NUM_OFFSET)[0] );
        //else
        //  printf( "from net ecc(%i): %i\n", seqNum, (recvBuff + DATA_NUM_OFFSET)[0] );

        eccReceiveMsg( seqNum, totalSeq, &ptr, dataSize );

        //printf( "received sequence number %i\n", seqNum );

        // TODO: check that totalSeq hasn't changed from packet to packet

      }
    }


    printf( "Received %i UDP packets\n", totalSeq );
  
    return reassembleBuffSize;
}
