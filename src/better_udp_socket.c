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

  printf( "sending data[0] = %i\n", buff[0] ); 
  /* calculate the number of chunks to return */
  if( msg_size > DATA_SIZE )
     totalMsgs = ( ( msg_size / DATA_SIZE ) + ( ( msg_size % DATA_SIZE ) ? 1 : 0 ) );

  sendBuff = ( char* )malloc( sizeof( char ) * MSG_CHUNK_SIZE );

  totalSize = msg_size;

  // We have 3 ecc messages for every 4 data messages.  
  // BILL TODO: comment the second part of this back in once eccAddMsg() below is implemented
  // FIXME:  when we scale this above 1bit per packet, we will need to figure out how many ecc
  //         packets there will be when we have less than 4 data pacekts.
  int maxSeqNum = totalMsgs ;//+ (totalMsgs/4)*3 ;

  while( i < totalMsgs )
  {
    char* data = buff + ( i * DATA_SIZE );

    // BILL TO DO: pass current data for packet to outer ecc function here!!!
    //     Thought here is to pass current data to outer ecc calculator, if its
    //     time to send a ecc packet instead of a data packet, eccAddMsg() will 
    //     modify the data pointer with the ecc data and return true.  Returning
    //     false means all you did was record data and not change data.  If false then
    //     we want to increment our i so we move to the next piece of actual data.
    //if( !eccSendMsg( &data, DATA_SIZE ) )
      i++;

    // BILL TO DO: call calculate inner ecc function here!!!
    //             Just FYI, flag is actually 1 bit according to carson, using 1 byte
    //             to store it for ease of programming
    char ecc_flag = eccInnerFlag( data );

    /* copy the seqence number into buffer */
    memcpy( sendBuff + SEQ_NUM_OFFSET, &sequenceNum, SEQ_NUM_SIZE ); 

    /* copy the total number of sequences into the buffer */
    memcpy( sendBuff + SEQ_TOTAL_NUM_OFFSET, &maxSeqNum, SEQ_TOTAL_NUM_SIZE ); 

    /* copy the ecc flag into the buff */
    memcpy( sendBuff + ECC_FLAG_NUM_OFFSET, &ecc_flag, ECC_FLAG_SIZE );

    /* copy the msg data */
    memcpy( sendBuff + DATA_NUM_OFFSET, data, DATA_SIZE ); 
    printf( "sending: %i\n", data[0] );

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
        printf( "processing packet: %i\n", seqNum );
        memcpy( reassembleBuff + seqNum*DATA_SIZE, buff, buff_len );
        printf( "   data[%i] = %i,%i\n", seqNum*DATA_SIZE, buff[0], (char)(*(reassembleBuff + seqNum*DATA_SIZE)) ); 
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
        memcpy( &eccFlag,  recvBuff + ECC_FLAG_NUM_OFFSET,  ECC_FLAG_SIZE );

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
        printf( "from net: %i\n", (char)(*(recvBuff + DATA_NUM_OFFSET)) );
        eccReceiveMsg( seqNum, totalSeq, eccFlag, &ptr, dataSize );

        //printf( "received sequence number %i\n", seqNum );

        // TODO: check that totalSeq hasn't changed from packet to packet

      }
    }

    reassembleBuffSize = maxSeqNum * MSG_CHUNK_SIZE;

    // For now Better UDP only supports bit data packets, so we must repack the bits into
    // bytes to reform the original message
    int new_size = fromBitStream( &reassembleBuff, sizeof(char)*MAXBUFFERLEN );

    printf( "Received %i UDP packets\n", maxSeqNum );
  
    return new_size;
}
