#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hamming.h"
#include "better_udp_socket.h"


/* This is a test of the [7,4,3] Linear Error-Correcting Code
   (i.e. [7,4] Hamming Code).
  
   This code requires a generator matrix G and a parity-check
   matrix H.  Operations are done modulo 2.
  
 */
 /* generator matrix  */
byte G [NUM_DATA_BITS][NUM_CODE_BITS] = 
		  { { 0xff, 0,    0,    0,    0xff, 0xff, 0    }, 
                    { 0,    0xff, 0,    0,    0xff, 0,    0xff },
                    { 0,    0,    0xff, 0,    0,    0xff, 0xff },
                    { 0,    0,    0,    0xff, 0xff, 0xff, 0xff } };


/* parity-check matrix  */
byte H [NUM_PARITY_BITS][NUM_CODE_BITS] = 
                { { 0xff, 0xff, 0,    0xff, 0xff, 0,    0    }, 
                  { 0xff, 0,    0xff, 0xff, 0,    0xff, 0    },
                  { 0,    0xff, 0xff, 0xff, 0,    0,    0xff } };


/* coset leader LUT index by syndrome (i.e. syn = 001 = index 4) */
byte coset_leader [NUM_CODE_BITS+1][NUM_CODE_BITS] = 
                             { { 0, 0, 0, 0, 0, 0, 0 }, 
                               { 0, 0, 0, 0, 1, 0, 0 },
                               { 0, 0, 0, 0, 0, 1, 0 },
                               { 1, 0, 0, 0, 0, 0, 0 },
                               { 0, 0, 0, 0, 0, 0, 1 },
                               { 0, 1, 0, 0, 0, 0, 0 },
                               { 0, 0, 1, 0, 0, 0, 0 },
                               { 0, 0, 0, 1, 0, 0, 0 } } ;



char byte_count;
byte outer_msg [NUM_BITS_PER_BYTE] ;
byte codeword [NUM_BITS_PER_BYTE];


byte eccInnerFlag( byte *buff )
{
 int j, n, k;
 byte msg [NUM_BITS_PER_BYTE] ;
 byte codeword [NUM_BITS_PER_BYTE];
 byte codeword_value;

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

  return codeword_value;
  
}

int eccSendMsg( byte** buff, int size )
{
  int k, n;

  if (byte_count > NUM_CODE_BITS-1)
    byte_count = 0;

  // Calculate ECC bits
  if (byte_count == NUM_DATA_BITS)
  {
     /* matrix multiplication */
     for ( n=0; n<NUM_CODE_BITS; n++ )
     {
         codeword [n] = 0 ;
	 for ( k=0; k<NUM_DATA_BITS; k++)
	    codeword [n] ^= ( outer_msg [k] & G [k][n] ) ;
     }
  }

  if (byte_count++ < NUM_DATA_BITS)
  {
    outer_msg[byte_count] = (int)**buff;
    return 0;
  }
  else
  {
    byte* data = codeword + byte_count - NUM_DATA_BITS;
    *buff = data;
    return -1;
  }
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
#define COUNTDOWN          NUM_BLOCKS-4 

typedef struct Packet
{
  int  seqNum;
  byte buff[DATA_SIZE];
  int  buffLen;

  int  countDown;
  int  state;
} Packet;


Packet receivedPackets[ ARRAY_SIZE ];
int    receivedPacketsCount[ NUM_BLOCKS ];
bool   waiting_for_more;
int    totalSeq;

int toIndex( int seqNum )
{
    return ((seqNum/BLOCK_PACKETS)%NUM_ECC_PACKETS)*BLOCK_PACKETS+(seqNum%BLOCK_PACKETS);
}

int toBlockIndex( int seqNum )
{
    return ( seqNum / BLOCK_PACKETS ) % NUM_BLOCKS ;
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
bool eccGetMsg( byte** buff, int* buff_len, int* seqNum )
{
    bool have_waiting_packets = false;

    // return the first ready packet
    int i;
    for( i=0; i<ARRAY_SIZE; i++ )
    {
      if( receivedPackets[i].state == STATE_READY )
      {
        int old_seqNum = receivedPackets[i].seqNum;
        if( i%BLOCK_PACKETS<NUM_DATA_PACKETS )
        {
          memcpy( buff, receivedPackets[i].buff, receivedPackets[i].buffLen );
          *buff_len = receivedPackets[i].buffLen;
          // convert sequence number to sequence number as if ecc packets weren't included
          //  NOTE: INT MATH!!
          *seqNum   = NUM_DATA_PACKETS * (old_seqNum / BLOCK_PACKETS ) +
                         ( old_seqNum % BLOCK_PACKETS ) ;
        }

        receivedPackets[i].state = STATE_SENT;

        // FIXME: doesn't account for an out of order packet where the last
        //        packet comes before another
        if( old_seqNum == totalSeq )
        {
          waiting_for_more = false;
        }

        //if sent whole block, then reset it
        int j;
        bool all_sent = true;
        for( j=toBlockStartIndex(old_seqNum); j<toBlockEndIndex(old_seqNum); j++ )
          if( receivedPackets[j].state != STATE_SENT )
            all_sent = false;

        if( all_sent )
        {
          for( j=toBlockStartIndex(old_seqNum); j<toBlockEndIndex(old_seqNum); j++ )
            receivedPackets[j].state = STATE_NOT_RECEIVED;
          receivedPacketsCount[toBlockIndex(old_seqNum)] = 0;
        }

        // if a data packet, return it
        if( i%BLOCK_PACKETS<NUM_DATA_PACKETS )
          return true;
      }
      else if( receivedPackets[i].state == STATE_WAITING )
      {
        have_waiting_packets = true;
      }
    }

    // we didn't find a ready packet but are we waiting for more
    *buff_len = 0;
    return have_waiting_packets || waiting_for_more ;
}



/* multiplies received vector by H to give syndrome */
void compute_syndrome (int idx, byte *syndrome) //(byte *received, byte *syndrome)
{
  int n, d;
  /* matrix multiplication */
  for ( d=0; d<NUM_PARITY_BITS; d++ )
  {
    syndrome [d] = 0 ;
    for ( n=0; n<NUM_CODE_BITS; n++)
      syndrome [d] ^= ( receivedPackets [idx + n].buff[0] & H [d][n] ) ;
    // probably needs to be the following
    //for (j=0; j<DATA_SIZE; j++) 
    //  syndrome [j][d] ^= ( receivedPackets [idx + n].buff[j] & H [d][n] ) ;
  }
  
  
}

/* Performs error-correction
 
 */
void eccCorrect( int block_idx )
{
  int i = toBlockStartIndex (block_idx * BLOCK_PACKETS) ;
  byte syndrome [NUM_PARITY_BITS] ;
  byte e ;
  byte n ;
  byte k ;
  byte b ;
  byte s [NUM_PARITY_BITS] ;
  byte c [NUM_CODE_BITS] ;
  byte r [NUM_CODE_BITS] ;
  
  /* calc syndrome */
  compute_syndrome ( i, syndrome ) ;
  
   
  /* codeword = r + e, and msg = codeword [1:K] */
  for ( n=0; n<NUM_CODE_BITS; n++ )
  {
    c [n] = 0 ;
    for (b=0; b< NUM_BITS_PER_BYTE; b++)
    {
       r [n] = ( receivedPackets [i+n] . buff[0] >> b ) & 0x01 ;
       s [0] = ( syndrome [0] >> b ) & 0x01 ;
       s [1] = ( syndrome [1] >> b ) & 0x01 ;
       s [2] = ( syndrome [2] >> b ) & 0x01 ;
       e = coset_leader [ s [0] | (s [1] << 1) | (s [2] << 2) ][n] ;
       c [n] ^= ( ( r [n] ^ e ) << b ) & ( 0x01 << b) ;
    }
    
    receivedPackets [i + n] . buff[0] = c [n] ;
    
    //if ((n<4) && (receivedPackets[ i + n ].state == STATE_WAITING))
    //  receivedPackets[ i + n ].state = STATE_RECEIVED ;
  }
}



/* Start tracking a new packet fromt the net.
 
   This will:
     1) initialize countdown timers for missed packets.
     2) Reorder out of order packets.
     3) Correct corrupted/missing packets
 */
void eccReceiveMsg( int seqNum, int _totalSeq, byte** buff, int buffLen )
{
    seqNum--;
    _totalSeq--;

    // find the position for this packet in the receivedPackets array
    int pos = toIndex( seqNum );
    int block_idx = toBlockIndex( seqNum );

    // sanity check, should rarely hit this, posible if COUNTDOWN number of
    // of blocks are dropped, if so for now just ignore (drop) the incoming packet
    if( receivedPackets[ pos ].state != STATE_WAITING &&
        receivedPackets[ pos ].state != STATE_NOT_RECEIVED )
    {
      printf( "eccReceiveMsg() - error receiving packet(%i).  Already packet in array!\n",seqNum );
      return;
    }
   
    receivedPackets[ pos ].seqNum    = seqNum;
    memcpy( receivedPackets[ pos ].buff, *buff, buffLen );
    receivedPackets[ pos ].buffLen   = buffLen;
    receivedPackets[ pos ].countDown = -1;

    totalSeq = _totalSeq > totalSeq ? _totalSeq : totalSeq;

    receivedPacketsCount[ block_idx ]++;

    // if this is the first packet of the block to be received, mark the block
    // as waiting and decrement the countdown timer on all others
    int i;
    //if( receivedPackets[ pos ].state == STATE_NOT_RECEIVED )
    if( receivedPacketsCount[ block_idx ] == 1 )
    {
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
        {
          receivedPackets[ i ].state = STATE_READY;
        }
      }
    }

    // if this is a data packet then go ahead and mark it ready
    //  we are assuming the lower layers of the network stack are correcting for
    //  corrupted packets
    receivedPackets[ pos ].state     = STATE_READY;

    // CARSON/BILLY TODO: add ecc correction here!!!
    if( receivedPacketsCount[ block_idx ] == 6 )
    {
      eccCorrect( block_idx );  //  do ecc correction for final packet
      //receivedPacketsCount[ block_idx ]++;
      //int end = ( totalSeq - seqNum ) <= ( totalSeq % 7 ) ? 
      //                  toIndex(totalSeq)+1 : 
      //                  toBlockEndIndex(seqNum);
      //pos = end;
    }

    // for all packets in this group before the current sequence number, if we haven't
    // received the packet, start the count down timer for it
    for( i = toBlockStartIndex( seqNum ); i<pos; i++ )
      if( receivedPackets[i].state == STATE_WAITING &&
          receivedPackets[i].countDown < 0  )
        receivedPackets[i].countDown = COUNTDOWN;
}
