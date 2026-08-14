#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
sieve(int input)
{
  int prime;
  int n;
  int p[2];

  if(read(input, &prime, sizeof(prime)) != sizeof(prime)){
    close(input);
    exit(0);
  }

  printf("prime %d\n", prime);

  if(pipe(p) < 0){
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    close(p[1]);
    close(input);
    sieve(p[0]);
    exit(0);
  }

  close(p[0]);
  while(read(input, &n, sizeof(n)) == sizeof(n)){
    if(n % prime != 0)
      write(p[1], &n, sizeof(n));
  }

  close(input);
  close(p[1]);
  wait(0);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];

  if(pipe(p) < 0){
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    close(p[1]);
    sieve(p[0]);
    exit(0);
  }

  close(p[0]);
  for(int i = 2; i <= 35; i++)
    write(p[1], &i, sizeof(i));

  close(p[1]);
  wait(0);
  exit(0);
}
