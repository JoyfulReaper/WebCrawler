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
#include <boost/algorithm/string/find.hpp>
#include <sstream>

static const bool DEBUG = true;

http_client::http_client(asio::io_service &io_service) 
  : io_service(io_service),
    strand(io_service),
    resolver(io_service),
    logger("http_client")
{
  logger.setIgnoreLevel(Level::TRACE);
}

http_client::~http_client()
{

}

void http_client::make_request(
  s_request request)
{
  std::string domain = request->get_server();
  std::size_t found = domain.find("://");
  if(found != std::string::npos)
  {
    std::string proto = domain.substr(0, found + 2);
    if(proto == "https://")
    {
      request->add_error("No https support :(");
      return;
    } else {
      domain = domain.substr(found + 3);
      request->set_server(domain);
      if(DEBUG)
      {
        logger.debug("Converted to: " + domain);
      }
    }
  }
  
  request->reset_errors();
  
  sockets.insert(std::pair<std::string, tcp::socket>(request->get_server(), 
    tcp::socket(io_service)));
    
  std::ostream request_stream(&request->get_request_buf());
  
  if(request->get_request().size() > 0) // Request provided
    request_stream << request->get_request();
  else if (request->get_request_type() == RequestType::GET) // Get request
  {
    request_stream << "GET " << request->get_path() << " HTTP/1.0\r\n";
    request_stream << "User-Agent: https://github.com/JoyfulReaper/WebCrawler\r\n";
    request_stream << "Host: " << request->get_server() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
  } else if (request->get_request_type() == RequestType::HEAD)  // Head request
  {
    request_stream << "HEAD " << request->get_path() << " HTTP/1.0\r\n";
    request_stream << "User-Agent: https://github.com/JoyfulReaper/WebCrawler\r\n";
    request_stream << "Host: " << request->get_server() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
  }
  
  tcp::resolver::query query(request->get_server(), std::to_string(request->get_port()));

  resolver.async_resolve(query, bind(&http_client::handle_resolve, this,
    asio::placeholders::error, asio::placeholders::iterator, request));
}

void http_client::handle_resolve(
  const system::error_code &err, 
  tcp::resolver::iterator endpoint_it, 
  s_request request)
{
  if(!err)
  {
    asio::async_connect(sockets.at(request->get_server()), endpoint_it,
      strand.wrap( strand.wrap(bind( &http_client::handle_connect, this, asio::placeholders::error, 
        request) ) ) );
  } else {
    request->add_error("Error: " + err.message());
  }
}

void http_client::handle_connect(
  const system::error_code &err, 
  s_request request)
{
  if(!err)
  {
    asio::async_write(sockets.at(request->get_server()), request->get_request_buf(), 
      strand.wrap(bind(&http_client::handle_write_request, this, 
        asio::placeholders::error, request)));
  } else {
    request->add_error ("Error: " + err.message());
  }
}

void http_client::handle_write_request(
  const system::error_code &err, 
  s_request request)
{
  if(!err)
  {
    asio::async_read_until(sockets.at(request->get_server()), request->get_response_buf(), "\r\n",
      strand.wrap(bind(&http_client::handle_read_status_line, this, asio::placeholders::error,
      request)));
  } else {
    request->add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_status_line(
  const system::error_code &err, 
  s_request request)
{
  if(!err)
  {
    std::istream response_stream(&request->get_response_buf());
    std::string http_version;
    std::string status_message;
    size_t status_code;
    
    response_stream >> http_version;
    response_stream >> status_code;
    request->set_http_version(http_version);
    request->set_status_code(status_code);
    
    std::getline(response_stream, status_message);
    if(!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      request->add_error( "Invalid response HTTP response");
      return;
    }
    
    if(DEBUG)
    {
      logger.debug("HTTP Version: " + http_version);
      logger.debug("Status code: " + std::to_string(status_code));
      logger.debug("Status message: " + status_message);
    }
    
    asio::async_read_until(sockets.at(request->get_server()), request->get_response_buf(), "\r\n\r\n",
      strand.wrap(bind(&http_client::handle_read_headers, this, asio::placeholders::error, 
        request)));
  } else {
    request->add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_headers(
  const system::error_code &err, 
  s_request request)
{
  if(!err)
  {
    std::istream response_stream(&request->get_response_buf());
    std::string header;
    
    while(std::getline(response_stream, header) && header != "\r")
    {
      if(DEBUG)
        logger.debug(header);
        
      request->add_header(header + "\n");
    }
    
    if(request->get_response_buf().size() > 0)
    {
      std::cout << header << "\n";
      request->add_header(header);
    }
     
    // I Hate string parsing :(
    if(request->get_status_code() == 302 || request->get_status_code() == 301)
    {
      auto headers = request->get_headers();
      for(auto &header : headers)
      {
        std::size_t found = header.find("Location: ");
        if(found != std::string::npos)
        {
          std::string location = header.substr(10, header.length());
          found = location.find("://");
          if(found)
          {
            location = location.substr(found + 3, location.length());
            found = location.find("/");
            if(found != std::string::npos)
            {
              std::string server = location.substr(0, found);
              std::string resource = location.substr(found, location.length());
              
              found = resource.find("\r");
              if(found != std::string::npos)
              {
                resource = resource.substr(0, found);
                //std::cout << "S: " << server << " L: " << resource << std::endl;
                request->set_server(server);
                request->set_path(resource);
                request->reset_buffers();
                make_request(request);
              } // Remove "\r"
            } // Found ://
          }
        } // found Location:
      } // loop through headers 
    } // Status is 302/301
      
    asio::async_read(sockets.at(request->get_server()), request->get_response_buf(), asio::transfer_at_least(1),
      strand.wrap(bind(&http_client::handle_read_content, this, asio::placeholders::error, 
        request)));
    
  } else {
    request->add_error ("Error: " + err.message());
  }
}

void http_client::handle_read_content(
  const system::error_code &err, 
  s_request request)
{
  if(!err)
  {
    std::ostringstream ss;
    ss << &request->get_response_buf();
    request->get_data().append(ss.str());

    
    asio::async_read(sockets.at(request->get_server()), request->get_response_buf(), asio::transfer_at_least(1),
      strand.wrap(bind(&http_client::handle_read_content, this,
        asio::placeholders::error, request)));
  } else if (err == asio::error::eof) {
    request->set_completed(true);     
  } else if (err != asio::error::eof) {
    request->add_error ("Error: " + err.message());
  }
}
