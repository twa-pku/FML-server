#include <set>
#include <map>
#include <string>
#include <string.h>
using namespace std;

class httpargs{
public:
	httpargs():parsestate(0){
		argswecare.insert(pair<string,string>("Cookie","HTTP_COOKIE"));
		argswecare.insert(pair<string,string>("Content-Length","CONTENT_LENGTH"));
		argswecare.insert(pair<string,string>("Content-Type","CONTENT_TYPE"));
	}
	~httpargs(){}
	int getstate(){return parsestate;}
	void setstate(int x){parsestate=x;}
	char* getmethod(){return method;}
	void setmethod(const char* x){strcpy(method,x);}
	char* getpath(){return path;}
	void setpath(const char* x){strcpy(path,x);}
	char* getparameter(){return parameter;}
	void setparameter(const char* x){strcpy(parameter,x);}
	bool inset(string x){return argswecare.find(x)!=argswecare.end();}
	void addset(string x,string y){
		args[x]=y;
	}
	void clearargs(){
		args.clear();
	}
	bool getisstatic(){return isstatic;}
	void setisstatic(bool x){isstatic=x;}
	map<string,string>& getargs(){return args;}
	string changetophpargs(string s){return argswecare[s];}
private:
	char method[10];
	char path[256];
	map<string,string> argswecare;
	map<string,string> args;
	char parameter[65536];
	int parsestate;
	bool isstatic;
};
