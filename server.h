#include <vector>
#include <set>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Eventloop.h"

class server{
public:
	server(int portnum,int threadnum_);
	~server();
	void loop();
	void initthread();
private:
	int port;
	int threadnum;
	int listenfd;
	vector<Eventloop*> Eventpool;
	int nextthread;
};
