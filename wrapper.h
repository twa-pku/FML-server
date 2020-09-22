#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
using namespace std;

static void Close(int fd){
	if(close(fd)==-1){
		fprintf(stderr, "Close logfile error!\n");
		exit(1);
	}
}

static void Epoll_ctl(int epollfd,int op,int fd,struct epoll_event *ev){
	fprintf(stdout,"enter epollfd with op %d epollfd %d fd %d\n",op,epollfd,fd);
	fflush(stdout);
	if(epoll_ctl(epollfd,op,fd,ev)==-1){
		fprintf(stderr,"Epoll_ctl error!\n");
		exit(1);
	}
}

