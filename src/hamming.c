#include <stdio.h>
#include <string.h>

#include "hamming.h"


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
