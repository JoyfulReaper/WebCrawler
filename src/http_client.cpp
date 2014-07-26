/*
 * WebCrawler: http_client.cpp
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

#include "http_client.hpp"
#include "http_request.hpp"
#include <boost/bind.hpp>

http_client::http_client(asio::io_service &io_service) 
  : resolver(io_service), 
    socket(io_service)
{

}


http_client::~http_client()
{

}

void http_client::make_request(http_request request)
{
  std::ostream request_stream(&request_buf);
  request_stream << request.get_request();
  
  tcp::resolver::query query(request.get_server(), std::to_string(request.get_port()));

  resolver.async_resolve(query, bind(&http_client::handle_resolve, this,
    asio::placeholders::error, asio::placeholders::iterator, boost::ref(request)));
}

void http_client::handle_resolve(const system::error_code &err, 
  tcp::resolver::iterator endpoint_it, http_request &request)
{
  if(!err)
  {
    asio::async_connect(socket, endpoint_it,
      bind(&http_client::handle_connect, this, asio::placeholders::error, boost::ref(request)));
  } else {
    std::cerr << "Error: " << err.message() << "\n";
  }
}

void http_client::handle_connect(const system::error_code &err, http_request &request)
{
  if(!err)
  {
    asio::async_write(socket, request_buf, bind(&http_client::handle_write_request,
      this, asio::placeholders::error, boost::ref(request)));
  } else {
    std::cerr << "Error: " << err.message() << "\n";
  }
}

void http_client::handle_write_request(const system::error_code &err, http_request &request)
{
  if(!err)
  {
    asio::async_read_until(socket, response_buf, "\r\n",
      bind(&http_client::handle_read_status_line, this, asio::placeholders::error, 
        boost::ref(request)));
  } else {
    std::cerr << "Error: " << err.message() << "\n";
  }
}

void http_client::handle_read_status_line(const system::error_code &err, http_request &request)
{
  if(!err)
  {
    std::istream response_stream(&response_buf);
    std::string http_version;
    std::string status_message;
    size_t status_code;
    
    response_stream >> http_version;
    response_stream >> status_code;
    
    request.set_http_version(http_version);
    request.set_status_code(status_code);
    
    std::getline(response_stream, status_message);
    if(!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      std::cout << "Invalid response: " << response_stream << std::endl;
      return;
    }
    std::cout << "HTTP Version: " << http_version << std::endl;
    std::cout << "Response returned with status code: ";
    std::cout << status_code << std::endl;
    
    asio::async_read_until(socket, response_buf, "\r\n\r\n",
      bind(&http_client::handle_read_headers, this, asio::placeholders::error, 
        boost::ref(request)));
  } else {
    std::cerr << "Error: " << err << std::endl;
  }
}

void http_client::handle_read_headers(const system::error_code &err, http_request &request)
{
  if(!err)
  {
    std::istream response_stream(&response_buf);
    std::string header;
    
    while(std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << std::endl;
    
    if(response_buf.size() > 0)
      std::cout << &response_buf;
      
    asio::async_read(socket, response_buf, asio::transfer_at_least(1),
      bind(&http_client::handle_read_content, this, asio::placeholders::error, 
        boost::ref(request)));
    
  } else {
    std::cerr << "Error: " << err << std::endl;
  }
}

void http_client::handle_read_content(const system::error_code &err, http_request &request)
{
  if(!err)
  {
    std::cout << &response_buf;
    
    asio::async_read(socket, response_buf, asio::transfer_at_least(1),
      bind(&http_client::handle_read_content, this,
        asio::placeholders::error, boost::ref(request)));
  } else if (err != asio::error::eof) {
    std::cout << "Error: " << err << std::endl;
  }
}
