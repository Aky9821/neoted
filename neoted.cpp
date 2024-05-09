#define CTRL_KEY(k) ((k) & 0x1f)
#include <stdlib.h>
#include <sys/ioctl.h> //io control to get window size
#include <termios.h>  // terminal io interface
#include <unistd.h>   // io
#include <iostream>
#include <fstream>
#include <vector>
#include<thread>
#include <chrono>  
#include <ext/rope>
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
  int row_nums;
  int scrollVertical;
  int scrollHorizontal;
  int screenRefreshLock;
  vector<string> rows;
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
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  // if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
  //   return -1;
  // }
  // else {
  *cols = ws.ws_col;
  *rows = ws.ws_row;
  return 0;
// }
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
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttr);  // sets modified terminal  attributes TCSAFLUSH :Change attributes when output has drained; alsoflush pending input.
}

void handleCursorX() {
  int rowIndex = cf.scrollVertical + cf.cursor_y;
  if (cf.cursor_x > cf.rows[rowIndex].length()) cf.cursor_x = cf.rows[rowIndex].length();
}

void moveCursor(int key) {
  int rowIndex = cf.scrollVertical + cf.cursor_y;

  switch (key) {
  case ARROW_LEFT:
    if (cf.cursor_x != 0) {
      cf.cursor_x--;
    }
    break;
  case ARROW_RIGHT:
    if (cf.cursor_x < cf.rows[rowIndex].length()) {
      cf.cursor_x++;
    }
    break;
  case ARROW_UP:
    if (cf.cursor_y != 0) {
      cf.cursor_y--;
    }
    break;
  case ARROW_DOWN:
    if (cf.cursor_y < cf.row_nums) {
      cf.cursor_y++;
    }
  }
  handleCursorX();
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

void handleVerticalScroll() {
  if (cf.cursor_y < cf.scrollVertical) {
    cf.scrollVertical = cf.cursor_y;
  }
  if (cf.cursor_y >= cf.scrollVertical + cf.screenHeight) {
    cf.scrollVertical = cf.cursor_y - cf.screenHeight + 1;
  }
}
void handleHorizontalScroll() {
  if (cf.cursor_x < cf.scrollHorizontal) {
    cf.scrollHorizontal = cf.cursor_x;
  }
  if (cf.cursor_x >= cf.scrollHorizontal + cf.screenWidth) {
    cf.scrollHorizontal = cf.cursor_x - cf.screenWidth + 1;
  }
}

void editorDrawRows(string& buffer) {

  for (int x = 0; x < cf.screenHeight; x++) {
    int rowIndex = cf.scrollVertical + x;
    if (rowIndex < cf.screenHeight + cf.scrollVertical - 1) {
      if (x >= cf.row_nums) {
        if (x == cf.screenHeight / 3 && cf.row_nums == 0) {
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
      else {
        int rowlength = cf.rows[rowIndex].length() - cf.scrollHorizontal;
        int startIndex = cf.scrollHorizontal;
        if (rowlength < 0) rowlength = 0;
        if (rowlength > cf.screenWidth) rowlength = cf.screenWidth;
        if (rowlength > cf.rows[rowIndex].length())rowlength = cf.rows[rowIndex].length();
        if (startIndex > cf.rows[rowIndex].length()) startIndex = cf.rows[rowIndex].length();
        buffer.append(cf.rows[rowIndex].substr(startIndex, rowlength )+ "\n\r");
      }
    }
  }
  buffer.append("\x1b[H");
}

void refreshScreen() {
  handleVerticalScroll();
  handleHorizontalScroll();
  string buffer = "\x1b[? 25l"; //disable cursor
  buffer.append("\x1b[2J");
  buffer.append("\x1b[H");
  editorDrawRows(buffer);
  string cursorPosition = "\x1b[" + to_string(cf.cursor_y - cf.scrollVertical + 1) + ";" + to_string(cf.cursor_x - cf.scrollHorizontal + 1) + "H";
  buffer.append(cursorPosition);
  buffer.append("\x1b[? 25h"); //enable cursor
  const char* c = buffer.c_str(); //string to C style  char array 
  write(STDOUT_FILENO, c, buffer.length());
  buffer = "";
}

void streamReader(istream& stream, string& line) {
  getline(stream, line);
}

void readFile(string filename) {
  ifstream myfile(filename);

  if (myfile.is_open()) {
    while (myfile.good()) {
      string line = "";
      streamReader(myfile, line);
      cf.rows.push_back(line);
      cf.row_nums += 1;
    }
    myfile.close();
  }
  else {
    exceptionExit("fread");
  }

}
void handleScreenRefresh() {
  while (1) {
    if (cf.screenRefreshLock != 1) {
      getWindowSize(&cf.screenHeight, &cf.screenWidth);
      refreshScreen();
    }
    this_thread::sleep_for(chrono::seconds(3));
  }
}

int main(int argc, char* argv[]) {
  enableInputMode();
  if (argc >= 2) {
    readFile(argv[1]);
  }
  cf.cursor_x = 0;
  cf.cursor_y = 0;
  cf.scrollVertical = 0;
  cf.scrollHorizontal = 0;
  cf.screenRefreshLock = 0;
  getWindowSize(&cf.screenHeight, &cf.screenWidth);
  // thread refreshScreenThread(handleScreenRefresh);
  while (1) {
    cf.screenRefreshLock = 1;
    refreshScreen();
    cf.screenRefreshLock = 0;
    handleKeyPress();
  }  // loop to handle one byte at a time.

  return 0;
}
