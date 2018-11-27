// tools.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h"

// clear DRAM by setting each byte to zero
void MyBzero(char *p, int size) {
  /*
     loop for 'size' number of times:
     set where p points to, to (char *)0
     increment p (by 1)*/

  int i;
  for(i=0; i < size; i++){
    //p[i] = (char *)0; //set where p points to, to (char *)0
    *((char*)p+i) = '\0';
  }
}

// dequeue, return 1st element in array, and move all forward
// if queue empty, return -1
int DeQ(pid_q_t *p) {
  int i,  element = -1;

  if(p->size == 0) return element; 	// if the size of the queue p points to is zero, return element (-1) (otherwise, continue)

  element = p->q[0]; 				//copy the 1st in the array that p points to, to element
  p->size -= 1; 					//decrement the size of the queue p points to by 1

  //move all elements in the array to the front by one position
  // ??????????????????????
  for(i = 0; i < (p->size); i++){
    p->q[i] = p->q[i+1];
  }

  return element;

}

// enqueue element to next available position in array, 'size' is array index
void EnQ(int element, pid_q_t *p) {
  /*
     if the size of the queue that p points to equals Q_SIZE {
     show on target PC: "Kernel Panic: queue is full, cannot EnQ!\n"
     return;       // alternative: breakpoint() into GDB
     }*/

  if(p->size==Q_SIZE){
    cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");	
    return;
  }

  p->q[p->size] = element; 	// copy element into the array indexed by 'size'
  p->size += 1; 			// increment 'size' of the queue p points to by 1
}
