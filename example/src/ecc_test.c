#include <stdio.h>
#include <string.h>

/* This is a test of the [7,4,3] Linear Error-Correcting Code
   (i.e. [7,4] Hamming Code).
  
   This code requires a generator matrix G and a parity-check
   matrix H.  Operations are done modulo 2.
  
 */

#define N 7
#define K 4
#define D 3


typedef unsigned char byte;


/* generator matrix  */
byte G [K][N] = { { 1, 0, 0, 0, 1, 1, 0 }, 
                  { 0, 1, 0, 0, 1, 0, 1 },
                  { 0, 0, 1, 0, 0, 1, 1 },
                  { 0, 0, 0, 1, 1, 1, 1 } };

/* parity-check matrix  */
byte H [D][N] = { { 1, 1, 0, 1, 1, 0, 0 }, 
                  { 1, 0, 1, 1, 0, 1, 0 },
                  { 0, 1, 1, 1, 0, 0, 1 } };



void printG ()
{
  int i, j;
  printf ("G = \n");
  for ( i=0; i<K; i++ )
  {
    for ( j=0; j<N; j++ )
      printf ( "%d ", G [i][j] ) ;
    printf ("\n") ;
  }
  printf("\n");
}


void printH ()
{
  int i, j;
  printf ("H = \n");
  for ( i=0; i<D; i++ )
  {
    for ( j=0; j<N; j++ )
      printf ( "%d ", H [i][j] ) ;
    printf ("\n") ;
  }
  printf("\n");
}


void printm (byte *msg)
{
  int k;
  
  printf("msg = ");
  for ( k=0; k<K; k++ )
    printf ("%d", msg [k] );
  printf("\n");
  
}


void printc (byte *codeword)
{
  int n;
  
  printf("codeword = ");
  for ( n=0; n<N; n++ )
    printf ("%d", codeword [n] );
  printf("\n");
  
}


/* H x G' - multiply H by G transpose modulo 2 */
void H_x_Gt ()
{
  int d, n, k;
  byte result [D][K] ;
  
  /* matrix multiplication */
  for ( d=0; d<D; d++ )
  {
    for ( k=0; k<K; k++)
    {
      result [d][k] = 0 ;
      for ( n=0; n<N; n++ )
        result [d][k] ^= ( H [d][n] & G [k][n] ) ;
    }
  }
  
  /* printout */
  printf ("HxG' = \n") ;
  for ( d=0; d<D; d++ )
  {
    for ( k=0; k<K; k++ )
      printf ( "%d ", result [d][k] ) ;
    printf ("\n") ;
  }
  printf ("\n") ;

}


void make_message(byte *msg, byte i)
{
  int j, remaining, div;
  
  memset ( msg, 0, sizeof (msg) ) ;
  remaining = i;
  div = 8;
  for (j=(K-1); j>0; j--)
  {
    if(remaining/div)
    {
      msg [j] = 1;
      remaining -= div;
    }
    div >>= 1;
  }
  msg [0] = i % 2 ;

}



/* codeword = encode(msg) */
void encode ( /*in*/ byte *msg, /*out*/ byte *codeword )
{
  int n, k;
  /* matrix multiplication */
  for ( n=0; n<N; n++ )
  {
    codeword [n] = 0 ;
    for ( k=0; k<K; k++)
      codeword [n] ^= ( msg [k] & G [k][n] ) ;
  }
  
  /* printout */
  printf ("codeword = encode(") ;
  for ( k=0; k<K; k++ )
    printf ( "%d", msg [k] ) ;
  printf (") = ") ;
  for ( n=0; n<N; n++ )
    printf ( "%d", codeword [n] ) ;
  
}


/* codeword = encode(msg) */
void decode ( /*in*/ byte *codeword, /*out*/ byte *msg )
{
  int n, k;
  /* matrix multiplication */
  for ( n=0; n<N; n++ )
  {
    codeword [n] = 0 ;
    for ( k=0; k<K; k++)
      codeword [n] ^= ( msg [k] & G [k][n] ) ;
  }
  
  /* printout */
  printf ("codeword = encode(") ;
  for ( k=0; k<K; k++ )
    printf ( "%d", msg [k] ) ;
  printf (") = ") ;
  for ( n=0; n<N; n++ )
    printf ( "%d", codeword [n] ) ;
  
}



int main()
{
  byte i;
  byte      msg [K] ;
  byte codeword [N] ;
  
  /* prelims */
  printG () ;
  printH () ;
  
  /* test H and G for correctness */
  H_x_Gt  () ;
  
  
  for (i=0; i<K*K; i++)
  {
    
    /* choose msg (actually do all possible 4-bit combos) */
    make_message (msg, i) ;
    
    /* encode msg */
    encode ( msg, codeword ) ;
    //printf (",          ") ;
    //decode ( codeword, msg ) ;
    printf ("\n") ;
    
  }
  
  return 0;
}
