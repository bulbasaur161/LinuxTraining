#include <stdio.h>

int main(int argc, char *argv[])
{
	int fd;
	char *file_name = "/dev/gpio_test";

	printf("argc = %d \n", argc);
	if(argc == 2)
	{
		if(strcmp(argv[1], "-g") == 0)
		{

		}
	}
	else
	{
		printf("input app 1 or app 0\n");
	}

	fd = open(file_name, O_RDWR);
	if(fd == -1)
	{
	    perror("query_apps open\n");
	    return 2;
	}
	
	if(ioctl(fd, 1, &q) == -1)
	{
		printf("query_apps ioctl clr\n");
	}

	close(fd);
	return 0;
}
