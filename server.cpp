#include "server.h"

server::server(int portnum,int threadnum_):port(portnum),threadnum(threadnum_),nextthread(0){
	//socket
	int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(fd==-1){
		fprintf(stderr,"Create socket error!\n");
		exit(errno);
	}
	int tmp=1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&tmp),sizeof(int))==-1){
		fprintf(stderr,"Setsockopt error!\n");
		exit(errno);
	}
	//bind
	struct sockaddr_in sin;
	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_port=htons(portnum);
	sin.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(fd,reinterpret_cast<struct sockaddr*>(&sin),sizeof(sin))==-1){
		fprintf(stderr,"Bind error!\n");
		exit(errno);
	}
	//listen
	if(listen(fd,1024)==-1){
		fprintf(stderr,"Listen error!\n");
		exit(errno);
	}
	listenfd=fd;
}

server::~server(){
	for(int i=0;i<threadnum;i++){
		delete Eventpool[i];
	}
}

void server::initthread(){
	Eventpool.resize(threadnum);
	for(int i=0;i<threadnum;i++){
		Eventpool[i]=new Eventloop(i);
	}
}

void server::loop(){
	while(1){
		int newfd=accept(listenfd,(struct sockaddr*)NULL,NULL);
		if(newfd==-1){
			fprintf(stderr,"Server accept error!\n");
			exit(1);
		}
		int tmp=fcntl(newfd, F_GETFL, 0);
		if(tmp==-1){
			fprintf(stderr,"Fcntl error!\n");
			return;
		}
		tmp|=O_NONBLOCK;
		if(fcntl(newfd,F_SETFL,tmp)==-1){
			fprintf(stderr,"Fcntl error!\n");
			return;
		}
		tmp=fcntl(newfd, F_GETFL, 0);
		if(tmp==-1){
			fprintf(stderr,"Fcntl error!\n");
			return;
		}
		tmp|=FD_CLOEXEC;
		if(fcntl(newfd,F_SETFL,tmp)==-1){
			fprintf(stderr,"Fcntl error!\n");
			return;
		}
		Eventpool[nextthread]->addfd(newfd);
		nextthread=(nextthread+1)%threadnum;
	}
}
