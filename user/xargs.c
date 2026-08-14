#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 512

static void
run(char *cmd, char *base_argv[], int base_argc, char *line)
{
  char *argv[MAXARG];
  char *p;
  int argc;
  int pid;

  for(argc = 0; argc < base_argc; argc++)
    argv[argc] = base_argv[argc];

  p = line;
  while(*p != 0){
    while(*p == ' ' || *p == '\t')
      p++;
    if(*p == 0)
      break;

    if(argc >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      return;
    }

    argv[argc++] = p;
    while(*p != 0 && *p != ' ' && *p != '\t')
      p++;
    if(*p != 0)
      *p++ = 0;
  }

  if(argc == base_argc)
    return;

  argv[argc] = 0;

  pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    exec(cmd, argv);
    fprintf(2, "xargs: exec %s failed\n", cmd);
    exit(1);
  }

  wait(0);
}

int
main(int argc, char *argv[])
{
  char line[MAXLINE];
  char *base_argv[MAXARG];
  char c;
  int n;
  int pos = 0;

  if(argc < 2){
    fprintf(2, "usage: xargs command [args...]\n");
    exit(1);
  }

  if(argc >= MAXARG){
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++)
    base_argv[i - 1] = argv[i];

  while((n = read(0, &c, 1)) > 0){
    if(c == '\r')
      continue;
    if(c == '\n'){
      line[pos] = 0;
      run(argv[1], base_argv, argc - 1, line);
      pos = 0;
      continue;
    }

    if(pos + 1 >= sizeof(line)){
      fprintf(2, "xargs: input line too long\n");
      exit(1);
    }
    line[pos++] = c;
  }

  if(pos > 0){
    line[pos] = 0;
    run(argv[1], base_argv, argc - 1, line);
  }

  exit(0);
}
