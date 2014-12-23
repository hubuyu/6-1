#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define DEV_NAME	"/dev/hubuyu"

int main()
{	
	int fd ;
	int err ;
	char data[16];
	char *send_buf="RaspberryPI";
	fd = open(DEV_NAME,O_RDWR);
	if(fd<0)
	{
		printf("Open %s failed \n" ,DEV_NAME );	
		return -1 ;
	}
	err = write(fd , send_buf , strlen(send_buf));
	if(err<0){
		printf("Write data to kernel failed\n");
		return -1 ;
	}
	printf("Write-%s to kernel success\n",send_buf );
	err = read(fd , data ,sizeof(data));
	printf("read length is %d \n", sizeof(data));
	if(err<0){
		printf("Read data from kernel failed\n");
		return -1 ;
	}
	printf("Read data-%s \n",data);
	close(fd);
}
