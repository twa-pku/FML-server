#include <vector>
#include <map>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include "Connection.h"
using namespace std;


template<typename TYPE,void(TYPE::*loop)()>
void* runEventloop(void* argp){
	if(pthread_detach(pthread_self())!=0){
                fprintf(stderr,"Thread detach error!\n");
                exit(1);
        }
        TYPE* p=reinterpret_cast<TYPE*>(argp);
        char tmpadd[50];
        sprintf(tmpadd,"creae working thread %d\n",p->getid());
        logs::getinstance()->addbuf(tmpadd);
        p->loop();
        return NULL;
}

class Eventloop{
public:
	Eventloop(int _id);
	~Eventloop();
	void loop();
	void wheel();
	void addfd(int fd);
	int getid(){return id;}
	void initthread();
private:
	vector<int> addingfd;
	struct epoll_event activefd[1024];
	map<int,Connection*> connmap;
	int epollfd;
	int fdnum;
	int id;
	uint32_t epollevents;
	uint64_t passfd;
	uint64_t receivefd;
	int event_fd;
	int timer_fd;
	struct itimerspec time_intv;
	queue<int> q;
	pthread_mutex_t mu;
	//for timewheel
	vector<set<int>> timewheel;
	map<int,int> twmap;
	int pnow;
};
