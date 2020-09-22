#include "fastcgi.h"

void fill_fcgi_header(char* buf,int fd_to_fcgi,int type,int length){
	*buf++=1;
	*buf++=type;
	*buf++=(fd_to_fcgi>>8);
	*buf++=(fd_to_fcgi&0xff);
	*buf++=(length>>8);
	*buf++=(length&0xff);
	*buf++=(((length&7)^7)+1)&7;
	*buf=0;
	return;
}
//
void fill_fcgi_beginrequestbody(char* buf,int flag){
	*buf++=0;
	*buf++=1;
	*buf++=flag;
	return;
}

int fill_fcgi_params(char* buf,int fd_to_fcgi,const char* name,const char* value){
	int nameLength=strlen(name);
	int valueLength=strlen(value);
	char* p=buf;
	int paramLength;
	if(nameLength<127 && valueLength<127){
		fill_fcgi_header(buf,fd_to_fcgi,4,2+nameLength+valueLength);
		p=p+8;
		*p++=(char)(nameLength&0xff);
		*p++=(char)(valueLength&0xff);
		paramLength=10+nameLength+valueLength;
	}
	else{
	fill_fcgi_header(buf,fd_to_fcgi,4,8+nameLength+valueLength);
	p=p+8;
	*p++=(char)((nameLength>>24)&0x7f)|0x80;
	*p++=(char)(nameLength>>16)&0xff;
	*p++=(char)(nameLength>>8)&0xff;
	*p++=(char)nameLength&0xff;
	*p++=(char)((valueLength>>24)&0x7f)|0x80;
	*p++=(char)(valueLength>>16)&0xff;
	*p++=(char)(valueLength>>8)&0xff;
	*p++=(char)valueLength&0xff;
	paramLength=8+nameLength+valueLength;
	}
	int i;
	for(i=0;i<nameLength;i++)
		*p++=name[i];
	for(i=0;i<valueLength;i++)
		*p++=value[i];
	int n=paramLength%8;
	if(n>0)
		return paramLength+8-n;
	else
		return paramLength;
}
