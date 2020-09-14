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
int main() {
io_service is;
char buff[DEFAULT_BUFF_SIZE];
udp::endpoint host(ip::address::from_string("127.0.0.1"),6555);
udp::endpoint send_point;
udp::socket client(is);
client.open(ip::udp::v4());
string line;
string cmdverb;
string p1,p2;
string msg;
int status;
size_t len,bufflen,rlen;
for(;;) {
	cout<<">";
	getline(cin,line);
	stringstream stream(line);
	stream>>cmdverb;
	if(cmdverb=="ls") {
		stream>>p1;
		client.send_to(boost::asio::buffer(cmdverb+" "+p1+"\n"),host);
		len=client.receive_from(boost::asio::buffer(buff,DEFAULT_BUFF_SIZE),send_point);
		stringstream streamls(string(buff,len));
		streamls>>status;
		std::getline(streamls,msg);	
		if(status==200)
		{
			client.receive_from(boost::asio::buffer(&bufflen,sizeof(bufflen)),send_point);
			rlen=0;
			while(rlen<bufflen)
			{
				len=client.receive_from(boost::asio::buffer(buff,DEFAULT_BUFF_SIZE    ),send_point);
				rlen+=len;
				buff[len]='\0';
				cout<<buff;
			}
		}

		else
		{
			cout<<"Error: "<<msg<<endl;
		}
	}
	else if(cmdverb=="get") {
		stream>>p1>>p2;
		client.send_to(boost::asio::buffer(cmdverb+" "+p1+"\n"),host);
		len=client.receive_from(boost::asio::buffer(buff,DEFAULT_BUFF_SIZE    ),send_point);
		stringstream streamget(string(buff,len));
		streamget>>status;
		std::getline(streamget,msg);
		if(status==200)
		{
			ofstream out(p2,ofstream::binary);	
			client.receive_from(boost::asio::buffer(&bufflen,sizeof(bufflen)),send_point);
			rlen=0;
			while(rlen<bufflen)
			{
				len=client.receive_from(boost::asio::buffer(buff,DEFAULT_BUFF_SIZE),send_point);
				rlen+=len;
			out.write(buff,len);
			}
			out.close();
			
		}
		else
		{
			cout<<"Error: "<<msg<<endl;
		}


	}
	else {
		cout<<"unknown command"<<endl;
	}		
}
return 0;
}
