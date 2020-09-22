#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <netdb.h>
#include "buffer.h"
#include "fastcgi.h"
#include "httpargs.h"
using namespace std;

#define MAXLINE 8192
int open_clientfd(char* hostname,char* port);
class Connection{
public:
	Connection(int n);
	~Connection();
	void parse();
	void parseheader(char* buf);
	void parseargs(char* buf);
	void handle(uint32_t events);
	void check_and_send_file();
	void preparefcgidata(int fd_fcgi);
	void parsefcgiresponse();
	int parsephpheader();
	void clienterror(int fd,const char* cause,const char* errnum,const char* shortmsg,const char* longmsg);
	bool getvalid(){return isvalid;}
	void setvalid(bool x){isvalid=x;}
	int getfd(){return fd;}
	//int fill_fcgi_params(char* buf,int fd_to_fcgi,const char* name,const char* value);
private:
	int fd;
	bool isvalid;
	buffer tohttp;
	buffer fromhttp;
	buffer tophp;
	buffer fromphp;
	httpargs args;
	int phpargstate;
};
