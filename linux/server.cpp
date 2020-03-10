#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

using namespace std;

bool fileExist(char*);
void sucWeb(int&,char*); 
void notFoundWeb(int&);
void *response(void *sock);
int main(int arc,char *argv[])
{
	int ssock,csock;
	ssock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ssock==-1)
	{
		cout<<"Create faily"<<endl;
		return -1;
	}
	sockaddr_in adrs={0},adrc={0};
	adrs.sin_family=AF_INET;
	adrs.sin_port=htons(80);
	adrs.sin_addr.s_addr=htonl(INADDR_ANY);
	//绑定监听地址端口
	if(-1==bind(ssock,(sockaddr*)&adrs,sizeof(adrs)))
	{
		cout<<"绑定失败！"<<endl;
		return -1;
	}
	//开启监听80端口
	if(-1==listen(ssock,10))
	{
		cout<<"监听失败"<<endl;
	}
	//接收web客户端连接
	int len=sizeof(adrc);
	pthread_t thread[10];
	short i=0;
	while((csock=accept(ssock,(sockaddr*)&adrc,(socklen_t*)&len))!=-1)
	{
		pthread_create(&thread[i++],NULL,response,(void*)&csock);
		if(i==10)i=0;
	}
	return 0;
}

//查找文件是否存在
bool fileExist(char filepath[1024])
{
	char fpath[1024]={"./wwwroot/"};
	strcat(fpath,filepath);
	ifstream ifs;
	ifs.open(fpath,ios::in);
	if(!ifs.is_open())
		return false;
	ifs.close();
	return true;
}

//返回成功查找到的页面，状态码：200
void sucWeb(int &sock,char filepath[1024])
{
	char head[1024];
	memset(head,0,1024);
	strcat(head,"HTTP/1.1 200 OK\r\n");
	strcat(head,"Content-Type: text/html\r\n");
	strcat(head,"Content-Encoding: utf-8\r\n");
	strcat(head,"\r\n");
	cout<<"发送数据"<<endl;
	char buf;
	ifstream ifs;
	ifs.open(filepath,ios::in);
	while(!ifs.eof())
	{
		ifs.get(buf);cout<<buf;
		send(sock,&buf,1,0);
	}
	ifs.close();
}

//未查找到界面,状态码：404
void notFoundWeb(int &sock)
{
	char head[512];
	memset(head,0,512);
	strcat(head,"HTTP/1.1 404 Not Found\r\n");
	strcat(head,"Content-type: text/html\r\n");
	strcat(head,"Content-Encoding: utf-8\r\n");
	strcat(head,"\r\n");
	cout<<"发送头部长度："<<strlen(head)<<endl;
	send(sock,head,strlen(head),0);
	cout<<"未查找到，返回404"<<endl;
	char buf;
	ifstream ifs;
	ifs.open("./error/404.html",ios::in);
	while(!ifs.eof())
	{
		ifs.get(buf);cout<<buf;
		send(sock,&buf,1,0);
	}
	ifs.close();
	
}

//创建线程进行请求处理
void *response(void*sock)
{
	cout<<"接收头部"<<endl;
	int csock=*((int*)sock);
	char head[1024];
	memset(head,0,1024);
	char buf;
	while(1)
	{
		recv(csock,&buf,1,0);cout<<buf;strcat(head,&buf);
		if(buf=='\r')
		{
			recv(csock,&buf,1,0);cout<<buf;strcat(head,&buf);
			if(buf=='\n')
			{
				recv(csock,&buf,1,0);cout<<buf;strcat(head,&buf);
				if(buf=='\r')
				{
					recv(csock,&buf,1,0);cout<<buf;strcat(head,&buf);
					if(buf=='\n')
					{
						//notFoundWeb(csock);
						char path[1024]="./wwwroot/index.html";
						sucWeb(csock,path);
						/*char fpath[1024]="index.html";
						if(!fileExist(fpath))cout<<"文件不存在"<<endl;*/
						break;
					}
				}
			}
		}
	}
	//提取请求路径
	//char fpath[1024];
	//memset(fpath,0,1024);
	//cout<<strstr(head,"HTTP");
	/*for(short i=0;i<strlen(head)&&(&head[i])!=pos;i++)
	{
		strcat(fpath,&head[i]);
	}
	cout<<"提取到的请求文件为："<<fpath;
	*/close(csock);
}
