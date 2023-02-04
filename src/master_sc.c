#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

int spawn(const char *program, char *arg_list[])
{ // function to spawn a process and open a console for it

  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    return 1;
  }

  else if (child_pid != 0)
  {
    return child_pid;
  }

  else
  {
    if (execvp(program, arg_list) == 0)
      ;
    perror("Exec failed");
    return 1;
  }
}

int main()
{

  int fd1;
  char px[80];
  char *myfifo = "/tmp/myfifo"; // Pipe address that comes from the command console
  if (mkfifo(myfifo, 0666) != 0)
  {
    perror("warning message:");
  }

  char buff[20];
  char IP[50];
  char port[50];
  int op;
  char string_ip[50];
  char string_port[50];
  printf("1. Normal mode: Assignment 2\n");
  printf("2. Client\n");
  printf("3. Server\n");
  printf("Choose the machine funcionality\n");
  fgets(buff, sizeof(buff), stdin);

  if (strncmp(buff, "1", 1) == 0)
  {
    // argument list to call the executable of each process
    char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", "out/testA.bmp", "5", NULL};
    char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", "5", NULL};
    // The spawn function is called to execute the other processes
    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
    sleep(1);
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

    int status;
    waitpid(pid_procA, &status, 0);
    waitpid(pid_procB, &status, 0);

    printf("Main program exiting with status %d\n", status);
    return 0;
  }
  else if (strncmp(buff, "2", 1) == 0)
  {

    printf("Input ip\n");
    fgets(IP, sizeof(IP), stdin);
    sprintf(string_ip, "%s", IP);
    printf("Input port\n");
    fgets(port, sizeof(port), stdin);
    sprintf(string_port, "%s", port);
    char *arg_list_Ac[] = {"/usr/bin/konsole", "-e", "./bin/clientA", string_ip, string_port, NULL};
    char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};
    pid_t pid_procAc = spawn("/usr/bin/konsole", arg_list_Ac);
    sleep(1);
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);
    int status;
    waitpid(pid_procAc, &status, 0);

    printf("Main program exiting with status %d\n", status);
    return 0;
  }

  else if (strncmp(buff, "3", 1) == 0)
  {
    printf("Input port\n");
    fgets(port, sizeof(port), stdin);
    sprintf(string_port, "%s", port);
    char *arg_list_As[] = {"/usr/bin/konsole", "-e", "./bin/serverA",string_port, NULL};
    char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};
    pid_t pid_procAs = spawn("/usr/bin/konsole", arg_list_As);
    sleep(1);

    fd1 = open(myfifo, O_RDONLY); // open command console pipe
    read(fd1, px, 80);            // Read the velocity value from the command console
    sscanf(px, "%d", &op);
    close(fd1); // close pipe

    if (op == 1)
    {
      pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

      int status;
      waitpid(pid_procAs, &status, 0);
      waitpid(pid_procB, &status, 0);
      return 0;
    }
  }
  // argument list to call the executable of each process
  /*char *arg_list_As[] = {"/usr/bin/konsole", "-e", "./bin/serverA", NULL};
  char *arg_list_Ac[] = {"/usr/bin/konsole", "-e", "./bin/clientA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};
  // The spawn function is called to execute the other processes
  pid_t pid_procAs = spawn("/usr/bin/konsole", arg_list_As);
  sleep(1);
  pid_t pid_procAc = spawn("/usr/bin/konsole", arg_list_Ac);
  sleep(1);
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  int status;
  waitpid(pid_procAs, &status, 0);
  waitpid(pid_procAc, &status, 0);
  waitpid(pid_procB, &status, 0);

  printf("Main program exiting with status %d\n", status);
  return 0;*/
}
