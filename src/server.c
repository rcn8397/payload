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
    char* seqBuff = (char*)malloc( sizeof( char) * MAXBUFFERLEN );
    int receivedSequences[ MAXBUFFERLEN / MSG_CHUNK_SIZE ];

    int size, i, error = 0, loop = 1;
    unsigned int seqNum = 0, totalSeq = 0, 
                 buffPtr = 0, 
                 seqBuffPtr = 0, seqBuffSize = 0, 
                 expectedSeq = 1;

    /* initialize the UDP connection */
    UDP_ServerInit( port, address );

    /* initialize the receivedSequences[] array */
    for( i = 0; i < MAXBUFFERLEN / MSG_CHUNK_SIZE; ++i )
      receivedSequences[ i ] = -1;

    while( loop <= 2 )
    {
      /* receive a msg */
      error = UDP_recv( recvBuff, &size );

      if( size > 0 )
      {
        buffPtr = 0;

        fprintf( stdout, "size is: %d\n", size ); fflush( stdout );

        while( buffPtr < size )
        {
          memcpy( &seqNum, recvBuff + buffPtr, sizeof( int ) );
          memcpy( &totalSeq, recvBuff + (buffPtr + SEQ_NUM_OFFSET), sizeof( int ) );
          memcpy( seqBuff + seqBuffPtr, recvBuff + (buffPtr + TOTAL_SEQ_NUM_OFFSET), DATA_SIZE );

          if( seqNum != expectedSeq )
            receivedSequences[ expectedSeq++ ] = 0;
          else
            receivedSequences[ expectedSeq++ ] = 1;

          /* for this test case, expecting REQUEST_PACKET data structures */
          /*
          memcpy( &rp, seqBuff, sizeof( rp ) );
          */
          fprintf( stdout, "Received SeqNum: %d, TotalSeq: %d\n", seqNum, totalSeq );
          fflush ( stdout );

          buffPtr += MSG_CHUNK_SIZE;
          seqBuffPtr += DATA_SIZE;
        }
      }

      /*
      fprintf( stdout, "error is: %d\n", error ); fflush( stdout );
      if( error != 0 )
        loop = 0;
      */
      loop++;
    }
  
    //free( seqBuff );
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

    /* for this test case, expecting REQUEST_PACKET data structures */
    for( i = 0, seqBuffPtr = 0; i < 10; i++, seqBuffPtr += sizeof( rp ) )
    {
      memcpy( &rp, seqBuff + seqBuffPtr, sizeof( rp ) );
      fprintf( stdout, "Opcode: %d, ID: %d\n", rp.opcode, rp.id );
      fflush ( stdout );
    }
}
