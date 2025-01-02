#include "Request.hpp"

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

