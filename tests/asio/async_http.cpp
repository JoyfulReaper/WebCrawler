//
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Modified by Kyle Givler
//

#include <iostream>
#include <string>
#include <ostream>
#include <istream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using namespace boost;

class client
{
public:
  client(asio::io_service &io_service,
    const std::string &server, const std::string &path) :
    resolver(io_service),
    socket(io_service)
  {
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
    
    tcp::resolver::query query(server, "http");
    resolver.async_resolve(query, bind(&client::handle_resolve, this,
      asio::placeholders::error, asio::placeholders::iterator));
  }
  
private:
  tcp::resolver resolver;
  tcp::socket socket;
  asio::streambuf request;
  asio::streambuf response;
  
  void handle_resolve(const system::error_code &err, 
    tcp::resolver::iterator endpoint_iterator)
  {
    if(!err)
    {
      asio::async_connect(socket, endpoint_iterator,
        bind(&client::handle_connect, this, asio::placeholders::error));
    } else {
      std::cerr << "Error: " << err.message() << "\n";
    }
  }
  
  void handle_connect(const system::error_code &err)
  {
    if(!err)
    {
      asio::async_write(socket, request, bind(&client::handle_write_request,
        this, asio::placeholders::error));
    } else {
      std::cerr << "Error: " << err.message() << "\n";
    }
  }
  
  void handle_write_request(const system::error_code &err)
  {
    if(!err)
    {
      asio::async_read_until(socket, response, "\r\n",
        bind(&client::handle_read_status_line, this, asio::placeholders::error));
    } else {
      std::cerr << "Error: " << err.message() << "\n";
    }
  }
  
  void handle_read_status_line(const system::error_code &err)
  {
    if(!err)
    {
      std::istream response_stream(&response);
      std::string http_version;
      std::string status_message;
      size_t status_code;
      
      response_stream >> http_version;
      response_stream >> status_code;
      std::getline(response_stream, status_message);
      if(!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response: " << response_stream << std::endl;
        return;
      }
      std::cout << "HTTP Version: " << http_version << std::endl;
      std::cout << "Response returned with status code: ";
      std::cout << status_code << std::endl;
      
      asio::async_read_until(socket, response, "\r\n\r\n",
        bind(&client::handle_read_headers, this, asio::placeholders::error));
    } else {
      std::cerr << "Error: " << err << std::endl;
    }
  }
  
  void handle_read_headers(const system::error_code &err)
  {
    if(!err)
    {
      std::istream response_stream(&response);
      std::string header;
      
      while(std::getline(response_stream, header) && header != "\r")
        std::cout << header << "\n";
      std::cout << std::endl;
      
      if(response.size() > 0)
        std::cout << &response;
        
      asio::async_read(socket, response, asio::transfer_at_least(1),
        bind(&client::handle_read_content, this, asio::placeholders::error));
      
    } else {
      std::cerr << "Error: " << err << std::endl;
    }
  }
  
    void handle_read_content(const system::error_code &err)
  {
    if(!err)
    {
      std::cout << &response;
      
      asio::async_read(socket, response, asio::transfer_at_least(1),
        bind(&client::handle_read_content, this,
          asio::placeholders::error));
    } else if (err != asio::error::eof) {
      std::cout << "Error: " << err << std::endl;
    }
  }
};

int main(int argc, char **argv)
{
  try
  {
    if (argc != 3)
    {
      std::cout << "Usage: async_client <server> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_service io_service;
    client c(io_service, argv[1], argv[2]);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
