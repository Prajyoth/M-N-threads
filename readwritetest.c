#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "scheduler.h"
#include <stdio.h>


struct aiocb aiocbp;

int main()
{
char *buf;
int count = 50;
buf = (char *)malloc(sizeof(char)*50);
FILE *fp = fopen("newfile.txt","r");
 memset(buf,'a',50);
 memset(&aiocbp, 0, sizeof(struct aiocb));
 aiocbp.aio_fildes = fp;
 aiocbp.aio_buf = buf;
 aiocbp.aio_nbytes = count;
 aiocbp.aio_sigevent.sigev_notify=SIGEV_NONE;
 if (aio_read(&aiocbp) == -1)
 {
        printf("Error at aio_read(): %d\n",errno);
        close(fp);
 }
 while (aio_error(&aiocbp) == EINPROGRESS)
 {
        //printf("Yielding\n");
        //yield();
 }
        aiocbp.aio_offset = lseek(fp,count, SEEK_CUR);
        printf("%d\n",aio_error(&aiocbp));
if (aio_return(&aiocbp) == -1)
{
	printf("OOps problem has occured");
	return 0;
}
printf("%s",buf);
