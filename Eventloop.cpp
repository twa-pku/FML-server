#include "Eventloop.h"

Eventloop::Eventloop(int _id):id(_id),fdnum(2),epollevents(EPOLLIN|EPOLLRDHUP|EPOLLOUT|EPOLLET),pnow(0){
	passfd=0;
	mu=PTHREAD_MUTEX_INITIALIZER;
	for(int i=0;i<15;i++)
		timewheel.push_back(set<int>());
	event_fd=eventfd(0,EFD_NONBLOCK);
	timer_fd=timerfd_create(CLOCK_MONOTONIC,0);
	time_intv.it_value.tv_sec=1;
	time_intv.it_value.tv_nsec=0;
	time_intv.it_interval.tv_sec=1;
	time_intv.it_interval.tv_nsec=0;
	timerfd_settime(timer_fd,0,&time_intv,NULL);
	epollfd=epoll_create(1024);
	if(epollfd==-1){
		logs::getinstance()->addbuf("Epoll_create error!\n");
		exit(1);
	}
	struct epoll_event ev;
	ev.data.fd=event_fd;
	ev.events=EPOLLIN;
	Epoll_ctl(epollfd,EPOLL_CTL_ADD,event_fd,&ev);
	ev.data.fd=timer_fd;
	Epoll_ctl(epollfd,EPOLL_CTL_ADD,timer_fd,&ev);
	pthread_t tid;
	if(pthread_create(&tid,NULL,runEventloop<Eventloop,&Eventloop::loop>,reinterpret_cast<void*>(this))!=0){
        logs::getinstance()->addbuf("Create Eventloop thread error!\n");
        exit(1);
    }
}

Eventloop::~Eventloop(){
	Close(timer_fd);
	Close(event_fd);
	Close(epollfd);
	for(auto p=connmap.begin();p!=connmap.end();p++){
		delete p->second;
	}
}

void Eventloop::initthread(){
pthread_t tid;
if(pthread_create(&tid,NULL,runEventloop<Eventloop,&Eventloop::loop>,reinterpret_cast<void*>(this))!=0){
                        logs::getinstance()->addbuf("Create Eventloop thread error!\n");
                        exit(1);
                }
}
void Eventloop::loop(){
	while(1){
		//epoll，将有变化的连接放到activefd里面
		int readynum=epoll_wait(epollfd,activefd,fdnum,-1);
		if(readynum==-1){
			logs::getinstance()->addbuf("Epoll_wait error!\n");
			exit(errno);
		}
		//处理activefd里面的连接
		printf("%d\n",readynum);
		for(int i=0;i<readynum;i++){
			if(activefd[i].data.fd==event_fd){
				int newfd;
				int ret=read(event_fd,&passfd,sizeof(uint64_t));
				newfd=passfd;
				if(ret==-1){
					logs::getinstance()->addbuf("Read passfd error!\n");
				}
				struct epoll_event ev;
				ev.data.fd=newfd;
				ev.events=epollevents;
				Epoll_ctl(epollfd,EPOLL_CTL_ADD,newfd,&ev);
				fdnum++;
				connmap[newfd]=new Connection(newfd);
				timewheel[pnow].insert(newfd);
				twmap[newfd]++;
			}
			else if(activefd[i].data.fd==timer_fd){
				read(timer_fd,&passfd,sizeof(uint64_t));
				wheel();
			}
			else{
				timewheel[pnow].insert(activefd[i].data.fd);
				twmap[activefd[i].data.fd]++;
				connmap[activefd[i].data.fd]->handle(activefd[i].events);
			}
		}
		//处理需要断开的fd
		struct epoll_event tmp;
		for(auto p=connmap.begin();p!=connmap.end();){
			if(p->second->getvalid()==false){
				Epoll_ctl(epollfd,EPOLL_CTL_DEL,p->second->getfd(),&tmp);
				delete(p->second);
				p=connmap.erase(p);
			}
			else
				p++;
		}
		//唤醒eventfd
		pthread_mutex_lock(&mu);
		if(!q.empty()){
			passfd=q.front();
			q.pop();
			pthread_mutex_unlock(&mu);
			if(write(event_fd,&passfd,sizeof(uint64_t))==-1){
				logs::getinstance()->addbuf("Write passfd error!\n");
				return;
			}
		}
		else
			pthread_mutex_unlock(&mu);
	}
}

void Eventloop::wheel(){
	pnow=(pnow+1)%15;
	for(auto p=timewheel[pnow].begin();p!=timewheel[pnow].end();p++){
		printf("%d,%d\n",pnow,timewheel[pnow].size());
		twmap[*p]--;
		if(twmap[*p]==0){
			connmap[*p]->setvalid(false);
		}
	}
	timewheel[pnow].clear();
}

void Eventloop::addfd(int fd){
	pthread_mutex_lock(&mu);
	q.push(fd);
	pthread_mutex_unlock(&mu);
}
