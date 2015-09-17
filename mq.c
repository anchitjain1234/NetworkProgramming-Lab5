#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

struct my_msgbuf
{
  long mtype;
  int mtext[3];
};

typedef struct my_msgbuf msgbuf;

int deletequeues(int pq,int *queues,int n)
{
	msgctl(pq,IPC_RMID, NULL);
	int i;

	for(i=0;i<n;i++)
		msgctl(queues[i],IPC_RMID, NULL);
}

int killchilds(int* pids,int n)
{
	int i;

	for(i=0;i<n;i++)
		kill(pids[i],SIGKILL);
}

int main(int argc, char const *argv[])
{
	msgbuf buf;

	if(argc!=2)
	{
		fprintf(stderr, "Invlaid arguments.Usage: ./mq n\n" );
		exit(-1);
	}

	int n=atoi(argv[1]);
	key_t *keys=(key_t *)malloc(n*sizeof(key_t));
	int *queues=(int *)malloc(n*sizeof(int)),pqueue;

	key_t parent=ftok("mq.c",0);
	pqueue=msgget(parent,IPC_CREAT | 0666);
	int i,j;

	for(i=0;i<n;i++)
	{
		keys[i]=ftok("mq.c",i+1);
		queues[i]=msgget(keys[i],IPC_CREAT | 0666);
	}

	pid_t *pids=(pid_t *)malloc(n*sizeof(pid_t));
	int ppid=getpid();
	for(i=0;i<n;i++)
	{
		pids[i]=fork();
		if(pids[i]==0)
		{
			while(1==1)
			{
				sleep(1);
			}
		}
		else
		{
			continue;
		}
	}

	deletequeues(pqueue,queues,n);
	killchilds(pids,n);
	return 0;
}