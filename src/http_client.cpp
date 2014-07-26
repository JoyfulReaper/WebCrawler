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
#include <sstream>

http_client::http_client(asio::io_service &io_service) 
  : io_service(io_service),
    strand(io_service),
    resolver(io_service)
{

}


http_client::~http_client()
{

}

void http_client::make_request(
  http_request &request)
{
  sockets.insert(std::pair<std::string, tcp::socket>(request.get_server(), 
    tcp::socket(io_service)));
    
  std::ostream request_stream(&request.get_request_buf());
  
  if(request.get_request().size() > 0)
    request_stream << request.get_request();
  else
  {
    request_stream << "GET " << request.get_path() << "HTTP/1.0\r\n";
    request_stream << "User-Agent: https://github.com/JoyfulReaper/WebCrawler";
    request_stream << "Host: " << request.get_server() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
  }
  
  tcp::resolver::query query(request.get_server(), std::to_string(request.get_port()));

  resolver.async_resolve(query, bind(&http_client::handle_resolve, this,
    asio::placeholders::error, asio::placeholders::iterator, boost::ref(request)));
}

void http_client::handle_resolve(
  const system::error_code &err, 
  tcp::resolver::iterator endpoint_it, 
  http_request &request)
{
  if(!err)
  {
    asio::async_connect(sockets.at(request.get_server()), endpoint_it,
      strand.wrap( bind( &http_client::handle_connect, this, asio::placeholders::error, 
        boost::ref(request) ) ) );
  } else {
    request.add_error("Error: " + err.message());
  }
}

void http_client::handle_connect(
  const system::error_code &err, 
  http_request &request)
{
  if(!err)
  {
    asio::async_write(sockets.at(request.get_server()), request.get_request_buf(), 
      strand.wrap(bind(&http_client::handle_write_request, this, 
        asio::placeholders::error, boost::ref(request))));
  } else {
    request.add_error ("Error: " + err.message());
  }
}

void http_client::handle_write_request(
  const system::error_code &err, 
  http_request &request)
{
  if(!err)
  {
    asio::async_read_until(sockets.at(request.get_server()), request.get_response_buf(), "\r\n",
      strand.wrap(bind(&http_client::handle_read_status_line, this, asio::placeholders::error, 
        boost::ref(request))));
  } else {
    request.add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_status_line(
  const system::error_code &err, 
  http_request &request)
{
  if(!err)
  {
    std::istream response_stream(&request.get_response_buf());
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
      request.add_error( "Invalid response HTTP response");
      return;
    }
    
    if(DEBUG)
    {
      std::cout << "DEBUG: HTTP Version: " << http_version << std::endl;
      std::cout << "DEBUG: Status code: " << status_code << std::endl;
      std::cout << "DEBUG: Status message: " << status_message << "\n";
    }
    
    asio::async_read_until(sockets.at(request.get_server()), request.get_response_buf(), "\r\n\r\n",
      strand.wrap(bind(&http_client::handle_read_headers, this, asio::placeholders::error, 
        boost::ref(request))));
  } else {
    request.add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_headers(
  const system::error_code &err, 
  http_request &request)
{
  if(!err)
  {
    std::istream response_stream(&request.get_response_buf());
    std::string header;
    
    while(std::getline(response_stream, header) && header != "\r")
    {
      if(DEBUG)
        std::cout << "DEBUG: " << header << "\n";
        
      request.add_header(header + "\n");
    }
    
    if(request.get_response_buf().size() > 0)
    {
      std::cout << header << "\n";
      request.add_header(header);
    }
      
    asio::async_read(sockets.at(request.get_server()), request.get_response_buf(), asio::transfer_at_least(1),
      strand.wrap(bind(&http_client::handle_read_content, this, asio::placeholders::error, 
        boost::ref(request))));
    
  } else {
    request.add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_content(
  const system::error_code &err, 
  http_request &request)
{
  if(!err)
  {
    std::istream data_stream(&request.get_response_buf());
    std::stringbuf data_string;
    data_stream.get(data_string);
    request.get_data().append(data_string.str());
    
    asio::async_read(sockets.at(request.get_server()), request.get_response_buf(), asio::transfer_at_least(1),
      strand.wrap(bind(&http_client::handle_read_content, this,
        asio::placeholders::error, boost::ref(request))));
  } else if (err != asio::error::eof) {
    request.add_error ("Error: " + err.message());
  }
}
