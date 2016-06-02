#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "scheduler.h"


//struct aiocb aiocbp;
ssize_t read_wrap(int fd, void * buf, size_t count)
{
 struct aiocb aiocbp;
 //char *buffer = (char *)buf;
// memset(buf,'a',sizeof(buf));
 memset(&aiocbp, 0, sizeof(struct aiocb));
 aiocbp.aio_fildes = fd;
 aiocbp.aio_buf = buf;
 aiocbp.aio_nbytes = count;
 aiocbp.aio_sigevent.sigev_notify=SIGEV_NONE;
 if (fd > 0) {
 aiocbp.aio_offset = lseek(fd, 0 , SEEK_SET);
 }
 if (aio_read(&aiocbp) == -1)
 {
	printf("Error at aio_read(): %d\n",errno);
	close(fd);
 }
 while (aio_error(&aiocbp) == EINPROGRESS)
 {
	//printf("Yielding\n");
	aiocbp.aio_offset = lseek(fd, 0 ,SEEK_CUR);
	yield();
 }
	if (aio_error(&aiocbp) > 0)
	{
		printf("aio error : %s\n", strerror(aio_error(&aiocbp)));
	}
	aiocbp.aio_offset = lseek(fd,count, SEEK_SET);
	//printf("%d\n",aio_error(&aiocbp));
	return aio_return(&aiocbp);
 
}
