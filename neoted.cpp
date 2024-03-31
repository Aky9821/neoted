#include <unistd.h>
#include <termios.h>
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
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttr); //sets modified terminal  attributes
  
} 

int main() {
  enableInputMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1); // STDIN_FILENO : Standard input value, stdin. Its value is 0.
  return 0;
}