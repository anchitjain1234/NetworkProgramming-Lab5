#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>

struct my_msgbuf
{
  long mtype;
  char mtext[3];
};

typedef struct my_msgbuf msgbuf;

key_t *keys,parent;
int *queues,pqueue,n;
pid_t *pids;

void inthandler(int signo)
{
	deletequeues(pqueue,queues,n);
	killchilds(pids,n);
	exit(0);
}

static void alarm_handler(int signo,siginfo_t *siginfo, void *context)
{
	alarm(5);
	// msgbug buf;
	// buf.mtype=1;
	// msgsnd(pqueue,&(buf.mtype),)
	// msgsnd()
	printf("WAKING up after 5 seconds PID: %ld\n",(long)getpid());
}

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
	signal(SIGINT,inthandler);
	msgbuf buf;

	if(argc!=2)
	{
		fprintf(stderr, "Invlaid arguments.Usage: ./mq n\n" );
		exit(-1);
	}

	n=atoi(argv[1]);
	keys=(key_t *)malloc(n*sizeof(key_t));
	queues=(int *)malloc(n*sizeof(int));

	parent=ftok("mq.c",0);
	pqueue=msgget(parent,IPC_CREAT | 0666);
	int i,j;

	for(i=0;i<n;i++)
	{
		keys[i]=ftok("mq.c",i+1);
		queues[i]=msgget(keys[i],IPC_CREAT | 0666);
	}

	pids=(pid_t *)malloc(n*sizeof(pid_t));
	int ppid=getpid();
	for(i=0;i<n;i++)
	{
		pids[i]=fork();
		if(pids[i]==0)
		{
			struct sigaction psa;
			memset (&psa, 0, sizeof (psa));
			psa.sa_sigaction=&alarm_handler;
			psa.sa_flags = SA_SIGINFO;

			sigaction(SIGALRM, &psa, NULL);
			buf.mtype=2;
			// signal(SIGALRM,alarm_handler);
			sleep(i);
			alarm(5);

			for (;;)
    		{
    			// printf("queue %d\n",queues[i]);
    			// printf("buf.mtype = %ld\n",buf.mtype);
      			if (msgrcv (queues[i], &(buf.mtype), sizeof (buf.mtext), 0, 0) == -1)
				{
					if (errno == EINTR) continue;
					// printf("text=%s\n",buf.mtext );
	  				perror ("error in receiving message.Exiting\n");
	  				exit (1);
				}
      			printf("PID: %d\n",getpid() );
    		}

			// while(msgrcv (queues[i], &(buf.mtype), sizeof (buf.mtext), 0, 0) != -1)
			// {
			// 	printf("PID: %d\n",getpid() );
			// }
		}
		else
		{
			continue;
		}
	}


	while(1==1)
	{

	}
	return 0;
}