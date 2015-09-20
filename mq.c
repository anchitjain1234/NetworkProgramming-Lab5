#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>

struct my_msgbuf
{
  long mtype;
  char mtext[7];
};

typedef struct my_msgbuf msgbuf;

key_t *keys,parent;
int *queues,pqueue,n;
pid_t *pids;

int randint_generator()
{
	srand(time(NULL)+getpid());
	return rand()%10000000;
}

void inthandler(int signo)
{
	deletequeues(pqueue,queues,n);
	killchilds(pids,n);
	exit(0);
}

void alarm_handler(int signo)
{
	alarm(5);
	msgbuf buf;
	buf.mtype=1;
	int r = randint_generator();
	sprintf(buf.mtext,"%d",r);
	if(msgsnd(pqueue,&(buf.mtype),sizeof(buf.mtext),0)==-1)
	{
		fprintf(stderr, "Error in sending message to PARENT FROM PID %d\n",getpid() );
	}
	else
		printf("MESSAGE SENT   - PID = %d   MESSAGE = %s\n",getpid(),buf.mtext);
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
	//Modified SIGINT handler so that all the message queues created could be deleted as well.
	signal(SIGINT,inthandler);
	msgbuf buf;

	if(argc!=2)
	{
		fprintf(stderr, "Invlaid arguments.Usage: ./mq n\n" );
		exit(-1);
	}

	n=atoi(argv[1]);

	if(n<=0)
	{
		perror("Please enter n greater than 0");
		exit(0);
	}

	//keys for queue of child processes.
	keys=(key_t *)malloc(n*sizeof(key_t));
	//id of message queues for child processes.
	queues=(int *)malloc(n*sizeof(int));

	//key for queue for parent
	parent=ftok("mq.c",0);
	//id of parent message queue.
	pqueue=msgget(parent,IPC_CREAT | 0666);
	int i,j;

	//creating n queues.
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
			buf.mtype=2;
			signal(SIGALRM,alarm_handler);
			//Variable sleep so that first 4 processes send message one at a time ,so that results can be seen clearly.
			// if(i<4)
			// 	sleep(i);
			// else
			// 	sleep(4);
			alarm(5);

			for (;;)
    		{
      			if (msgrcv (queues[i], &(buf.mtype), sizeof (buf.mtext), 0, 0) == -1)
				{
					//Since in this blocking process we might also receive SIGALRM so EINTR needs to be ignored.
					if (errno == EINTR) continue;
	  				fprintf(stderr, "error in receiving message for pid %d\n",getpid() );
				}
      			printf("MESSAGE RECEIVED   - PID = %d   MESSAGE = %s\n",getpid(),buf.mtext);
    		}
		}
		else
		{
			continue;
		}
	}

	//Sending messages from parent to child processes.
	for (;;)
	{
		buf.mtype=1;
		if (msgrcv (pqueue, &(buf.mtype), sizeof (buf.mtext), 0, 0) == -1)
		{
			if (errno == EINTR) continue;
				perror ("error in receiving message to PARENT\n");
		}
		for(i=0;i<n;i++)
		{
			buf.mtype=2;
			if(msgsnd(queues[i],&(buf.mtype),sizeof(buf.mtext),0)==-1)
				fprintf(stderr, "Error in sending message to QUEUE %d from PARENT\n",queues[i] );
		}
	}
	return 0;
}