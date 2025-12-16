#include <ncurses.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

/*
 *    DEFINE
 */

typedef struct {
  struct termios orginanl_config;

}system_config;

system_config screen;




/*
 *    TERMINAL 
 */

void die(const char* error){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(error);
  exit(1);

}

void disabelRawMode(){
  if (tcsetattr(STDIN_FILENO, TCSANOW, &screen.orginanl_config) == -1)
    die("tcsetattr");
  write(STDOUT_FILENO, "\x1b[?25h", 6);
  write(STDOUT_FILENO, "\x1b[?1049l", 8);
  exit(0);

}

void enableRawMode(){
  struct termios raw;

  if (tcgetattr(STDIN_FILENO, &raw) == -1){
    die("getattr");
  }

  screen.orginanl_config = raw;
  atexit(disabelRawMode);
  raw.c_iflag &= ~(IXON);
  //raw.c_iflag &= ~(BRKINT);
  raw.c_oflag &= ~(OPOST | OLCUC | OCRNL);
  raw.c_cflag |= (CS8 | CREAD | CLOCAL);
  raw.c_lflag &= ~(ECHO | IEXTEN | ISIG | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;


  if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
    die("tcsetattr");

}

/*
 *    EDITOR
 */

void editorExit(){
  disabelRawMode();
}

int editorReadKey(){
  int nread;
  char c;

  while((nread = read(STDIN_FILENO, &c , 1)) != 1){
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  
  if (c == 'q'){
    editorExit();
    return 1;
  }

  return c;
}

void editorReloadScreen(){
  char c = editorReadKey();

  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[?25l", 6);
}


int main(int argc, char *arg[]){
  enableRawMode();
  write(STDOUT_FILENO, "\x1b[?1049h", 8);

  while (1){
    editorReloadScreen();
  }
  return 0;

}
