#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hamming.h"
#include "better_udp_socket.h"

typedef int bool;
#define true 1
#define false 0

/* This is a test of the [7,4,3] Linear Error-Correcting Code
   (i.e. [7,4] Hamming Code).
  
   This code requires a generator matrix G and a parity-check
   matrix H.  Operations are done modulo 2.
  
 */

byte eccInnerFlag( char *buff)
{
 int j, n, k;
 byte msg [NUM_BITS_PER_BYTE] ;
 byte codeword [NUM_BITS_PER_BYTE];
 byte codeword_value;
 /* generator matrix  */
 byte G [NUM_DATA_BITS][NUM_CODE_BITS] = { { 1, 0, 0, 0, 1, 1, 0 }, 
                  { 0, 1, 0, 0, 1, 0, 1 },
                  { 0, 0, 1, 0, 0, 1, 1 },
                  { 0, 0, 0, 1, 1, 1, 1 } };
 
  for( j=0; j<NUM_BITS_PER_BYTE; j++ )
      msg[j] = ( *buff >> j ) & 0x1;
 
  /* matrix multiplication */
  for ( n=0; n<NUM_CODE_BITS; n++ )
  {
    codeword [n] = 0 ;
    for ( k=0; k<NUM_DATA_BITS; k++)
      codeword [n] ^= ( msg [k] & G [k][n] ) ;
  }
  codeword_value = 0;
  for( j=0; j<NUM_BITS_PER_BYTE; j++ )
      codeword_value = codeword_value + ( (codeword)[j] << ((NUM_CODE_BITS-1)-j) );

  
  /* printout */
//  printf ("eccInnerFlag    ") ;
//  printf ("buff = %d= ",*buff);
//  printf ("codeword = encode(") ;
//  for ( k=0; k<NUM_DATA_BITS; k++ )
//    printf ( "%d", msg [k] ) ;
//  printf (") = ") ;
//  for ( n=0; n<NUM_CODE_BITS; n++ )
//    printf ( "%d", codeword [n] ) ;
//  
//  printf ("    Codeword Value = %d   ", codeword_value);
//  printf ("eccInnerFlag Done   ") ;
    
  return codeword_value;
  
}

/* no packet has been received in the block */
const int STATE_NOT_RECEIVED = 0;
/* we received a packet from the block, so we must be waiting on this one */
const int STATE_WAITING      = 1;
/* received the packet */
const int STATE_RECEIVED     = 2;
/* finished processing the packet, is ready to be sent up the chain */
const int STATE_READY        = 3;
/* sent the packet up the chain */
const int STATE_SENT         = 4;

#define NUM_ECC_PACKETS    3
#define NUM_DATA_PACKETS   4
#define BLOCK_PACKETS      ( NUM_ECC_PACKETS + NUM_DATA_PACKETS )
#define NUM_BLOCKS         10
#define ARRAY_SIZE         ( NUM_BLOCKS * BLOCK_PACKETS )

typedef struct Packet
{
  int  seqNum;
  char eccFlag;
  char buff[DATA_SIZE];
  int  buffLen;

  int  countDown;
  int  state;
} Packet;


Packet receivedPackets[ ARRAY_SIZE ];
bool  waiting_for_more;
int  totalSeq;

int toIndex( int seqNum )
{
    return ((seqNum/BLOCK_PACKETS)%NUM_ECC_PACKETS)*BLOCK_PACKETS+(seqNum%BLOCK_PACKETS);
}

int toBlockStartIndex( int seqNum )
{
    return ((seqNum/BLOCK_PACKETS)%NUM_ECC_PACKETS)*BLOCK_PACKETS;
}

/* return the end index + 1 of the block the given seqNum is in
 */
int toBlockEndIndex( int seqNum )
{
    return ((seqNum/BLOCK_PACKETS)%NUM_ECC_PACKETS)*BLOCK_PACKETS+BLOCK_PACKETS;
}

void eccStartReceive() 
{ 
    waiting_for_more = true; 
    totalSeq = 0;
    
    int i;
    for( i=0; i<ARRAY_SIZE; i++ )
    {
      receivedPackets[i].state = STATE_NOT_RECEIVED;
      receivedPackets[i].countDown = -1;
    }
}

bool eccEOM()
{
  return !waiting_for_more;
}

/* Return a packet that is ready to be pushed up to the app.
 */
int eccGetMsg( char** buff, int* buff_len, int* seqNum )
{
    bool have_waiting_packets = false;

    // return the first ready packet
    int i;
    for( i=0; i<ARRAY_SIZE; i++ )
    {
      if( receivedPackets[i].state == STATE_READY )
      {
        memcpy( buff, receivedPackets[i].buff, receivedPackets[i].buffLen );
        *buff_len = receivedPackets[i].buffLen;
        *seqNum   = receivedPackets[i].seqNum;
        //printf( "getting packet %i, seq = %i\n", i, *seqNum );


        receivedPackets[i].state = STATE_SENT;

        // FIXME: doesn't account for an out of order packet where the last
        //        packet comes before another
        if( *seqNum == totalSeq )
        {
          waiting_for_more = false;
          //printf( "sending last packet\n" );
        }

        //if sent whole block, then reset it
        int j;
        bool all_sent = true;
        for( j=toBlockStartIndex(*seqNum); j<toBlockEndIndex(*seqNum); j++ )
          if( receivedPackets[j].state != STATE_SENT )
            all_sent = false;
        if( all_sent )
          for( j=toBlockStartIndex(*seqNum); j<toBlockEndIndex(*seqNum); j++ )
            receivedPackets[j].state = STATE_NOT_RECEIVED;
        //if( all_sent )
          //printf( "finished block\n" );

        return true;
      }
      else if( receivedPackets[i].state == STATE_WAITING )
      {
        have_waiting_packets = true;
      }
    }

    *buff_len = 0;
    //if( waiting_for_more )
    //  printf( "waiting for more packets\n" );
    //if( have_waiting_packets )
    //  printf( "packets are waiting to be sent\n" );
    return have_waiting_packets || waiting_for_more ;
}

/* Start tracking a new packet fromt the net.
 
   This will:
     1) initialize countdown timers for missed packets.
     2) Reorder out of order packets.
     3) Correct corrupted/missing packets
 */
void eccReceiveMsg( int seqNum, int _totalSeq, char eccFlag, char** buff, int buffLen )
{
    //printf( "eccReceiveMsg( %i, %i )\n", seqNum, _totalSeq );

    seqNum--;
    _totalSeq--;

    // find the position for this packet in the receivedPackets array
    int pos = toIndex( seqNum );

    // sanity check, we should never hit this thanks to the countdown timer
    if( receivedPackets[ pos ].state != STATE_WAITING &&
        receivedPackets[ pos ].state != STATE_NOT_RECEIVED )
      printf( "eccReceiveMsg() - error receiving packet(%i).  Already packet in array!\n",seqNum );
   
    //printf( "packet pos - %i\n", pos );
    receivedPackets[ pos ].seqNum    = seqNum;
    receivedPackets[ pos ].eccFlag   = eccFlag;
    memcpy( receivedPackets[ pos ].buff, *buff, buffLen );
    //printf( "    packet data = %i,%i\n", (*buff)[0], receivedPackets[ pos ].buff[0] );
    receivedPackets[ pos ].buffLen   = buffLen;
    receivedPackets[ pos ].countDown = -1;

    totalSeq = _totalSeq > totalSeq ? _totalSeq : totalSeq;

    // if this is the first packet of the block to be received, mark the block
    // as waiting and decrement the countdown timer on all others
    // FIXME: this is a really simplistic approach to the countdown timer. It is more
    //        of a memory used timer rather than a quantum timer.  It allows us to make
    //        sure we don't have to store too many packets at any one point in time. We 
    //        probably want to make this a pure quantum timer. Would require the use of
    //        threads!!
    int i;
    if( receivedPackets[ pos ].state == STATE_NOT_RECEIVED )
    {
      //printf( "first packet of block\n" );
      // account for the possibility that we might not be using all packets of the final block
      int end = ( totalSeq - seqNum ) <= ( totalSeq % 7 ) ? 
                        toIndex(totalSeq)+1 : 
                        toBlockEndIndex(seqNum);
      for( i = toBlockStartIndex( seqNum ); i<end; i++ )
      {
        if( i != pos )
        {
          // initialize the other packets
          receivedPackets[ i ].state     = STATE_WAITING;
          receivedPackets[ i ].countDown = 0;
          memset( receivedPackets[ i ].buff, 0, DATA_SIZE );
          receivedPackets[ i ].buffLen   = DATA_SIZE;
          receivedPackets[ i ].seqNum    = BLOCK_PACKETS * ( seqNum / BLOCK_PACKETS ) + ( i % BLOCK_PACKETS );
        }
      }

      // decrement countdown, if we reach 0, then go ahead and forcibly mark packet as ready
      for( i = 0; i < 21; i++ )
      {
        if( receivedPackets[ i ].state == STATE_WAITING )
          receivedPackets[ i ].countDown--;
        if( receivedPackets[ i ].countDown == 0 )
          receivedPackets[ i ].state = STATE_READY;
      }
    }

    // TODO: correct this or previous packets if possible here
    // for now mark this packet ready
    receivedPackets[ pos ].state     = STATE_READY;

    //printf( "setting count downs\n" );
    // for all packets in this group before the current sequence number, if we haven't
    // received the packet, start the count down timer for it
    for( i = toBlockStartIndex( seqNum ); i<pos; i++ )
      if( receivedPackets[i].state == STATE_WAITING &&
          receivedPackets[i].countDown < 0  )
        receivedPackets[i].countDown = NUM_BLOCKS - 4; // should allow for 3 completely missed blocks
}
