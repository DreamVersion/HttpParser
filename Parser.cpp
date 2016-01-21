#include "Parser.hpp"

#define LF     (char)10
#define CR     (char)13

std::vector<std::string> Parser::getParameters()
{
    std::vector<std::string> result;
    std::map<std::string, std::string>::iterator it = KVMap.begin();
    for( ; it!=KVMap.end() ; it++)
        result.push_back(it->first);
    return result;
}

std::string Parser::getValueByParameter(std::string parameter)
{
    std::string result;
    if(KVMap.find(parameter) != KVMap.end())
        result = KVMap[parameter];
    return result;
}

int Parser::parse_packet()
{
    pos = packet;
    int ret = parse_request_line();
    switch(ret) {
        case HTTP_PARSE_INVALID_REQUEST:
        case HTTP_PARSE_IGNORED_METHOD:
            return HTTP_ERR;
        default:
            break;
    }

    for(;;)
    {
        ret = parse_header_line();

        if (ret == HTTP_PARSE_HEADER_DONE) {
            break;
        }
        else if (ret == HTTP_PARSE_AGAIN && HTTP_PARSE_INVALID_HEADER == ret) {
            return HTTP_ERR;
        }
    }
    return ret;

}

int Parser::parse_request_line()
{
    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
        sw_uri,
        sw_http_09,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    };

    uint32_t state = sw_start;
    uint32_t http_minor = 0;
    uint32_t http_major = 0;

    char ch,*p=NULL;

    char *method_start=NULL,*method_end=NULL;
    char *uri_start=NULL,*uri_end=NULL;

    for(p=pos; p<packet + length; p++)
    {
        ch=*p;
        switch(state)
        {
            case sw_start:
                if(ch<'A' || ch>'Z')
                    return HTTP_PARSE_INVALID_REQUEST;
                method_start=p;
                state = sw_method;
                break;
            case sw_method:
                if(ch == ' ') {
                    method_end=p;
                    state = sw_spaces_before_uri;
                    method = std::string(method_start,method_end-method_start);
                }
                else
                {
                    if(ch>'Z' || ch <'A')
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
            case sw_spaces_before_uri:
                if (ch != ' ') {
                    uri_start = p;
                    state = sw_uri;
                }
                break;
            case sw_uri:
                switch (ch) {
                    case ' ':
                        uri_end = p;
                        state = sw_http_09;
                        uri = std::string(uri_start,uri_end-uri_start);
                        break;
                    case CR:
                        uri_end = p;
                        http_minor = 9;
                        state = sw_almost_done;
                        break;
                    case LF:
                        uri_end = p;
                        http_minor = 9;
                        pos = p + 1;
                        version = http_major * 1000 + http_minor;
                        return HTTP_OK;
                    case '\0':
                        return HTTP_PARSE_INVALID_REQUEST;
                    default:break;
                }
                break;
            case sw_http_09:
                switch (ch) {
                    case ' ':
                        break;
                    case CR:
                        http_minor = 9;
                        state = sw_almost_done;
                        break;
                    case LF:
                        http_minor = 9;
                        pos = p + 1;
                        version = http_major * 1000 + http_minor;
                        return HTTP_OK;
                    case 'H':
                        state = sw_http_H;
                        break;
                    default:
                        state = sw_uri;
                        break;
                }
                break;
            case sw_http_H:
                switch (ch) {
                    case 'T':
                        state = sw_http_HT;
                        break;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
            case sw_http_HT:
                switch (ch) {
                    case 'T':
                        state = sw_http_HTT;
                        break;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
            case sw_http_HTT:
                switch (ch) {
                    case 'P':
                        state = sw_http_HTTP;
                        break;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
            case sw_http_HTTP:
                switch (ch) {
                    case '/':
                        state = sw_first_major_digit;
                        break;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
                // first digit of major HTTP version
            case sw_first_major_digit:
                if (ch < '1' || ch > '9') {
                    return HTTP_PARSE_INVALID_REQUEST;
                }
                http_major = ch - '0';
                state = sw_major_digit;
                break;
                // major HTTP version or dot
            case sw_major_digit:
                if (ch == '.') {
                    state = sw_first_minor_digit;
                    break;
                }
                if (ch < '0' || ch > '9') {
                    return HTTP_PARSE_INVALID_REQUEST;
                }
                http_major = http_major * 10 + ch - '0';
                break;
                // first digit of minor HTTP version
            case sw_first_minor_digit:
                if (ch < '0' || ch > '9') {
                    return HTTP_PARSE_INVALID_REQUEST;
                }
                http_minor = ch - '0';
                state = sw_minor_digit;
                break;
                // minor HTTP version or end of request line
            case sw_minor_digit:
                if (ch == CR) {
                    state = sw_almost_done;
                    break;
                }
                if (ch == LF) {
                    pos = p + 1;
                    version = http_major * 1000 + http_minor;
                    return HTTP_OK;
                }
                if (ch == ' ') {
                    state = sw_spaces_after_digit;
                    break;
                }
                if (ch < '0' || ch > '9') {
                    return HTTP_PARSE_INVALID_REQUEST;
                }
                http_minor = http_minor * 10 + ch - '0';
                break;
            case sw_spaces_after_digit:
                switch (ch) {
                    case ' ':
                        break;
                    case CR:
                        state = sw_almost_done;
                        break;
                    case LF:
                        pos = p + 1;
                        version = http_major * 1000 + http_minor;
                        return HTTP_OK;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
                break;
                // end of request line
            case sw_almost_done:
                switch (ch) {
                    case LF:
                        pos = p + 1;
                        version = http_major * 1000 + http_minor;
                        return HTTP_OK;
                    default:
                        return HTTP_PARSE_INVALID_REQUEST;
                }
        }
    }
    pos = p;

    return HTTP_PARSE_HEADER_DONE;
}

int Parser::parse_header_line()
{
    enum {
        sw_start = 0,
        sw_name,
        sw_space_before_value,
        sw_value,
        sw_space_after_value,
        sw_ignore_line,
        sw_almost_done,
        sw_header_almost_done
    };

    char ch, *p=NULL;
    uint32_t state = sw_start;
    char *header_name_start=NULL,*header_name_end=NULL;
    char *header_start=NULL,*header_end=NULL;

    for(p = pos; p < packet +length; p++)
    {
        ch = *p;
        switch(state)
        {
            case sw_start:
            {
                header_name_start = p;
                switch (ch) {
                    case CR:
                        header_end = p;
                        state = sw_header_almost_done;
                        break;
                    case LF:
                        header_end = p;
                        pos = p + 1;
                        return HTTP_PARSE_HEADER_DONE;
                    default:
                        state = sw_name;
                        break;
                }
            }
            break;

            case sw_name:
            {
                switch(ch)
                {
                    case ':':
                        header_name_end = p;
                        state = sw_space_before_value;
                        break;
                    case CR:
                        header_name_end = p;
                        header_start = p;
                        header_end = p;
                        state = sw_almost_done;
                        break;
                    case LF:
                        header_name_end = p;
                        header_start = p;
                        header_end = p;
                        pos = p + 1;
                        return 0;
                    default:
                        break;
                }
            }
            break;

            case sw_space_before_value:
            {
                switch (ch) {
                    case ' ':
                        break;
                    case CR:
                        header_start = p;
                        header_end = p;
                        state = sw_almost_done;
                        break;
                    case LF:
                        header_start = p;
                        header_end = p;
                        pos = p + 1;
                        return 0;
                    case '\0':
                        return HTTP_PARSE_INVALID_HEADER;
                    default:
                        header_start = p;
                        state = sw_value;
                        break;
                }
            }
            break;
            case sw_value:
            {
                switch (ch) {
                    case ' ':
                        header_end = p;
                        state = sw_space_after_value;
                        break;
                    case CR:
                        header_end = p;

                        state = sw_almost_done;
                        break;
                    case LF:
                        header_end = p;
                        pos = p + 1;
                        return 0;
                    case '\0':
                        return HTTP_PARSE_INVALID_HEADER;
                }
            }
            break;
            case sw_space_after_value:
            {
                switch (ch) {
                    case ' ':
                        break;
                    case CR:

                        state = sw_almost_done;
                        break;
                    case LF:
                        pos = p + 1;
                        return 0;
                    case '\0':
                        return HTTP_PARSE_INVALID_HEADER;
                    default:
                        state = sw_value;
                        break;
                }
            }
            break;
            case sw_ignore_line:
            {
                switch (ch) {
                    case LF:
                        state = sw_start;
                        break;
                    default:
                        break;
                }
            }
            break;
            case sw_almost_done:
            {
                switch (ch) {
                    case LF:
                        pos = p + 1;
                        if(header_name_start!=NULL
                           &&header_name_end>header_name_start
                           && header_start!=NULL
                           && header_end>header_start)
                        {
                            std::string key = std::string(header_name_start,header_name_end-header_name_start);
                            std::string value = std::string(header_start,header_end-header_start);
                            KVMap[key]=value;
                        }

                        return 0;
                    case CR:
                        break;
                    default:
                        return HTTP_PARSE_INVALID_HEADER;
                }
            }
            break;
            case sw_header_almost_done:
            {
                switch (ch) {
                    case LF:
                        pos = p + 1;
                        return HTTP_PARSE_HEADER_DONE;
                    case CR:
                        break;
                    default:
                        return HTTP_PARSE_INVALID_HEADER;
                }
            }
            break;

        }
    }

    pos = p;
    return HTTP_PARSE_AGAIN;
}