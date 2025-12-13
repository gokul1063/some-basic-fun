#define NANO_SLEEP 35
#define SYS_WRITE 1
#define SYS_EXIT 60

typedef struct {
  long tv_sec;;
  long tv_nanosec;
} timespec;

long syscall1(long number, long arg1){
  long result;

  __asm__ __volatile__(
      "syscall" 
      : "=a"(result) 
      : "a"(number), "D"(arg1)
      : "rcx", "r11", "memory"
    );
  return result;

}

long syscall2(long number, long arg1, long arg2) {
  long result;

  __asm__ __volatile__(
      "syscall" 
      : "=a"(result) 
      : "a"(number), "D"(arg1), "S"(arg2)
      : "rcx", "r11", "memory"
    );
  return result;

}

long syscall3(long number, long arg1, long arg2, long arg3){
  long result;

  __asm__ __volatile__(
      "syscall" 
      : "=a"(result) 
      : "a"(number), "D"(arg1), "S"(arg2), "d"(arg3)
      : "rcx", "r11", "memory"
    );
  return result;

}

long parse_int(char *raw){
  int result = 0;

  char *cursor = raw;

  while (*cursor >= '0' && *cursor <= '9'){
    result = result * 10 + (*cursor - '0');
    cursor++;

  }
  return result;

}

long unsigned strlen(char *input){
  char *cursor= input;
  while(*cursor)
    cursor ++;
  long unsigned result = cursor - input;
  return result;

}

void print(char *input){
  syscall3(SYS_WRITE , 1, (long)input, strlen(input));

}

void sleep(long second){
  timespec duration = {0};
  duration.tv_sec = second;
  syscall2(NANO_SLEEP, (long)&duration, 0);

}

int main(int argc, char *arg[]){

  if (argc != 2){

    print("The time is not mentione\n pause for null section");
    return 1;

  }

  char *raw = arg[1];
  long seconds = parse_int(raw);
  print("sleeping : ");
  print(raw);
  print(" seconds \n");
  sleep(seconds);
  return 0;

}

void exit(int exit_code){
  syscall1(SYS_EXIT , exit_code);
  for(;;){}
}

__attribute__((naked))
void _start(){
  __asm__ __volatile__(
      "xor %ebp, %ebp\n"
      "mov (%rsp), %rdi\n"
      "lea 8(%rsp), %rsi\n"
      "and $-16, %rsp\n"
      "call main\n"
      "mov %rax, %rdi\n"
      "call exit\n"
    );
}


