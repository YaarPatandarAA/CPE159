// tools.c, 159
#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h"

// clear DRAM by setting each byte to zero
void MyBzero(char *p, int size) {
  int i;
  for(i=0; i < size; i++){
    *((char*)p+i) = '\0';
  }
}

// dequeue, return 1st element in array, and move all forward // if queue empty, return -1
int DeQ(pid_q_t *p) {
  int i,  element = -1;

  if(p->size == 0) return element;
  element = p->q[0];
  p->size -= 1;

  for(i = 0; i < (p->size); i++){
    p->q[i] = p->q[i+1];
  }

  return element;
}

// enqueue element to next available position in array, 'size' is array index
void EnQ(int element, pid_q_t *p) {
  if(p->size==Q_SIZE){
    cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
    return;
  }

  p->q[p->size] = element;
  p->size += 1;
}

void MyStrcpy(char *dst, char *src){
  while(*src != '\0'){
    *dst++ = *src++;
  }
  *dst = '\0';
}

void MVStr(char *str){
  int i=0;
  while(str[i] != '\0'){
    str[i] = str[i+1];
    i++;
  }
}

void APNDStr(char ch, char *kb){
  int len = strlen(kb);

  kb[len] = ch;
  kb[len+1] = '\0';
}

int MyStrcmp(char *s1, char *s2){
  int index;
  
  if(strlen(s1) != strlen(s2)) return 0;
  
  for(index = 0; index <strlen(s1); index++){  
    if (s1[index] != s2[index]) return 0;
  }
    
    return 1;
  }

void MyMemcpy(char *dst, char *src, int bytes){
  while(bytes != 0){
    *dst++ = *src++;
    bytes--;
  }
}
