#define CTRL_KEY(k) ((k) & 0x1f)
#include <stdlib.h>
#include <sys/ioctl.h> //io control to get window size
#include <termios.h>  // terminal io interface
#include <unistd.h>   // io
#include <iostream>
using namespace std;

struct config {
  struct termios termAttrOrigin;
};

struct config cf;

void clearScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);  // escape sequence \[2J : clear entire Screen
  write(STDOUT_FILENO, "\x1b[H", 3);  // escape sequence \[H : cursor position top
}

void exceptionExit(char* er) {
  clearScreen();
  perror(er);  // perror : descriptive error print
  exit(1);
}

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &cf.termAttrOrigin);
}

void enableInputMode() {
  struct termios termAttr;
  tcgetattr(STDIN_FILENO, &termAttr);  // gets current terminal attributes
  cf.termAttrOrigin = termAttr;
  atexit(disableRawMode);
  termAttr.c_iflag &= ~(IXON);  // disables io control ctrl s
  termAttr.c_lflag &= ~(ECHO);  // sets the terminal echo bit to zero
  termAttr.c_lflag &= ~(ICANON);  // sets the terminal Canonical mode off-> reads bytes by bytes
  termAttr.c_lflag &= ~(ISIG);    // disable interupt signals
  termAttr.c_lflag &= ~(IEXTEN);  // disables control ctrl v
  termAttr.c_oflag &= ~(OPOST);   // disables carrage-return
  termAttr.c_lflag &= ~(BRKINT);  // disables SIGINT break Ctrl-c
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttr);  // sets modified terminal  attributes TCSAFLUSH :Change        attributes when output has drained; alsoflush pending input.
}

char readKey() {
  char ch;
  read(STDIN_FILENO, &ch, 1);  // Read One byte. STDIN_FILENO : Standard input value, stdin. Its value is 0.
  return ch;
}
void handleKeyPress() {
  char ch = readKey();
  printf("%d ('%c')\r\n %d", ch, ch, CTRL_KEY('q'));
  if (ch == CTRL_KEY('q')) {
    clearScreen();
    exit(0);
  }
}

void editorDrawRows() {
  for (int x = 0; x < 24; x++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void refreshScreen() {
  clearScreen();
  editorDrawRows();
}

int main() {
  enableInputMode();
  while (1) {
    refreshScreen();
    handleKeyPress();
  }  // loop to read one byte at a time.

  return 0;
}
