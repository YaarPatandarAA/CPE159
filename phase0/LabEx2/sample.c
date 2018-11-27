#include <spede/stdio.h>
#include <spede/flames.h>

int main(void)
{

  long i;

  i = 128;
  printf( "%d Hello world %d \nECS", i, 2 * i);
  cons_printf ( "--> Hello World <--\nCPE/CSC" );
  return 0;

}
