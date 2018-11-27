#include <spede/stdio.h>
#include <spede/flames.h>

DisplayMsg(long i){
 printf( "%d Hello world %d \nECS", i, 2 * i);
 cons_printf ( "--> Hello World <--\nCPE/CSC" );
}

int main(void)
{

  long i;
  for(i=111; i < 116; i++)
  {
    DisplayMsg(i);
  }
  
  return 0;
}
