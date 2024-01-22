/*******************************************************************************
 *  The BYTE UNIX Benchmarks - Release 3
 *          Module: syscall.c   SID: 3.3 5/15/91 19:30:21
 *
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Ben Smith, Rick Grehan or Tom Yager at BYTE Magazine
 *	ben@bytepb.byte.com   rick_g@bytepb.byte.com   tyager@bytepb.byte.com
 *
 *******************************************************************************
 *  Modification Log:
 *  $Header: syscall.c,v 3.4 87/06/22 14:32:54 kjmcdonell Beta $
 *  August 29, 1990 - Modified timing routines
 *  October 22, 1997 - code cleanup to remove ANSI C compiler warnings
 *                     Andy Kahn <kahn@zk3.dec.com>
 *
 ******************************************************************************/
/*
 *  syscall  -- 循环调用系统调用
 *
 */
char SCCSid[] = "@(#) @(#)syscall.c:3.3 -- 5/15/91 19:30:21";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "timeit.c"

unsigned long iter;  // 全局变量，用于记录系统调用的迭代次数

// 声明 report 函数，用于输出测试结果并退出程序
void report()
{
	fprintf(stderr,"COUNT|%ld|1|lps\n", iter);
	exit(0);
}

// 创建一个文件描述符用于管道通信
int create_fd()
{
	int fd[2];

	// 创建管道，如果创建失败则退出
	if (pipe(fd) != 0 || close(fd[1]) != 0)
	    exit(1);

	return fd[0];
}

// 主函数，接收命令行参数
int main(argc, argv)
int	argc;
char	*argv[];
{
        char   *test;
	int	duration;
	int	fd;

	// 检查命令行参数，如果不足两个参数则输出提示信息并退出
	if (argc < 2) {
		fprintf(stderr,"Usage: %s duration [ test ]\n", argv[0]);
                fprintf(stderr,"test is one of:\n");
                fprintf(stderr,"  \"mix\" (default), \"close\", \"getpid\", \"exec\"\n");
		exit(1);
	}
        if (argc > 2)
            test = argv[2];
        else
            test = "mix";

	// 将命令行参数转换为整数，作为测试运行的时长
	duration = atoi(argv[1]);

	// 初始化迭代次数为 0，通过 wake_me 函数设置报告函数和运行时间
	iter = 0;
	wake_me(duration, report);

        // 根据测试类型选择相应的测试操作
        switch (test[0]) {
        case 'm':
	   // 创建文件描述符，并在循环中调用不同的系统调用
	   fd = create_fd();
	   while (1) {
		close(dup(fd));
		syscall(SYS_getpid);
		getuid();
		umask(022);
		iter++;
	   }
	   /* NOTREACHED */
        case 'c':
           // 创建文件描述符，并在循环中执行 close 操作
           fd = create_fd();
           while (1) {
                close(dup(fd));
                iter++;
           }
           /* NOTREACHED */
        case 'g':
           // 在循环中调用 getpid 系统调用
           while (1) {
                syscall(SYS_getpid);
                iter++;
           }
           /* NOTREACHED */
        case 'e':
           // 在循环中创建子进程并执行 /bin/true 命令
           while (1) {
                pid_t pid = fork();
                if (pid < 0) {
                    fprintf(stderr,"%s: fork failed\n", argv[0]);
                    exit(1);
                } else if (pid == 0) {
                    execl("/bin/true", "/bin/true", (char *) 0);
                    fprintf(stderr,"%s: exec /bin/true failed\n", argv[0]);
                    exit(1);
                } else {
                    if (waitpid(pid, NULL, 0) < 0) {
                        fprintf(stderr,"%s: waitpid failed\n", argv[0]);
                        exit(1);
                    }
                }
                iter++;
           }
           /* NOTREACHED */
        }

        // 退出程序
        exit(9);
}
