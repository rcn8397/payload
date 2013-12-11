#ifndef _HAMMING_H_
#define _HAMMING_H_

#define NUM_DATA_BITS       ( 4 )
#define NUM_PARITY_BITS     ( 3 )
#define NUM_CODE_BITS       ( 7 )
#define NUM_BITS_PER_BYTE   ( 8 )

typedef int bool;
#define true 1
#define false 0

typedef unsigned char byte;

byte eccInnerFlag( byte *buff );
int eccSendMsg( byte** buff, int size );

/** eccStartReceive() - let the ecc processor know we are 
      starting to receive a message so it can initialize
      internal data structures
**/
void eccStartReceive();

/** return true if we have received all packets in this message
**/
bool eccEOM();

/** pass a packet from the net to the ecc processor
  
    \param seqNum sequence of packet to add, 1 based
    \param totalSeq total number of packets to expect, 1 based
**/
void eccReceiveMsg( int seqNum, int totalSeq,
                    byte** buffer, int bufferSize ); 

/** grab a processed packet back from the ecc processor
    \return true if there are more packets to process or !eccEOM(),
           false otherwise
**/
bool eccGetMsg( byte** buff, int* buff_len, int* seqNum );
#endif /* _HAMMING_H_ */
