all:mq

mq:mq.c
	gcc -g mq.c -o mq

clean:
	rm mq