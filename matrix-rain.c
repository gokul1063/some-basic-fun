#include <ncurses.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <string.h>


/*
 *    DEFINE
 */

#define color "\x1b"


typedef struct {
  struct termios orginanl_config;
  int height;
  int width;
  int max_height;
  int max_width;
  volatile sig_atomic_t screenSizeCH;

} system_config;

system_config screen;

typedef struct {
  char *data;
  int size;

} screenBuffer;

typedef struct{
  char character;
  int age;
  int timeToDie;
} element;


typedef struct {
  int length;
  int exist;
  int rowNum;

}matrixData;

element *screenData;
char **screenArray;
screenBuffer *screenBufferArray;
matrixData *matrix;

/*
 * predefines
 */

int screenGetSize(int *height, int *width);
void die(const char* error);
int generateRandomNumber(int low, int high);

/*
 *      HELPER FUNCTION
 */

int min(int a, int b){
  return a < b ? a : b;
}

int max(int a, int b){
  return a > b ? a : b;
}

void appendScreenBuffer(screenBuffer *array, char *str, int len){
  char *new = realloc(array->data, array->size + len);
  if (new == NULL)
    die("realloc in appendScreenBuffer");
  memcpy(&new[array->size], str, len);
  array->data = new;
  array->size += len;

}

void freeScreenBuffer(screenBuffer **array){
  free((*array)->data);
  free(array);

}

void clearScreenBuffer(screenBuffer **array){
  free((*array)->data);
  (*array)->data = NULL;
  (*array)->size = 0;
}

void freeScreenArray(){
  for (int row = 0 ; row < screen.height; row++)
    free(screenArray[row]);
}

void initScreenBuffer(screenBuffer **array){
  *array = (screenBuffer*)malloc(sizeof(screenBuffer));
  (*array)->data = NULL;
  (*array)->size = 0;
}

void die(const char* error){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(error);
  exit(1);

}

int generateRandomNumber(int low, int high){
  int result = low + (int) ((double)rand()/RAND_MAX * (high - low));
  return result;

}

void sizeChangeHandler(int signo){
  (void)signo;
  screen.screenSizeCH = 1;

}

void randomizeScreenArray(){
  for (int row = 0; row < screen.height; row++){
    for (int col = 0; col < screen.width; col++){
      screenArray[row][col] = generateRandomNumber(33, 126);
    }
  }
}

void screenArrayInit(){
  screenArray = malloc(screen.height * sizeof(char*));
  for (int row = 0 ; row < screen.height ; row++){
    screenArray[row] = (char*) malloc(screen.width * sizeof(char));
  }
}

void screenArrayUpdate(){
  clearScreenBuffer(&screenBufferArray);
  initScreenBuffer(&screenBufferArray);
  if (screenGetSize(&screen.height, &screen.width) == -1)
    die("window size");
  screenArrayInit();

  for (int row = 0; row < screen.height; row++){
    for (int col = 0; col < screen.width; col++){
      screenArray[row][col] = generateRandomNumber(33, 126);
    }
  }

}

/*
 *    MATRIX
 */
void initMatrixData(){
  matrix = malloc(sizeof(matrixData) * (screen.width));
  if (matrix == NULL)
    die("matrix declaration error");
  for (int ind = 0; ind < screen.width; ind++){

    matrix[ind].exist = 0;
    matrix[ind].rowNum = 0;
    matrix[ind].length = 0;

    int randomLen = generateRandomNumber(0,screen.height);
    if (randomLen > screen.height/3){
      matrix[ind].exist = 1;
      matrix[ind].rowNum = 0;
      matrix[ind].length = randomLen;
    }
  }

}

int matrixUpdate(){
  for (int col = 0; col < screen.width; col++){
    if (!matrix[col].exist)
        continue;

    int start = matrix[col].rowNum;
    int length = matrix[col].length;

    if (matrix[col].rowNum > screen.height){
      length -= (matrix[col].rowNum - screen.height);
      start = screen.height - 1;
    }

    int row = start;
    char change[20];
    for (int e = 0; e < length; e++){
      row = start - e;

      if (row < 0 || row >= screen.height)
        continue;

      int changeLen = snprintf(change, sizeof(change), "\x1b[%d;%dH%c", row + 1, col + 1, screenArray[row][col]);

      write(STDOUT_FILENO, change, changeLen); 
    }

    matrix[col].rowNum++;

    if (matrix[col].rowNum - matrix[col].length > screen.height){
      matrix[col].exist = 0;
      continue;
    }

    int tail = max(matrix[col].rowNum - matrix[col].length, 0);

    if (tail == 0)
      continue;

    for (int e = 0; e < screen.height - tail ; e++){
      row = tail - e;

      if (row < 0 || row >= screen.height)
        continue;

      int changeLen = snprintf(change, sizeof(change), "\x1b[%d;%dH%c", row + 1, col + 1, ' ');

      write(STDOUT_FILENO, change, changeLen); 

    }

     

  }
  return 1; 

}


/*
 *    TERMINAL 
 */


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

void screenAddArray(){
  for (int row = 0; row < 4; row++){
    for (int col = 0; col < screen.width; col++){
      appendScreenBuffer(screenBufferArray, &screenArray[row][col], 1);
    }
  }

}

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
  char c;

  ssize_t nread = read(STDIN_FILENO, &c , 1);
  if (nread == 1){
    if (c == 'q'){
      screenExit();
      return c;
    }
  }
  

  return -1;
}

void screenReloadScreen(){
  struct timespec req;
  req.tv_sec = 0;
  req.tv_nsec = 166666666L;
  nanosleep(&req, NULL);
  //sleep(1);
  screenReadKey();
  int val = matrixUpdate();

}

/*
 *    INITIALIZAATION
 */

int signalInit(){
  struct sigaction sa = {0};
  memset(&sa , 0 , sizeof(sa));
  sa.sa_handler = sizeChangeHandler;
  sa.sa_flags = SA_RESTART;
  return sigaction(SIGWINCH, &sa , NULL);
}

void init(){
  write(STDOUT_FILENO, "\x1b[?1049h", 8);
  write(STDOUT_FILENO, "\x1b[?25l", 6);
  screen.screenSizeCH = 0;

  if (screenGetSize(&screen.height, &screen.width) == -1)
    die("window size");

  if (signalInit() == -1)
    die("sigaction error");

  screen.max_height = screen.height;
  screen.max_width = screen.width;

  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[2J", 4);

  srand((unsigned int) time(NULL));

  screenArrayInit();
  randomizeScreenArray();
  initScreenBuffer(&screenBufferArray);
  initMatrixData();

}

int main(int argc, char *arg[]){
  enableRawMode();
  init();

  while (1){
    if (screen.screenSizeCH == 1){
      screen.screenSizeCH = 0;
      write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
      freeScreenArray();
      screenArrayUpdate();
      initMatrixData();
    }
    screenReloadScreen();
  }
  return 0;

}
