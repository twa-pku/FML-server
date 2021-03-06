#ifndef __FASTCGI_H__
#define __FASTCGI_H__

#include <string.h>
using namespace std;

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

//fcgi_beginrequestbody roles
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

//flags
#define FCGI_KEEP_CONN  1

//fcgi_endrequestbody protocolstatus
#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

//working folder
#define WORKING_FOLDER "/home/FML-server"

typedef struct{
	unsigned char version;
	unsigned char type;
	unsigned char requestIdB1;
	unsigned char requestIdB0;
	unsigned char contentLengthB1;
	unsigned char contentLengthB0;
	unsigned char paddingLength;
	unsigned char reserve;
}fcgi_header;

typedef struct{
	unsigned char roleB1;
	unsigned char roleB0;
	unsigned char flags;
	unsigned char reserved[5];
}fcgi_beginrequestbody;

typedef struct{
	unsigned char appStatusB3;
	unsigned char appStatusB2;
	unsigned char appStatusB1;
	unsigned char appStatusB0;
	unsigned char protocolStatus;
	unsigned char reserved[3];
}fcgi_endrequestbody;

void fill_fcgi_header(char* buf,int fd_to_fcgi,int type,int length);
void fill_fcgi_beginrequestbody(char* buf,int flag);
int fill_fcgi_params(char* buf,int fd_to_fcgi,const char* name,const char* value);
#endif
