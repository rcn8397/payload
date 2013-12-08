#include <stdio.h>

#include "udp_socket.h"
#include "server.h"
#include <string.h>
#include <stdlib.h>

void server( const char *address, const char *port )
{
    REQUEST_PACKET rp;

    printf("Run Server\n");
    printf(" addr: %s\n", address );
    printf(" port: %s\n", port );

    char recvBuff[ MAXBUFFERLEN ];
    char* reassembleBuff = (char*)malloc( sizeof( char) * MAXBUFFERLEN );
    int receivedSequences[ MAXBUFFERLEN / MSG_CHUNK_SIZE ];

    int size, i, error = 0, loop = 1;
    unsigned int seqNum = 0, totalSeq = 0, 
                 buffPtr = 0, 
                 dataSize = DATA_SIZE,
                 reassembleBuffPtr = 0, reassembleBuffSize = 0, 
                 expectedSeq = 1;

    /* initialize the UDP connection */
    UDP_ServerInit( port, address );

    /* initialize the receivedSequences[] array */
    for( i = 0; i < MAXBUFFERLEN / MSG_CHUNK_SIZE; ++i )
      receivedSequences[ i ] = -1;

    while( loop <= 3 )
    {
      /* receive a msg */
      error = UDP_recv( recvBuff, &size );

      if( size > 0 )
      {
        buffPtr = 0;

        while( buffPtr < size )
        {
          /* grab the seq num and total seq */
          memcpy( &seqNum, recvBuff + buffPtr, sizeof( int ) );
          memcpy( &totalSeq, recvBuff + (buffPtr + SEQ_NUM_OFFSET), sizeof( int ) );

          /* tailor the data size for msgs that are smaller than the MSG_CHUNK_SIZE */
          if( size >= MSG_CHUNK_SIZE )
            dataSize = DATA_SIZE;
          else
            dataSize = size - TOTAL_SEQ_NUM_OFFSET;

          /* copy the data (minus the sequence and total sequence) into a buffer */
          memcpy( reassembleBuff + reassembleBuffPtr, recvBuff + (buffPtr + TOTAL_SEQ_NUM_OFFSET), dataSize );

          /* record that we received a sequence */
          receivedSequences[ seqNum ] = 1;

          /* update the buffer pointer */
          buffPtr += MSG_CHUNK_SIZE;

          /* update the current pointer and buffer size for the 'reassemble' buffer */
          reassembleBuffPtr += dataSize;
          reassembleBuffSize += dataSize;
        }
      }

      loop++;
    }
  
    //free( reassembleBuff );
    UDP_close();

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
