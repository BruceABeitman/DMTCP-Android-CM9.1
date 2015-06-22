#include <stdio.h>
#include <unistd.h>

void f(double d){
  printf(" %f ",d);
}

int main(int argc, char* argv[])
{
  double count = 1.0;

  while (1)
  {
	  printf(" %f ",count+=1.0);
	  f(count);
	  fflush(stdout);
	  sleep(2);
  }
  return 0;
}
