#include <vector>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include "logs.h"
using namespace std;

class buffer
{
public:
	buffer();
	~buffer(){}
	int getin(int fd);
	int getout(int fd);
	void addin(int n){pin+=n;}
	void addout(int n){pout+=n;}
	void reinit(){pin=0;pout=0;}
	bool empty(){return pin==pout;}
	int readline(char* buf);
	void copystr(const char* s);
	void copysome(const char* s,int n);
	void readstr(char* buf);
	void readsome(char* buf,int length);
	int nowlength(){return pin-pout;}
	char* getoutptr(){return (&(*vb.begin()))+pout;}
	char* getinptr(){return (&(*vb.begin()))+pin;}
private:
	int pin;
	int pout;
	vector<char> vb;	
};
