#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "led.h"

int main(int argc, char *argv[])
{
	int i = 1;
	int fd;
	
	if (0 > (fd = open("/dev/LED", O_RDWR))) 
	{
		printf("fd open failed\n");
		return -1;
	}

	ioctl(fd, LED_ON, 0);
	sleep(1);
	ioctl(fd, LED_OFF, 0);
	sleep(5);

	while (1) 
	{
		if (4 < i)
		{
			i = 1;
		}

		ioctl(fd, LED_ON, i);
		sleep(1);
		i++;
	}

	close(fd);

	return 0;
}
