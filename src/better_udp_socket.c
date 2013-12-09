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

// temporary hack - store each bit of every byte of the in buffer into 
//                  its own byte in the out buff.  
//                    sizeof(out_buff) == 8*sizeof(in_buff)
//                  bit will be stored little endian
// \return size of out buffer
int toBitStream( char** out_buff, char** in_buff, int in_size )
{
  *out_buff = (char*)malloc( sizeof(char)*in_size*8 );
  
  int i,j;
  for( i=0; i<in_size; i++ )
    for( j=0; j<8; j++ )
      (*out_buff)[i*8+j] = ( (*in_buff)[i] >> j ) & 0x1;
  return in_size*8;
}

// inverse of toBitStream() - takes a buffer that contains byte data where
//              only the first bit of every byte contains data.  Pack those
//              bits into a new byte buffer. Bits are stored little endian.
//              New buffer will have the property:
//                 sizeof(out_buff)*8 == sizeof(in_buff)
// \return size of out buffer
int fromBitStream( char** in_buff, int in_size )
{
  int size = in_size/8 + (in_size%8 ? 1 : 0 );
  char* out_buff = (char*)malloc( sizeof(char)*size );

  int i,j;
  for( i=0; i<size; i++ )
    for( j=0; j<8; j++ )
      if( i*8+j < in_size )
        (out_buff)[i] |= (*in_buff)[i*8+j] << j ;

  memcpy( *in_buff, out_buff, size );
  (*in_buff)[size] = 0 ;
  return size;
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
  free( buff_mod );
  buff = buff_mod;

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

int BetterUDP_receive( char** receive_buffer )
{
    char recvBuff[ MAXBUFFERLEN ];
    char* reassembleBuff = *receive_buffer;
    // FIXME: this should be set to size of max int because sequence number is an int
    //        see TODO node below on how to fix this because setting this to be max int
    //        uses A LOT of space
    int receivedSequences[ MAXBUFFERLEN / MSG_CHUNK_SIZE ];

    int size, i, error = 0;
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
          //printf( "received sequence number %i\n", seqNum );
        }
      }
    }

    // For now Better UDP only supports bit data packets, so we must repack the bits into
    // bytes to reform the original message
    int new_size = fromBitStream( &reassembleBuff, sizeof(char)*MAXBUFFERLEN );

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

    return new_size;
}
