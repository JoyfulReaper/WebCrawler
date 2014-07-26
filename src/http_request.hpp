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


#ifndef _WC_HTTP_REQUEST_H_
#define _WC_HTTP_REQUEST_H_

#include <string>
#include <sstream>

class http_request
{
public:
  http_request(std::string server = "NULL", unsigned int port = 80);
  
  virtual ~http_request();

  unsigned int get_port() { return this->port; }
  
  void set_port(unsigned int port) { this->port = port; }
  
  std::string get_server() { return this->server; }
  
  void set_server(std::string) { this->server = server; }
  
  std::string get_path() { return this->path; }
  
  void set_path(std::string) { this->path = path; }

  std::string get_http_version() { return this->http_version; }

  void set_http_version(std::string version) {this->http_version = version;}

  unsigned int get_status_code() { return this->status_code; }
  
  void set_status_code(unsigned int code) { this->status_code = code; }
  
  std::stringstream& get_data() { return this->data; }
  
  //void set_data(std::stringstream data) {this->data = data; }
  
  std::stringstream& get_headers() { return this->headers; }
  
  //void set_headers(std::stringstream headers) {this->headers = headers; }
  
  void set_request(std::string request) { this->request = request; }
  
  std::string get_request() { return this->request; }

private:
  unsigned int port = 80;
  std::string server = "NULL";
  std::string path = "NULL";
  std::string http_version = "NULL";
  unsigned int status_code = 0;
  std::stringstream headers;
  std::stringstream data;
  std::string request;
};

#endif
