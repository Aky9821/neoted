#define CTRL_KEY(k) ((k) & 0x1f)
#include <stdlib.h>
#include <sys/ioctl.h> //io control to get window size
#include <termios.h>  // terminal io interface
#include <unistd.h>   // io
#include <iostream>
using namespace std;


enum keyMap {
  ARROW_LEFT = 1005,
  ARROW_DOWN = 1001,
  ARROW_UP = 1002,
  ARROW_RIGHT = 1003
};

struct config {
  int cursor_x;
  int cursor_y;
  int screenHeight;
  int screenWidth;
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

int getWindowSize(int* rows, int* cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  }
  else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
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

void moveCursor(int key) {
  switch (key) {
  case ARROW_LEFT:
    if (cf.cursor_x != 0) {
      cf.cursor_x--;
    }
    break;
  case ARROW_RIGHT:
    if (cf.cursor_x != cf.screenWidth - 1) {
      cf.cursor_x++;
    }
    break;
  case ARROW_UP:
    if (cf.cursor_y != 0) {
      cf.cursor_y--;
    }
    break;
  case ARROW_DOWN:
    if (cf.cursor_y != cf.screenHeight - 1) {
      cf.cursor_y++;
    }
  }
}

int readKey() {
  char ch;
  read(STDIN_FILENO, &ch, 1);  // Read One byte. STDIN_FILENO : Standard input value, stdin. Its value is 0.
  if (ch == '\x1b') { // handling arrow keys
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      switch (seq[1]) {
      case 'A': return ARROW_UP;
      case 'B': return ARROW_DOWN;
      case 'C': return ARROW_RIGHT;
      case 'D': return ARROW_LEFT;
      }
    }
    return '\x1b';
  }
  return ch;
}

void handleKeyPress() {
  int ch = readKey();

  if (ch == CTRL_KEY('q')) {
    clearScreen();
    exit(0);
  }
  else if (ch == ARROW_DOWN || ch == ARROW_LEFT || ch == ARROW_RIGHT || ch == ARROW_UP) {
    moveCursor(ch);
  }
}

void editorDrawRows(string& buffer) {
  for (int x = 0; x < cf.screenHeight; x++) {
    if (x == cf.screenHeight / 3) {
      string welcomeMessage = "Welcome to Neoted!";
      int padding = (cf.screenWidth - welcomeMessage.length()) / 2;
      if (padding) {
        buffer.append("~");
        padding--;
      }
      while (padding--) buffer.append(" ");
      buffer.append(welcomeMessage);

    }
    else {
      buffer.append("~");
      if (x < cf.screenHeight - 1) { //remove last \n
        buffer.append("\r\n");
      }
    }
  }
  buffer.append("\x1b[H");
}

void refreshScreen() {
  clearScreen();
  string buffer = "\x1b[? 25l"; //disable cursor
  editorDrawRows(buffer);
  string cursorPosition = "\x1b[" + to_string(cf.cursor_y) + ";" + to_string(cf.cursor_x) + "H";
  buffer.append(cursorPosition);
  buffer.append("\x1b[? 25h"); //enable cursor
  const char* c = buffer.c_str(); //string to C style  char array 
  write(STDOUT_FILENO, c, buffer.length());
  buffer = "";
}


int main() {
  enableInputMode();
  getWindowSize(&cf.screenHeight, &cf.screenWidth);
  cf.cursor_x = 0;
  cf.cursor_y = 0;
  while (1) {
    refreshScreen();
    handleKeyPress();
  }  // loop to handle one byte at a time.

  return 0;
}
