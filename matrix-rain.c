#include <ncurses.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>

/*
 *    DEFINE
 */

typedef struct {
  struct termios orginanl_config;
  int height;
  int width;

} system_config;

system_config screen;

typedef struct{
  char character;
  int age;
  int timeToDie;
} element;

element *screenData;

/*
 *      HELPER FUNCTION
 */

double generateRandomNumber(int low, int high){
  double result = low + (double) rand()/RAND_MAX * (high - low);
  return result;

}

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
 *    Screen
 */

int screenGetSize_NonPosix(int *height, int *width){
  if ((write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)) != 12)
    return -1;

  if ((write(STDOUT_FILENO, "\x1b[6n", 4)) != 4)
    return -1;


  ssize_t i = 0;
  char buf[32];

  while (i < sizeof(buf)-1){
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }

  buf[i] = '\0';

  if (buf[0] != '\x1b' ||  buf[1] != '[') return -1;
  if ((sscanf(&buf[2], "%d;%d", height, width) != 2)) return -1; 
  
  return 0;


}

int screenGetSize(int *height, int *width){
  struct winsize screenWindowSize;
  if ((ioctl(STDIN_FILENO, TIOCGWINSZ, &screenWindowSize) == -1) || screenWindowSize.ws_col == 0){
    if ((screenGetSize_NonPosix(height, width)) == -1)
      return -1;
  } else {

    *height = screenWindowSize.ws_row;
    *width = screenWindowSize.ws_col;
  }
  return 0;
}

void screenExit(){
  disabelRawMode();
}

int screenReadKey(){
  int nread;
  char c;

  while((nread = read(STDIN_FILENO, &c , 1)) != 1){
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  
  if (c == 'q'){
    screenExit();
    return 1;
  }

  return c;
}

void screenReloadScreen(){
  char c = screenReadKey();

  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[?25l", 6);



  char data[32];
  int len = snprintf(data, sizeof(data), "height : %d, width : %d\n" , screen.height, screen.width);
  data[len] = '\0';

  write(STDOUT_FILENO, data, len );


  char data1[32];
  double randomNumber = generateRandomNumber(50,100);
  int len1 = snprintf(data1, sizeof(data), "random number : %.2f\n", randomNumber);
  write(STDOUT_FILENO, data1, len1);

}

/*
 *    INITIALIZAATION
 */


void init(){
  write(STDOUT_FILENO, "\x1b[?1049h", 8);
  if (screenGetSize(&screen.height, &screen.width) == -1)
    die("window size");
  srand((unsigned int) time(NULL));

}

int main(int argc, char *arg[]){
  enableRawMode();
  init();

  while (1){
    screenReloadScreen();
  }
  return 0;

}
