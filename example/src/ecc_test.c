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


/* coset leader LUT index by syndrome (i.e. syn = 001 = index 4) */
byte coset_leader [N+1][N] = { { 0, 0, 0, 0, 0, 0, 0 }, 
                               { 0, 0, 0, 0, 1, 0, 0 },
                               { 0, 0, 0, 0, 0, 1, 0 },
                               { 1, 0, 0, 0, 0, 0, 0 },
                               { 0, 0, 0, 0, 0, 0, 1 },
                               { 0, 1, 0, 0, 0, 0, 0 },
                               { 0, 0, 1, 0, 0, 0, 0 },
                               { 0, 0, 0, 1, 0, 0, 0 } } ;


/* prints the generator matrix G, which converts msgs to codewords */
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


/* prints the parity-check matrix G, which is used to compute syndromes */
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


/* prints the given msg */
void printm (byte *msg)
{
  int k;
  
  printf("msg = ");
  for ( k=0; k<K; k++ )
    printf ("%d", msg [k] );
  printf("\n");
  
}


/* prints the given codeword */
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


/* makes a msg based on i */
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


/* multiplies received vector by H to give syndrome */
void compute_syndrome (byte *received, byte *syndrome)
{
  int n, d;
  /* matrix multiplication */
  for ( d=0; d<D; d++ )
  {
    syndrome [d] = 0 ;
    for ( n=0; n<N; n++)
      syndrome [d] ^= ( received [n] & H [d][n] ) ;
  }
  
  /* printout */
  printf ("syndrome = compute_syndrome(") ;
  for ( n=0; n<N; n++ )
    printf ( "%d", received [n] ) ;
  printf (") = ") ;
  for ( d=0; d<D; d++ )
    printf ( "%d", syndrome [d] ) ;
  
}


/* msg = decode(received) */
void decode ( /*in*/ byte *received, /*out*/ byte *msg )
{
  byte syndrome [D] ;
  byte codeword [N] ;
  byte e ;  /* e is a index to the error vector */
  byte n ;
  byte k ;
  
  /* calc syndrome */
  compute_syndrome (received, syndrome) ;
  
  /* codeword = r + e, and msg = codeword [1:K] */
  for ( n=0; n<N; n++ )
  {
    codeword [n] = received [n] ^ coset_leader [    syndrome [0] 
                                                 | (syndrome [1] << 1)
                                                 | (syndrome [2] << 2) ][n] ;
    if (n<K)
      msg [n] = codeword [n] ;
  }
  
  /* printout */
  printf (",   ") ;
  printf ("msg = decode(") ;
  for ( n=0; n<N; n++ )
    printf ( "%d", received [n] ) ;
  printf (") = ") ;
  for ( k=0; k<K; k++ )
    printf ( "%d", msg [k] ) ;
  
}



/* main program entry point */
int main()
{
  byte i, j;
  byte      msg [K] ;
  byte codeword [N] ;
  byte received [N] ;
  byte syndrome [D] ;
  
  /* prelims */
  printG () ;
  printH () ;
  
  /* test H and G for correctness */
  H_x_Gt  () ;
  
  
  /* these are all codewords, so syndrome should be all-zero vector */
  for (i=0; i<K*K; i++)
  {
    /* choose msg (actually do all possible 4-bit combos) */
    make_message (msg, i) ;
    
    /* encode msg */
    encode ( msg, codeword ) ;
    printf (",   ") ;
    decode ( codeword, msg ) ;
    printf ("\n") ;
    
  }
  printf ("\n") ;

  /* now let's test correction of one error in received vector */
  printf ("testing ECC...\n") ;
  i = 15 ;
  make_message (msg, i) ;
  encode ( msg, codeword ) ;
  printf ("\n") ;
  for (i=0; i<=N; i++)
  {
    /* send it thru noisy channel (causing an error) */
    for (j=0; j<N; j++)
      received [j] = codeword [j] ^ coset_leader [i][j] ;
    
    /* determine syndrome */
    decode ( received, msg ) ;
    printf ("\n") ;
    
  }
  printf ("\n") ;
  
  return 0;
}
