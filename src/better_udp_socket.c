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

// temporary hack - store each bit of every byte of the in buffer into 
//                  its own byte in the out buff.  
//                    sizeof(out_buff) == 8*sizeof(in_buff)
//                  bit will be stored little endian
int toBitStream( char** out_buff, char** in_buff, int in_size )
{
  *out_buff = (char*)malloc( sizeof(char)*in_size*8 );
  
  int i,j;
  for( i=0; i<in_size; i++ )
    for( j=0; j<8; j++ )
      (*out_buff)[i*8+j] = ( (*in_buff)[i] >> j ) & 0x1;
  return in_size*8;
}

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

  ////
  // we only support single bit packets for now, so convert byte buff to bit buffers
  //
  char* buff_mod;
  msg_size = toBitStream( &buff_mod, &buff, msg_size );
  buff = buff_mod;

  /* calculate the number of chunks to return */
  if( msg_size > DATA_SIZE )
     totalMsgs = ( ( msg_size / DATA_SIZE ) + ( ( msg_size % DATA_SIZE ) ? 1 : 0 ) );

  sendBuff = ( char* )malloc( sizeof( char ) * MSG_CHUNK_SIZE );

  totalSize = msg_size;


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

    // BILL TO DO: call calculate inner ecc function here!!!
    //             Just FYI, flag is actually 1 bit according to carson, using 1 byte
    //             to store it for ease of programming
    char ecc_flag = 0; 
    //int ecc_flag = eccInnerFlag( data );

    /* copy the seqence number into buffer */
    memcpy( sendBuff + SEQ_NUM_OFFSET, &sequenceNum, SEQ_NUM_SIZE ); 

    /* copy the total number of sequences into the buffer */
    memcpy( sendBuff + SEQ_TOTAL_NUM_OFFSET, &totalMsgs, SEQ_TOTAL_NUM_SIZE ); 

    /* copy the ecc flag into the buff */
    memcpy( sendBuff + ECC_FLAG_NUM_OFFSET, &ecc_flag, ECC_FLAG_SIZE );

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

int BetterUDP_receive( )
{
    REQUEST_PACKET rp;

    char recvBuff[ MAXBUFFERLEN ];
    char* reassembleBuff = (char*)malloc( sizeof( char) * MAXBUFFERLEN );
    int receivedSequences[ MAXBUFFERLEN / MSG_CHUNK_SIZE ];

    int size, i, error = 0, loop = 1;
    unsigned int seqNum = 0, totalSeq = 1, 
                 buffPtr = 0, 
                 dataSize = DATA_SIZE,
                 reassembleBuffPtr = 0, reassembleBuffSize = 0, 
                 expectedSeq = 1,
                 eccFlag = 0;
    char ecc_flag = 0;
    unsigned int maxSeqNum = 0;

    /* initialize the receivedSequences[] array */
    for( i = 0; i < MAXBUFFERLEN / MSG_CHUNK_SIZE; ++i )
      receivedSequences[ i ] = -1;

    // receive while we haven't received the final sequence number
    // TODO: need to timeout on a missed sequence
    while( maxSeqNum < totalSeq )
    {
      /* receive a msg */
      error = UDP_recv( recvBuff, &size );

      if( size > 0 )
      {
        buffPtr = 0;

        while( buffPtr < size )
        {
          /* grab the seq num and total seq */
          memcpy( &seqNum,   recvBuff + (buffPtr + SEQ_NUM_OFFSET),       SEQ_NUM_SIZE );
          memcpy( &totalSeq, recvBuff + (buffPtr + SEQ_TOTAL_NUM_OFFSET), SEQ_TOTAL_NUM_SIZE );
          memcpy( &eccFlag,  recvBuff + (buffPtr + ECC_FLAG_NUM_OFFSET),  ECC_FLAG_SIZE );

          /* tailor the data size for msgs that are smaller than the MSG_CHUNK_SIZE */
          if( size >= MSG_CHUNK_SIZE )
            dataSize = DATA_SIZE;
          else
            dataSize = size - DATA_NUM_OFFSET;

          memcpy( reassembleBuff + reassembleBuffPtr, recvBuff + (buffPtr + DATA_NUM_OFFSET), dataSize );

          /* record that we received a sequence */
          // TODO: Should probably copy what TCP does and only keep track of the largest sequence number
          //       we have received.  Trying to store an array for every potential integer slot for a 
          //       sequence number can be expensive.  Maybe cut down to only a short for sequence number.
          //       Short is still a lot of memory.
          receivedSequences[ seqNum ] = 1;

          /* update the buffer pointer */
          buffPtr += MSG_CHUNK_SIZE;

          /* update the current pointer and buffer size for the 'reassemble' buffer */
          reassembleBuffPtr += dataSize;
          reassembleBuffSize += dataSize;

          maxSeqNum = seqNum > maxSeqNum ? seqNum : maxSeqNum ;
          // TODO: check that totalSeq hasn't changed from packet to packet
          printf( "received sequence number %i\n", seqNum );
        }
      }

      loop++;
    }

    printf( "Received %i UDP packets\n", maxSeqNum );
  
    /* check for missed sequences and simply print for now */
    for( i = 1; i < ( MAXBUFFERLEN / MSG_CHUNK_SIZE ); ++i )
    {
      if( receivedSequences[ i ] == 0 )  
      {
        fprintf( stdout, "Missed sequence # %d\n", i );
        fflush ( stdout );
      }
    }

    int loopSize = reassembleBuffSize / ( sizeof( rp ) );

    /* for this test case, expecting REQUEST_PACKET data structures */
    for( i = 0, reassembleBuffPtr = 0; i < loopSize; i++, reassembleBuffPtr += sizeof( rp ) )
    {
      memcpy( &rp, reassembleBuff + reassembleBuffPtr, sizeof( rp ) );
      fprintf( stdout, "Opcode: %d, ID: %d\n", rp.opcode, rp.id );
      fflush ( stdout );
    }
}
