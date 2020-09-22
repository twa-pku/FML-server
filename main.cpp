#include <iostream>
#include <signal.h>
#include "server.h"
using namespace std;

#define MAXLINE 8192

logs* logs::log=new logs();
int main(int argc,char** argv){
	signal(SIGPIPE,SIG_IGN);
	if (argc != 3) {
	fprintf(stderr, "argc (%d) error\n", argc);
	exit(1);
    }

	server httpserver(atoi(argv[1]),atoi(argv[2]));
	httpserver.initthread();
	httpserver.loop();
	return 0;
}
