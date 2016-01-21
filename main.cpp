#include <iostream>
#include <stdlib.h>
#include "Parser.hpp"

using namespace std;

int main()
{

	char *http_header =	"GET /index.html HTTP/1.0\r\n"
						"Host: www.baidu.com\r\n"
						"Connection: keep-alive\r\n"
						"Accept: text/html,application/xml\r\n"
						"User-Agent: Mozilla/5.0 (Macintosh) \r\n"
						"Accept-Encoding: gzip, deflate, sdch\r\n"
						"Accept-Language: zh-CN,zh;q=0.8\r\n"
						"\r\n";

	cout<<strlen(http_header)<<endl;
	Parser parser(http_header,strlen(http_header));

	vector<string> allParamters= parser.getParameters();
	for(int i=0;i<allParamters.size();i++)
	{
		cout<<"Key: "<<allParamters[i]<<endl;
		cout<<"Value: " << parser.getValueByParameter(allParamters[i])<<endl;
	}
	cout<<parser.getMethod()<<endl;
	cout<<parser.getUri()<<endl;
	cout<<parser.getVersion()<<endl;
	return 0;
}
