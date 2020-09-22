#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "wrapper.h"
using namespace std;
#define MAXLINE 8192

template<typename TYPE,void(TYPE::*writelog)()>
void* runthread(void* argp){
	if(pthread_detach(pthread_self())!=0){
		fprintf(stderr,"Thread detach error!\n");
		exit(1);
	}
	TYPE* p=reinterpret_cast<TYPE*>(argp);
	printf("Log thread created\n");
	p->writelog();
	return NULL;
}
class logs{
public:
	~logs();
	static logs* getinstance(){return log;}
	void addbuf(const char* s);
	void writelog();
	void copystr(const char* s);
	void addtime();
private:
	static logs* log;
	char buf1[MAXLINE];
	char buf2[MAXLINE];
	char filename[30];
	int now;
	int filefd;
	int month;
	pthread_mutex_t mu;
	pthread_mutex_t mu2;
	pthread_cond_t cond;
	logs();
	logs(const logs& x){}
	logs& operator=(const logs& x){return *this;}
};
