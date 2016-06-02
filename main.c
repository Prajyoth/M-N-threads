#include "scheduler.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void print_nth_prime(void * pn) {
  int n = *(int *) pn;
  int c = 1, i = 1;
  while(c <= n) {
    ++i;
    int j, isprime = 1;
    for(j = 2; j < i; ++j) {
      if(i % j == 0) {
        isprime = 0;
        break;
      }
    }
    if(isprime) {
      ++c;
    }
    yield();
  }
  printf("%dth prime: %d\n", n, i);
 
}

void count(void * num) {
int n = *(int *) num;
int i = 0;
int j=0;
for (i=0; i< n ;  i++)
{
	for (j = 0; j <n; j++)
	{}
	yield();
}
printf("Counted to %d X %d times\n",n,n);
}

void handleread(void * arg) {
char *buffer;
buffer = (char *)malloc(sizeof(char)*1000);
int fd;
fd = STDIN_FILENO;

ssize_t ret = read_wrap(fd, buffer, 1000);
if (ret == -1) { printf("Error after return : %d\n",errno); }
printf("You typed on the console : %s\n",buffer);

}

void handleread_file(void * arg) {
char *buffer;
buffer = (char *)malloc(sizeof(char)*1000);
int fd;
fd = open("newfile.txt",O_RDONLY);

if (fd == -1)
{
        printf("FIle open error :%d\n",errno);
}
ssize_t ret = read_wrap(fd, buffer, 1000);
if (ret == -1) { printf("Error after return : %d\n",errno); }
printf("The file contents : %s\n", buffer);
}


int main(void) {
  scheduler_begin();
  char buffer[100];
  int n1 = 20000, n2 = 10000, n3 = 30000, num1 = 10, num2 = 122222;
  thread_fork(handleread_file,"File");
  thread_fork(print_nth_prime, &n1);
  thread_fork(print_nth_prime, &n2);
  //thread_fork(print_nth_prime, &n3);
  thread_fork(count,&num1);
  //thread_fork(count,&num2);
  thread_fork(handleread,"Console");
  //thread_fork(handleread_file,"File");
  
  scheduler_end();
printf("ending\n");
}
