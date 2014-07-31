/*
 * WebCrawler: http_request.hpp
 * Copyright (C) 2014 Kyle Givler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file http_request.hpp
 * @author Kyle Givler
 */

#ifndef _WC_HTTP_REQUEST_H_
#define _WC_HTTP_REQUEST_H_

#include <boost/asio.hpp>
#include <gumbo.h>
#include <string>
#include <vector>
#include <logger/logger.hpp>

enum class RequestType { HEAD, GET, CRAWL };

class http_request
{
public:
  http_request(std::string server = "NULL", 
    std::string path = "/", 
    unsigned int port = 80);
  
  virtual ~http_request();

  /**
   * @return port associated with this http_request
   */
  unsigned int get_port() { return this->port; }
  
  /**
   * @param port port associated with this http_request
   */
  void set_port(unsigned int port) { this->port = port; }
  
  /**
   * @return server associated with this http_request
   */
  std::string get_server() { return this->server; }
  
  /**
   * @param server Server associated with this http_request
   */
  void set_server(std::string server) { this->server = server; }
  
  /**
   * @return Path/Resource associated with this http_request
   */
  std::string get_path() { return this->path; }
  
  /**
   * @param path Path/Resource associated with this http_request
   */
  void set_path(std::string) { this->path = path; }

  /**
   * @return The HTTP version the server is using
   */
  std::string get_http_version() { return this->http_version; }

  /**
   * @param version The HTTP version the server is using
   */
  void set_http_version(std::string version) {this->http_version = version;}

  /**
   * @return The status code that the server responded with
   */
  unsigned int get_status_code() { return this->status_code; }
  
  /**
   * @param code The status code the server responded with
   */
  void set_status_code(unsigned int code) { this->status_code = code; }
  
  /**
   * @return The raw data that the server returned
   */
  std::string& get_data() { return this->data; }
  
  /**
   * @param header The header to add
   */
  void add_header(std::string header) { this->headers.push_back(header); }
  
  /**
   * @return A vector of the headers that the server responded with
   */
  std::vector<std::string> get_headers() { return this->headers; }
  
  /**
   * @param request The http request to make
   */
  void set_request(std::string request) { this->request = request; }
  
  /**
   * @return The http request to make
   */
  std::string get_request() { return this->request; }
  
  /**
   * @param error The error to add
   */
  void add_error(std::string error) { errors.push_back(error); }
  
  /**
   * @return true on error, false on success
   */
  bool error() { return (!errors.empty()); }
  
  /**
   * @return a vector of errors
   */
  std::vector<std::string> get_errors() { return this->errors; }

  /**
   * @return This request's response buffer
   */
  boost::asio::streambuf& get_response_buf() {return this->response_buf; }
  
  /**
   * @return This request's request buffer
   */
  boost::asio::streambuf& get_request_buf() {return this->request_buf; }
  
  /**
   * @param type The type of request to make.
   * Currently supported: GET and HEAD
   * CRAWL mean do a HEAD request the a GET request
   */
  void set_request_type(RequestType type) { this->type = type; }
  
  /**
   * @return This request's type
   */
  RequestType get_request_type() { return this->type; }
  
  /**
   * @return all links to other pages
   */
  std::vector<std::string> get_links();

  /**
   * Reset buffers
   */
  void reset_buffers()
  {
    response_buf.consume(response_buf.size());
    request_buf.consume(request_buf.size());
  }

  /**
   * Reset Errors
   */
  void reset_errors()
  {
    while(!errors.empty())
      errors.pop_back();
  }

  /**
   * @return True if the request completed, false if it is in progress
   */
  bool is_completed() { return this->requestCompleted; }
  
  /**
   * @param completed True if completed, false if in progress
   */
  void set_completed(bool completed) { this->requestCompleted = completed; }
  
  /**
   * @param proto This request's protocol
   */
  bool set_protocol(std::string proto)
  {
    if(proto == "http" || proto == "https")
    {
      this->protocol = proto;
      if(this->protocol == "https")
        set_port(443);
      return true;
    }
  return false;
  }
  
  /**
   * @return This request's protocol
   */
  std::string get_protocol() { return this->protocol; }
  
  void should_blacklist(bool blacklist, std::string reason = "default") 
  { 
    this->blacklist = blacklist;
    this->blacklist_reason = reason;
  }
  
  bool should_blacklist() { return this->blacklist;  }

  std::string get_blacklist_reason() { return blacklist_reason; }

private:
  std::string server = "NULL";
  std::string path = "NULL";
  unsigned int port = 80;
  RequestType type = RequestType::GET;
  boost::asio::streambuf response_buf;
  boost::asio::streambuf request_buf;
  std::vector<std::string> errors;
  std::string http_version = "NULL";
  unsigned int status_code = 0;
  std::vector<std::string> headers;
  std::string data;
  std::string request;
  bool requestCompleted = false;
  Logger logger;
  std::string protocol = "http";
  bool blacklist = false;
  std::string blacklist_reason;
  
  void search_for_links(GumboNode *node, std::vector<std::string> &links);
};

#endif
