#include "logs.h"

logs::logs():now(0),month(5){
	buf2[0]='\0';
	buf1[0]='\0';
	char filename[10];
	sprintf(filename,"logs%d",month);
	filefd=open(filename,O_WRONLY|O_CREAT|O_APPEND,0);
	if(filefd==-1){
		fprintf(stderr, "Open logfile error!\n");
		exit(1);
	}
	mu=PTHREAD_MUTEX_INITIALIZER;
	mu2=PTHREAD_MUTEX_INITIALIZER;
	cond=PTHREAD_COND_INITIALIZER;
	pthread_t tid;
	if(pthread_create(&tid,NULL,runthread<logs,&logs::writelog>,reinterpret_cast<void*>(this))!=0){
		fprintf(stderr, "Create log error!\n");
		exit(1);
	}
}

logs::~logs(){
	Close(filefd);
	delete getinstance();
}

void logs::addbuf(const char* s){
	addtime();
	copystr(s);
	if(now+100>MAXLINE){
		pthread_cond_signal(&cond);
	}
}

void logs::copystr(const char* s){
	const char* p=s;
	pthread_mutex_lock(&mu);
	while(*p!='\0'){
		buf1[now++]=*p++;
	}
	buf1[now]='\0';
	pthread_mutex_unlock(&mu);
}

void logs::addtime(){
	struct timeval tv;
	char tmbuf[100],buf[100];
	struct tm *nowtm;
	gettimeofday(&tv,NULL);
	nowtm=localtime(&tv.tv_sec);
	strftime(tmbuf,sizeof(tmbuf),"%Y-%m-%d %H:%M:%S",nowtm);
	snprintf(buf,sizeof(buf),"%s.%06ld ",tmbuf,tv.tv_usec);
	copystr(buf);
}

void logs::writelog(){
	timespec ts;
	ts.tv_sec=5;
	ts.tv_nsec=0;
	while(1){
		pthread_mutex_lock(&mu2);
		if(buf2[0]=='\0')
			pthread_cond_timedwait(&cond,&mu2,&ts);
		pthread_mutex_lock(&mu);
		strcpy(buf2,buf1);
		now=0;
		buf1[0]='\0';
		pthread_mutex_unlock(&mu);
		write(filefd,buf2,strlen(buf2));
		buf2[0]='\0';
		pthread_mutex_unlock(&mu2);
	}
}
