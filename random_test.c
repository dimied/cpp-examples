/* rand example: guess the number */
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
//#include <time.h>       /* time */

#define NUM_VALS 200

#define ELEMS_PER_LINE 5

#define MOD_VALUE (1000*1000*100)

int vals[NUM_VALS];

int main ()
{
  /* initialize random seed: */
  srand (213);

  int val;
  int c;
  
  printf("MAX: %d\n", RAND_MAX);

  for(int i=1; i <= NUM_VALS; i++) {
    val = rand() % MOD_VALUE;

    c=(i>0&&(i%ELEMS_PER_LINE==0))?0xA:' ';
    
    printf ("%d%c", val, c);   
  }
  printf("\n");

  return 0;
}
