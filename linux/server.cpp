#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <vector>
using namespace std;

bool fileExist(char*);
void sucWeb(int&,char*); 
void notFoundWeb(int&);
void *response(void *sock);
void sendFavicon(int&);
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
		//response((void*)&csock);
	}
	return 0;
}

//查找文件是否存在
bool fileExist(char filepath[1024])
{
	cout<<"检测得路径为:"<<filepath<<endl;
	ifstream ifs;
	ifs.open(filepath,ios::in);
	if(ifs.is_open())
	{	ifs.close();
		return true;}
	ifs.close();
	return false;
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
	send(sock,head,strlen(head),0);
	char buf;
	ifstream ifs;
	ifs.open(filepath,ios::in|ios::binary);
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
	char sendhead[512];
	memset(sendhead,0,512);
	strcat(sendhead,"HTTP/1.1 404 Not Found\r\n");
	strcat(sendhead,"Content-type: text/html\r\n");
	strcat(sendhead,"Content-Encoding: utf-8\r\n");
	strcat(sendhead,"\r\n");
	cout<<"发送头部长度："<<strlen(sendhead)<<endl;
	send(sock,sendhead,strlen(sendhead),0);
	cout<<"未查找到，返回404"<<endl;
	char filebuf;
	ifstream ifs;
	ifs.open("./error/404.html",ios::in);
	while(!ifs.eof())
	{
		ifs.get(filebuf);cout<<filebuf;
		send(sock,&filebuf,1,0);
	}
	ifs.close();
	
}
void sendFavicon(int &sock)
{
	char head[1024];
        memset(head,0,1024);
        strcat(head,"HTTP/1.1 200 OK\r\n");
        strcat(head,"Content-Type: image/x-icon\r\n");
        //strcat(head,"Content-Encoding: utf-8\r\n");
        strcat(head,"\r\n");
        cout<<"发送数据"<<endl;
	char buf;
        ifstream ifs;
        ifs.open("./wwwroot/favicon.ico",ios::in|ios::binary);
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
	char head[4098];
	memset(head,0,4098);;
	char buf[2]={0};
	string allrequest;
	char bufarray[1024];
	int rc=0;//头部数据行数
	while(1)
	{
		if(1024>recv(csock,bufarray,1024,0)){
			allrequest.append(bufarray);
			break;}
		allrequest.append(bufarray);
	}cout<<"请求数据长度"<<allrequest.length()<<endl;
	cout<<"数据读取完成"<<endl;
	for(int i=0;i<allrequest.length();i++)
	{	
		buf[0]=allrequest[i];cout<<buf[0];strcat(head,buf);
		if(buf[0]=='\r')
		{
			i++;buf[0]=allrequest[i];cout<<buf[0];strcat(head,buf);
			if(buf[0]=='\n')
			{	
				rc++;
				i++;buf[0]=allrequest[i];cout<<buf[0];strcat(head,buf);
				if(buf[0]=='\r')
				{
					i++;buf[0]=allrequest[i];cout<<buf[0];strcat(head,buf);
					if(buf[0]=='\n')
					{
						//notFoundWeb(csock);
						char pathcs[1024]="./wwwroot/index.html";
						//sucWeb(csock,pathcs);cout<<strlen(head);
						/*char fpath[1024]="index.html";
						if(!fileExist(fpath))cout<<"文件不存在"<<endl;*/
						break;

					}
				}
			}
		}
	}
	//提取请求路径
	char path[1024];//路径数组
	memset(path,0,1024);
	char rdata[1024]={0};//头部行中间数据
	cout<<"头部长度:"<<strlen(head)<<endl;
	int len=strlen(head),r=0;
	vector <string>headdata;//存储头部各段
	for(int i=0;i<len;i++)
	{
		if(head[i]==' ' || head[i]=='\r' || head[i]=='\n')
		{	if(head[i]=='\n')continue;
			headdata.push_back(rdata);
			cout<<"提取的数据:"<<headdata[r++]<<endl;memset(rdata,'\0',1024);
		}
		else
		{	buf[0]=head[i];
			strcat(rdata,buf);
		}
        }
	//参数解析
	strcat(path,"./wwwroot");
	
	/*if(headdata[1]=="/favicon.ico")
	{
		sendFavicon(csock);
		//char paths[]="./wwwroot/index.html";sucWeb(csock,paths);
		close(csock);
		return NULL;
	}*/
	if(headdata[1]=="/")headdata[1].append("index.html");
		
	strcat(path,headdata[1].c_str());
	cout<<"string路径长度1:"<<headdata[1].length();
	//path[strlen(path)-1]='\0';
	cout<<"路径长度2:"<<strlen(path);
	
	//for(int i=0;i<strlen(path);i++)cout<<"序号"<<i<<"内容"<<path[i]<<"码值"<<(int)path[i]<<endl;

	ifstream ifcs;ifcs.open(path);if(ifcs.is_open())cout<<"打开成功!"<<endl;else cout<<"打开失败!"<<endl;ifcs.close();
	if(fileExist(path))
		sucWeb(csock,path);
	else
		notFoundWeb(csock);
	
	close(csock);
}
