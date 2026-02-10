//user/xargs.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"  // 包含MAXARG（参数最大数量，xv6中定义为32）

#define NULL ((void*)0)//c语言宏定义末尾不用加分号

int read_line(char *buf, int buf_size)
{
    int i = 0;
    char c ;

    while (i<buf_size-1)
    {
        int n = read(0,&c,1);
        if (n < 0)
        {
            fprintf(2,"xargs: read error\n");
            return -1;
        }
        if (n ==0)
        {
            if(i == 0)
            {
                return 0;
            }
            break;//已经读取部分字符，终止读取
        }
        if(c == '\n')
        {
            break;
        }
        buf[i] = c;
        i++;
    }
    buf[i] = '\0'; // Ensure null termination
    return i;//return actual char numbers
    

}

int main(int argc,char *argv[])
{
    char line_buf[1024];//存储读取的输入行
    char *exec_argv[MAXARG];//传给exec的参数数组
    int i,n;

    if (argc < 2)
    {
        fprintf(2,"usage: xargs command [args...]\n");
        exit(1);
    }

    for(i=1;i<argc;i++)
    {
        
        exec_argv[i-1] = argv[i];
    }
    int base_argc = argc-1; //基础命令参数个数

    while ((n=read_line(line_buf,sizeof(line_buf)))>0)
    {
        if(base_argc +1 >= MAXARG)
        {
            fprintf(2,"xargs: too many arguments\n");
            exit(1);
        }
        exec_argv[base_argc] = line_buf;
        exec_argv[base_argc+1] =NULL ;
        int pid = fork();
        if (pid < 0)
        {
            fprintf(2,"xargs: fork error\n");
            exit(1);
        }
        else if (pid == 0)
        {
            exec(exec_argv[0],exec_argv);
            fprintf(2,"xargs: exec error\n");
            exit(1);
        }
        else
        {
            int status;
            wait(&status);
        }

    }

    if(n == -1)
    {
        fprintf(2,"xargs: read error\n");
        exit(1);
    }

    exit(0);
}