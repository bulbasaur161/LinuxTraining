#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
	int fd;
	char *file_name = "/dev/gpio_test";

	printf("argc = %d \n", argc);
	if(argc == 2)
	{
		if(strcmp(argv[1], "1") == 0)
		{
			fd = open(file_name, O_RDWR);
			if(fd == -1)
			{
			    perror("query_apps open\n");
			    return 2;
			}

			if(ioctl(fd, 1, 0) == -1)
			{
				printf("query_apps ioctl clr\n");
			}

			close(fd);
		}
		else if(strcmp(argv[1], "0") == 0)
		{
			fd = open(file_name, O_RDWR);
			if(fd == -1)
			{
			    perror("query_apps open\n");
			    return 2;
			}

			if(ioctl(fd, 0, 0) == -1)
			{
				printf("query_apps ioctl clr\n");
			}

			close(fd);
		}
	}
	else
	{
		printf("input app 1 or app 0\n");
	}

	return 0;
}
