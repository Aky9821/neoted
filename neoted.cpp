#include <unistd.h> //
#include <termios.h> // terminal io interface
#include <iostream>
#include <stdlib.h>
using namespace std;

struct termios termAttrOrigin;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttrOrigin);
}


void enableInputMode(){
  struct termios termAttr;
  tcgetattr(STDIN_FILENO, &termAttr); // gets current terminal attributes
  termAttrOrigin = termAttr;
  atexit(disableRawMode);
  termAttr.c_lflag &= ~(ECHO); // sets the terminal echo bit to zero
  termAttr.c_lflag &= ~(ICANON); // sets the terminal Canonical mode off-> reads bytes by bytes
  termAttr.c_lflag &= ~(ISIG); // disable interupt signals 
  termAttr.c_lflag &= ~(IXON); //disables io control ctrl s
  termAttr.c_lflag &= ~(IEXTEN); //disables io control ctrl v
  termAttr.c_lflag &= ~(OPOST); // disables carrage-return
  termAttr.c_lflag &= (BRKINT); // disables SIGINT break Ctrl-c

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttr); //sets modified terminal  attributes TCSAFLUSH :Change attributes when output has drained; also flush pending input.
  
} 

int main() {
  enableInputMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1){
    // printf("%d ('%c')\n", c, c);
  } //loop to read one byte at a time. STDIN_FILENO : Standard input value, stdin. Its value is 0.
  
  return 0;
}