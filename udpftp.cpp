#include <iostream>
#include <ctime>
#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <set>
#include <functional>
#include <istream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
enum {DEFAULT_BUFF_SIZE=1024};
using namespace std;
using namespace boost::asio;
using  boost::asio::ip::udp;
using namespace  boost::filesystem;
class Command {
public:
	virtual void execute(){};
};
class GetCommand:public Command {
public:
	GetCommand(shared_ptr<udp::socket> socket,const path& path,const udp    ::endpoint& remoteend):socket(socket),m_path(path),remote_end(remoteend) {
	}
	void execute() override {
	     ifstream is;
	     char buffer[DEFAULT_BUFF_SIZE];
	     size_t len;
	     string statusline;
	     is.open(m_path.string(),ios::binary);
	     if(is) {
	     statusline="200 OK\n";
	     socket->send_to(boost::asio::buffer(statusline),remote_end);
	     is.seekg(0,ios::end);
	     len=is.tellg();
	     is.seekg(0,ios::beg);		
	     socket->send_to(boost::asio::buffer(&len,sizeof(len)),remote_end);
	     while((len=is.readsome(buffer,DEFAULT_BUFF_SIZE))>0)
		{
			socket->send_to(boost::asio::buffer(buffer,len),remote_end);
		} 
	     is.close();	
		}
	     else {
	     statusline="404 FILE NOT EXISTS\n";
	     socket->send_to(boost::asio::buffer(statusline),remote_end);
		}
	}
	
private:
shared_ptr<udp::socket> socket;
path m_path;
udp::endpoint remote_end;
};
class LsCommand:public Command {
public:
	LsCommand(shared_ptr<udp::socket> socket,const path& path,const udp    ::    endpoint& remoteend):socket(socket),m_path(path),remote_end(remoteend) {
	}
	void execute() override {
	stringstream result;
	string statusline;
	if(is_directory(m_path))
	{
        statusline="200 OK\n";
        socket->send_to(boost::asio::buffer(statusline),remote_end);
	directory_iterator it{m_path};
	for(;it!=directory_iterator{};it++)
	{
		result<<it->path().string()<<endl;
	}
	auto strval=result.str();
	size_t len=strval.length();
	socket->send_to(boost::asio::buffer(&len,sizeof(len)),remote_end);
	const char *ptr=strval.c_str();	
	size_t sendlen=0;
	while(sendlen<len)
	{
		size_t slen=(len-sendlen)>512 ? 512:(len-sendlen);
		slen=socket->send_to(boost::asio::buffer(ptr,slen),remote_end);
		cout<<sendlen<<" "<<slen<<endl;
		ptr+=slen;
		sendlen+=slen;

	}
	}
	else
	{
		
	     statusline="404 DIR NOT EXISTS OR is not dircotry\n";
	     socket->send_to(boost::asio::buffer(statusline),remote_end);
	}
	}
private:
shared_ptr<udp::socket> socket;
path m_path;
udp::endpoint remote_end;
};
class UnknownCommand:public Command {
public:
	UnknownCommand(shared_ptr<udp::socket> socket,const string&cmd,udp::endpoint point):socket(socket),mcmd(cmd),remote_end(point) {}
	void execute() override {
	}
private:
	shared_ptr<udp::socket> socket;
	udp::endpoint remote_end;
	string mcmd;
};
int main() {
io_service is;
char buff[DEFAULT_BUFF_SIZE];
vector<shared_ptr<thread>> threads;
set<udp::endpoint> clients;
udp::endpoint ep(ip::udp::v4(),6555);
udp::endpoint re;
shared_ptr<udp::socket> ac(new udp::socket(is,ep));
for(;;) {
	auto len=ac->receive_from(boost::asio::buffer(buff,DEFAULT_BUFF_SIZE),re);
	string line(buff,len);
	string cmdverb,p1,p2;
	stringstream stream(line);
	stream>>cmdverb;
	if(cmdverb=="get")
	{
		stream>>p1;
		auto cmd=make_shared<GetCommand>(ac,p1,re);
		shared_ptr<thread> t(new thread(bind(&GetCommand::execute,cmd)));
		threads.push_back(t);
	}
	else if(cmdverb=="ls")
	{
		stream>>p1;
		auto cmd=make_shared<LsCommand>(ac,p1,re);
		shared_ptr<thread> t(new thread(bind(&LsCommand::execute,cmd)));
		threads.push_back(t);
	}
	else {
		cout<<"unknown command"<<endl;	
		auto cmd=make_shared<UnknownCommand>(ac,cmdverb,re);
		shared_ptr<thread> t(new thread(bind(&UnknownCommand::execute,cmd)));
		threads.push_back(t);

	}
	
}
return 0;
}
