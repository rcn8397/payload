#ifndef _HAMMING_H_
#define _HAMMING_H_

#define NUM_DATA_BITS       ( 4 )
#define NUM_PARITY_BITS     ( 3 )
#define NUM_CODE_BITS       ( 7 )
#define NUM_BITS_PER_BYTE   ( 8 )


typedef unsigned char byte;

byte eccInnerFlag( char *buff);
int eccAddMsg( char** buff, int size );

#endif /* _HAMMING_H_ */
