#include "Request.hpp"
#include <iomanip>

Request::Request()
{
}

void Request::setMethod(const std::string &method)
{
    this->method = method;
}

void Request::setPath(const std::string &path)
{
    this->path = path;
}

void Request::setDecodedPath(const std::string &decoded_path)
{
    this->decoded_path = decoded_path;
}

void Request::setVersion(const std::string &version)
{
    this->version = version;
}

void Request::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->headers = headers;
}

std::string Request::generateErrorPage(const int code)
{
    std::stringstream errorPage;
    switch (code)
    {
    case 400:
        errorPage << "<html><body> <center> <h1>" << code << " BAD REQUEST" << "</h1></center></body></html>";
        break;
    case 404:
        errorPage << "<html><body> <center> <h1>" << code << " NOT FOUND" << "</h1></center></body></html>";
        break;
    case 405:
        errorPage << "<html><body> <center> <h1>" << code << " NOT IMPLEMENTED" << "</h1></center></body></html>";
        break;
    case 414:
        errorPage << "<html><body> <center> <h1>" << code << " URI TOO LONG " << "</h1></center></body></html>";
        break;
    case 505:
        errorPage << "<html><body> <center> <h1>" << code << " HTTP Version Not Supported" << "</h1></center></body></html>";
        break;
    default:
        break;
    }
    return errorPage.str();
}

std::string Request::generateStatusMsg(const int code)
{
    std::string msg = "";
    switch (code)
    {
    case 400:
        msg = "BAD REQUEST";
        break;
    case 404:
        msg = "NOT FOUND";
        break;
    case 405:
        msg = "NOT ALLOWED";
        break;
    case 414:
        msg = "URI TOO LONG";
        break;
    case 403:
        msg = "FORBIDDEN";
        break;
    case 505:
        msg = "HTTP Version Not Supported";
        break;

    default:
        break;
    }

    return msg;
}

void Request::setBody(const std::string &body)
{
    this->body = body;
}

void Request::setQueryParams(const std::map<std::string, std::string> &query_params)
{
    this->query_params = query_params;
}

const std::string &Request::getMethod() const
{
    return method;
}

const std::string &Request::getPath() const
{
    return path;
}

const std::string &Request::getDecodedPath() const
{
    return decoded_path;
}

const std::string &Request::getVersion() const
{
    return version;
}

const std::map<std::string, std::string> &Request::getHeaders() const
{
    return headers;
}

const std::string &Request::getBody() const
{
    return body;
}

const std::map<std::string, std::string> &Request::getQueryParams() const
{
    return query_params;
}
