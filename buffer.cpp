#include "buffer.h"

buffer::buffer():pin(0),pout(0){}
int buffer::getin(int fd){
	char tmpbuf[65536];
	int readsum=0;
	int n;
	while((n=read(fd,tmpbuf,65536))>0){
		copysome(tmpbuf,n);
		readsum+=n;
	}
	if(n<0){
		if(errno!=EAGAIN)//error
		return n;
	}
	return readsum;
}

void buffer::copystr(const char* s){
	copysome(s,strlen(s));
}

void buffer::copysome(const char* s,int n){
	if(vb.capacity()<n+pin){
		vb.resize(pin+n);
		vb.resize(vb.capacity());
	}
	memcpy(getinptr(),s,n);
	pin+=n;
}

void buffer::readstr(char* buf){
	memcpy(buf,getoutptr(),pin-pout);
	pout=pin;
	reinit();
}
int buffer::readline(char* buf){
	char* p=buf;
	for(int i=pout;;i++){
		if(i+1>=pin)
			return 0;
		if(vb[i]=='\r' && vb[i+1]=='\n'){
			*p='\r';
			*(p+1)='\n';
			*(p+2)='\0';
			addout(i+2-pout);
			return 1;
		}
		*p=vb[i];
		p++;
	}
}
void buffer::readsome(char* buf,int length){
	memcpy(buf,getoutptr(),length);
	*(buf+length)='\0';
	addout(length);
}
int buffer::getout(int fd){
	int writesum=0;
	int n;
	while((n=write(fd,getoutptr(),pin-pout))>0){
		addout(n);
		writesum+=n;
	}
	if(n<0){
		if(errno!=EAGAIN)//error
		return n;
	}
	if(pin==pout)
	reinit();
	return writesum;
}
