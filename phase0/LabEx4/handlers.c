//***********************************************************************************
// handlers.c, Phase 0, Exercise 4 -- Timer Event
//***********************************************************************************
#include "spede.h"

// 2-byte (unsigned short) ptr points to video memory location
// assume screen has 24 rows, 80 cols, upper-left corner (b8000)
char my_name[] = " Amarjit Singh";

int i = 0, j = 0; // which char in my name
int tick_count = 0; // count # of timer events
// position video memory location to show 1st char in name
unsigned short *char_p = (unsigned short *) 0xB8000+12*80+34; //vid mem

void TimerHandler() { //evoked from TimerEvent

  int arrSize = (sizeof(my_name) / sizeof(my_name[0]));

  if (tick_count == 0) { 
    *char_p = (0x0f00 + my_name[i]);
  }

  tick_count++;
  if (tick_count == 75) { // ever.75 seconds
    tick_count = 0;
    i++;
    char_p++;
    if (i == arrSize) {
      i = 0;

      char_p = (unsigned short *) 0xB8000+12*80+34;
      for(j=0; j<arrSize ; j++){
        char_p[j] = ' ';
      }
      //char_p rools back to location/address of begining location on screen again
      //loop on j to erase all chars shown, by char_p[j] = '' (need no color mask 0xf00)
     }
  }
  // dismiss timer event (IRQ 0), otherwise, new event from timer
  // won't be recognized by CPU since circuit uses edge-trigger flipflop
  outportb(0x20, 0x60); // 0x20 is PIC control reg, 0x60 dismissed IRQ 0
}
