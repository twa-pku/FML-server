#include "Connection.h"

Connection::Connection(int n):fd(n),isvalid(true),phpargstate(0){}
Connection::~Connection(){
	if(!tohttp.empty()){
		fprintf(stderr,"Something error in buffer\n");
	}
	if(shutdown(fd,2)==-1){
		fprintf(stderr,"Shutdown error!\n");
	}
}
void Connection::handle(uint32_t events){
	if(events&EPOLLERR){
		fprintf(stderr,"Epoll error!\n");
		isvalid=false;
		//exit(errno);
	}
	if(events&(EPOLLIN|EPOLLRDHUP)){
		if(fromhttp.getin(fd)==0){
			isvalid=false;
		}
		else{
			parse();
			if(args.getstate()==3){
				if(args.getisstatic()){
					check_and_send_file();
				}
				else{
					int fd_fcgi=socket(AF_INET,SOCK_STREAM,0);
					if(fd_fcgi==-1){
						fprintf(stderr,"Create php socket error!\n");
						exit(1);
					}
					struct sockaddr_in sin;
					bzero(&sin,sizeof(sin));
					sin.sin_family=AF_INET;
					sin.sin_port=htons(9000);
					if(inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr)!=1){
						fprintf(stderr,"Inet_pton error\n");
						exit(errno);
					}
					if(connect(fd_fcgi,reinterpret_cast<struct sockaddr*>(&sin),sizeof(sin))==-1){
						fprintf(stderr,"Bind to php-fpm error\n");
						exit(errno);
					}
					preparefcgidata(fd_fcgi);
					while(!tophp.empty())
						tophp.getout(fd_fcgi);
					fromphp.getin(fd_fcgi);
					int tmpcount=20;
					while(tmpcount>0 && phpargstate!=3){
						parsefcgiresponse();
						tmpcount--;
					}
					if(tmpcount==0)
						clienterror(fd,"","500","Internall server error","Parse error");
					else
						tohttp.getout(fd);
					phpargstate=0;
					Close(fd_fcgi);
				}
			}
			else if(args.getstate()!=4){
				clienterror(fd,"","500","Internall server error","Parse error");
			}
			args.setstate(0);
			args.clearargs();
		}
	}
	if(events&EPOLLOUT){
		if(!tohttp.empty()){
			logs::getinstance()->addbuf("Receive EPOLLOUT\n");
			tohttp.getout(fd);
		}
	}
	if(events&EPOLLHUP){
		char tmpbuf[60];
		sprintf(tmpbuf,"Receive EPOLLHUP in fd %d\n",fd);
		logs::getinstance()->addbuf(tmpbuf);
		isvalid=false;
	}
}
void Connection::parse(){
	//函数的目的是填写httpargs中的参数
	//状态：未解析第一行，解析了第一行，解析完了，信息收集完了
	while(args.getstate()<3 && !fromhttp.empty()){
		char buf[MAXLINE];
		if(fromhttp.readline(buf)==0)
			break;
		if(args.getstate()==0)
			parseheader(buf);
		else if(args.getstate()==1)
			parseargs(buf);
		if(args.getstate()==2){
			fromhttp.readstr(buf);
			strcpy(args.getparameter(),buf);
			args.setstate(3);
		}
	}
}
void Connection::parseheader(char* buf){
	//填写method和filename
	char method[10],uri[MAXLINE],version[10];
	sscanf(buf, "%s %s %s", method, uri, version);
	char addtmpbuf[300];
	sprintf(addtmpbuf,"Parsing header %s",buf);
	logs::getinstance()->addbuf(addtmpbuf);
	if(strcasecmp(method,"GET")!=0 && strcasecmp(method,"POST")!=0){
		clienterror(fd, method, "501", "Not Implemented","FML server does not implement this method");
		args.setstate(4);
		return;
	}
	args.setmethod(method);
	args.setparameter("");
	char tmppath[100]=WORKING_FOLDER;
	//printf(tmppath);
	if(strcasecmp(uri,"/")==0){
		args.setpath(strcat(tmppath,"/index.php"));
		args.setstate(1);
		args.setisstatic(false);
		return;
	}
	char* p=index(uri,'?');
	if(p){
		args.setparameter(p+1);
		*p='\0';
	}
	args.setpath(strcat(tmppath, uri));
	args.setstate(1);
	if(!strstr(uri,".php"))
		args.setisstatic(true);
	else
		args.setisstatic(false);
}

void Connection::parseargs(char* buf){
	if(*buf=='\r' && *(buf+1)=='\n'){
		if(strcasecmp(args.getmethod(),"POST")==0)
			args.setstate(2);
		else
			args.setstate(3);
		return;
	}
	char* p=index(buf,':');
	char* p2=index(buf,'\r');
	*p='\0';
	*p2='\0';
	p+=2;
	string bufs(buf);
	if(args.inset(bufs)){
		args.addset(bufs,string(p));
	}
}
void Connection::check_and_send_file(){
	char* filename=args.getpath();
	struct stat sbuf;
	if (stat(filename, &sbuf) < 0) {
	clienterror(fd, filename, "404", "Not found","FML server couldn't find this file");
	return;
    }      
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
	    clienterror(fd, filename, "403", "Forbidden","FML server couldn't read the file");
	    return;
	}
	char filetype[20];
	if(strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else
		strcpy(filetype, "text/plain");
	int srcfd;
	char* srcp;
	char tmpbuf[100];
	tohttp.copystr("HTTP/1.1 200 OK\r\nServer: FML Server\r\nConnection: keep-alive\r\nKeep-alive: timeout=15\r\n");
	int filesize=sbuf.st_size;
	sprintf(tmpbuf,"Content-length: %d\r\n",filesize);
	tohttp.copystr(tmpbuf);
	sprintf(tmpbuf, "Content-type: %s\r\n\r\n", filetype);
	tohttp.copystr(tmpbuf);
	srcfd = open(filename, O_RDONLY, 0);
    srcp = static_cast<char*>(mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0));
    Close(srcfd);
    tohttp.copystr(srcp);
    munmap(srcp, sbuf.st_size);
    tohttp.getout(fd);
}
void Connection::preparefcgidata(int fd_fcgi){
	char buf[65536];
	fill_fcgi_header(buf,fd_fcgi,1,8);
	tophp.copysome(buf,8);
	fill_fcgi_beginrequestbody(buf,0);
	tophp.copysome(buf,8);
	int paramlength;
	paramlength=fill_fcgi_params(buf,fd_fcgi,"REQUEST_METHOD",args.getmethod());
	tophp.copysome(buf,paramlength);
	paramlength=fill_fcgi_params(buf,fd_fcgi,"SCRIPT_FILENAME",args.getpath());
	tophp.copysome(buf,paramlength);
	for(auto p=args.getargs().begin();p!=args.getargs().end();p++){
		paramlength=fill_fcgi_params(buf,fd_fcgi,args.changetophpargs(p->first).c_str(),p->second.c_str());
		tophp.copysome(buf,paramlength);
	}
	if(strcasecmp(args.getmethod(),"GET")==0){
		paramlength=fill_fcgi_params(buf,fd_fcgi,"QUERY_STRING",args.getparameter());
		tophp.copysome(buf,paramlength);
	}
	fill_fcgi_header(buf,fd_fcgi,4,0);
	tophp.copysome(buf,8);
	if(strcasecmp(args.getmethod(),"POST")==0){
		paramlength=strlen(args.getparameter());
		fill_fcgi_header(buf,fd_fcgi,5,paramlength);
		tophp.copysome(buf,8);
		if(paramlength%8==0)
		tophp.copysome(args.getparameter(),paramlength);
		else
			tophp.copysome(args.getparameter(),paramlength+8-paramlength%8);
	}
	fill_fcgi_header(buf,fd_fcgi,5,0);
	tophp.copysome(buf,8);
}
void Connection::parsefcgiresponse(){
	while(1){
		unsigned char value4=static_cast<unsigned char>(*(fromphp.getoutptr()+4));
		unsigned char value5=static_cast<unsigned char>(*(fromphp.getoutptr()+5));
		int length=(value4<<8)+value5;
		int paddingsize=(*(fromphp.getoutptr()+6));
		if(fromphp.nowlength()>=8 && fromphp.nowlength()>=8+length+paddingsize){
			if(*(fromphp.getoutptr()+1)==FCGI_STDOUT){
				fromphp.addout(8);
				while(phpargstate<2){
					length-=parsephpheader();
				}
				if(phpargstate==2){
					tohttp.copystr("\r\n");
					char chunklen[10];
					sprintf(chunklen,"%x\r\n",length);
					tohttp.copystr(chunklen);
					tohttp.copysome(fromphp.getoutptr(),length);
					fromphp.addout(length);
				}
			}
			else if(*(fromphp.getoutptr()+1)==FCGI_STDERR){
				fromphp.addout(8);
				char errstr[MAXLINE];
				fromphp.readsome(errstr,length);
				fprintf(stderr,errstr);
				//handle error
			}
			else{
				phpargstate=3;
				fromphp.reinit();
				tohttp.copystr("\r\n0\r\n\r\n");
				return;
			}
			fromphp.addout(paddingsize);
		}
		else{
			logs::getinstance()->addbuf("Some errors in parsing php data\n");
			break;
		}
	}
}

int Connection::parsephpheader(){
	char buf[MAXLINE];
	fromphp.readline(buf);
	if(buf[0]=='\r' && buf[1]=='\n'){
		tohttp.copystr("Server: FML server\r\nConnection: keep-alive\r\nKeep-alive: timeout=15\r\nTransfer-Encoding: chunked\r\n");
		phpargstate=2;
		return 2;
	}
	if(phpargstate==0){
		if(strstr(buf,"Status: ")){
			logs::getinstance()->addbuf("php get other status\n");
			logs::getinstance()->addbuf(buf);
			char* p=index(buf,':');
			char tstr[30];
			strcat(tstr,"HTTP/1.1 ");
			strcat(tstr,p+2);
			tohttp.copystr(tstr);
		}
		else{
			tohttp.copystr("HTTP/1.1 200 OK\r\n");
			tohttp.copystr(buf);
		}
		phpargstate=1;
		return strlen(buf);
	}
	else{
		tohttp.copystr(buf);
		return strlen(buf);
	}
}

void Connection::clienterror(int fd,const char* cause,const char* errnum,const char* shortmsg,const char* longmsg) 
{
    char buf[MAXLINE];
	tohttp.reinit();

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    tohttp.copystr(buf);
	tohttp.copystr("Connection: keep-alive\r\nKeep-alive: timeout=15\r\nTransfer-Encoding: chunked\r\nContent-type: text/html\r\n\r\n");

    /* Print the HTTP response body */
    char lenbuf[10];
    sprintf(lenbuf,"%lx\r\n",100+strlen(errnum)+strlen(shortmsg)+strlen(longmsg)+strlen(cause));
	tohttp.copystr(lenbuf);
    sprintf(buf, "<html><title>FML server Error</title><body bgcolor=""ffffff"">\r\n");
    tohttp.copystr(buf);
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    tohttp.copystr(buf);
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    tohttp.copystr(buf);
    sprintf(buf, "<hr><em>The FML server</em>\r\n\r\n0\r\n\r\n");
    tohttp.copystr(buf);
}
