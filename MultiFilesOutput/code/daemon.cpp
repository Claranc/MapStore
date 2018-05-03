#include <unistd.h>  
#include <signal.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <sys/stat.h>  
#include <time.h>  
#include <stdio.h>  
#include <fstream>
#include <iostream>
using namespace std;
  
static bool flag = true;  
void create_daemon();  
void handler(int);  
  
int main()  
{  
    time_t t;  
    int fd;  
    create_daemon();  
    struct sigaction act;  
    act.sa_handler = handler;  
    sigemptyset(&act.sa_mask);  
    act.sa_flags = 0;  
    if(sigaction(SIGQUIT, &act, NULL))  
    {  
        printf("sigaction error.\n");  
        exit(0);  
    }  
    while(1)  
    {  
        ofstream fout("time.log", ios::app);  
        system("./a.out -s");
    }  
    return 0;  
}  
void handler(int sig)  
{  
    printf("I got a signal %d\nI'm quitting.\n", sig);  
    flag = false;  
}  
void create_daemon()  
{  
    pid_t pid;  
    pid = fork();  
      
    if(pid == -1)  
    {  
        printf("fork error\n");  
        exit(1);  
    }  
    else if(pid)  
    {  
        exit(0);  
    }  
  
    if(-1 == setsid())  
    {  
        printf("setsid error\n");  
        exit(1);  
    }  
  
    pid = fork();  
    if(pid == -1)  
    {  
        printf("fork error\n");  
        exit(1);  
    }  
    else if(pid)  
    {  
        exit(0);  
    }  
  
    chdir("/home/xinjie/MapStore/MultiFilesOutput/code");  
    int i;  
    for(i = 0; i < 3; ++i)  
    {  
        close(i);  
    }  
    umask(0);  
    return;  
}  
