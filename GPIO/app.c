#include <stdio.h>

int main(int argc, char *argv[])
{
  int fd;
  
  printf("argc = %d \n", argc);
  if(argc == 2)
  {
    if(strcmp(argv[1], "-g") == 0)
    {
      
    }
  }
  else
  {
    printf("input app 1 or app 0");
  }
  
  if(ioctl(fd, QUERY_CLR_VARIABLES) == -1)
  {
          printf("query_apps ioctl clr\n");
  }

  return 0;
}
