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
#include <memory>
#include <logger/logger.hpp>

enum class RequestType { HEAD, GET};
class request_reciver;

class http_request
{
public:
  http_request(request_reciver &reciver,
    std::string server, 
    std::string path,
    std::string protocol);
  
  virtual ~http_request();

  /**
   * Send the completed http_request to the caller
   */
  void call_request_reciver(http_request *r);

  /**
   * @return port associated with this http_request
   */
  unsigned int get_port() const { return this->port; }
  
  /**
   * @param port port associated with this http_request
   */
  void set_port(unsigned int port) { this->port = port; }
  
  /**
   * @return server associated with this http_request
   */
  std::string get_server() const { return this->server; }
  
  /**
   * @param server Server associated with this http_request
   */
  void set_server(std::string server) { this->server = server; }
  
  /**
   * @return Path/Resource associated with this http_request
   */
  std::string get_path() const { return this->path; }
  
  /**
   * @param path Path/Resource associated with this http_request
   */
  void set_path(std::string path) { this->path = path; }

  /**
   * @return The HTTP version the server is using
   */
  std::string get_http_version() const { return this->http_version; }

  /**
   * @param version The HTTP version the server is using
   */
  void set_http_version(std::string version) {this->http_version = version;}

  /**
   * @return The status code that the server responded with
   */
  unsigned int get_status_code() const { return this->status_code; }
  
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
  std::vector<std::string> get_headers() const { return this->headers; }
  
  /**
   * @param request The http request to make
   */
  void set_request(std::string request) { this->request = request; }
  
  /**
   * @return The http request to make
   */
  std::string get_request() const { return this->request; }
  
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
  std::string get_protocol() const { return this->protocol; }
  
  /**
   * @param blacklist true if the URL should be blacklisted, false if not
   * @param reason the reason for the blacklisting
   */
  void should_blacklist(bool blacklist, std::string reason = "default") 
  { 
    this->blacklist = blacklist;
    this->blacklist_reason = reason;
  }
  
  /**
   * @return true if this URL should be blacklisted, false if not
   */
  bool should_blacklist() { return this->blacklist;  }

  /**
   * @return the reason why this URL should be blacklisted
   */
  std::string get_blacklist_reason() { return blacklist_reason; }
  
  /**
   * @param timedOut true if the request timed out, false if it didn't
   */
  void set_timed_out(bool timedOut) { this->timed_out = timedOut; }
  
  /**
   * @return true if the request timedout, false if it didn't
   */
  bool get_timed_out() {return this->timed_out; }

private:
  std::string server = "NULL";
  std::string path = "NULL";
  std::string data;
  std::string request;
  std::string http_version = "NULL";
  std::string protocol = "http";
  std::string blacklist_reason = "default";
  unsigned int port = 80;
  RequestType type = RequestType::GET;
  boost::asio::streambuf response_buf;
  boost::asio::streambuf request_buf;
  std::vector<std::string> errors;
  std::vector<std::string> headers;
  unsigned int status_code = 0;
  bool requestCompleted = false;
  bool blacklist = false;
  bool timed_out = false;
  request_reciver *reciver;
  Logger logger;
  
  void search_for_links(GumboNode *node, std::vector<std::string> &links);
};

#endif
