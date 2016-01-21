#include <map>
#include <vector>
#include <string>

#define HTTP_ERR -1
#define HTTP_OK 0
#define HTTP_PARSE_IGNORED_METHOD      10
#define HTTP_PARSE_INVALID_REQUEST     11
#define HTTP_PARSE_INVALID_HEADER      13

#define HTTP_PARSE_AGAIN  (-2)
#define HTTP_PARSE_HEADER_DONE         1

#define HTTP_VERSION_9                 9
#define HTTP_VERSION_10                1000
#define HTTP_VERSION_11                1001

#define HTTP_METHOD_GET                 0x0002
#define HTTP_METHOD_POST                0x0003
#define HTTP_METHOD_HEAD                0x0004
#define HTTP_METHOD_OPTIONS             0x0005
#define HTTP_METHOD_PUT                 0x0006
#define HTTP_METHOD_DELETE              0x0007
#define HTTP_METHOD_TRACE               0x0008

class Parser
{
public:
    Parser(char *data,int len):
            packet(data),length(len){
        parse_packet();
    }

    //Get All Command in Http Head
    std::vector<std::string> getParameters();
    //Query the value of corresponding command
    std::string getValueByParameter(std::string parameter);
    inline int getVersion(){return version;}
    inline std::string getMethod(){return method;}
    inline std::string getUri(){return uri;}
private:
    /**
     * Parse the Http Packet
     */
    int parse_packet();
    int parse_header_line();
    int parse_request_line();

private:
    int length;
    int version;
    char *packet;
    char *pos;//record index of parse process
    std::string method;
    std::string uri;
    std::map<std::string, std::string> KVMap;
};
