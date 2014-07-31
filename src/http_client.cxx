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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

http_client::http_client(asio::io_service &io_service, http_request &request) 
  : io_service(io_service),
    socket(io_service),
    strand(io_service),
    resolver(io_service),
    logger("http_client"),
    sslctx(asio::ssl::context::sslv23),
    ssl_sock(socket, sslctx),
    deadline(io_service)
{
  logger.setIgnoreLevel(Level::NONE);
  make_request(request);
}

http_client::~http_client()
{

}

void http_client::stop(http_request &request, std::string where)
{
  stopped = true;
  logger.trace("stop: " + request.get_server() + " From: " + where);
  if(deadline.expires_at() <= asio::deadline_timer::traits_type::now())
  {
    if(request.get_protocol() == "https")
      ssl_sock.lowest_layer().close();
    else
      socket.close();
    
    logger.warn("Timeout: " + request.get_server());
    deadline.expires_at(boost::posix_time::pos_infin);
    return;
  }
  deadline.cancel();
}

void http_client::make_request(
  http_request &request)
{
  logger.trace("make_request: " + request.get_server());
  if(stopped)
    return;
    
  deadline.expires_from_now(posix_time::seconds(120));
  std::string stopstring = "timer";
  deadline.async_wait(boost::bind(&http_client::stop, this, ref(request), stopstring ) );
  
  logger.trace( "Requesting: " + request.get_protocol() + "://" + request.get_server() + request.get_path());
  std::string domain = request.get_server();
  
  if(domain[0] == '/' && domain[1] == '/')
  {
    logger.trace("Fixing link: " + domain);
    domain.erase(0,2);
    domain.insert(0, "http://");
  }
  
  std::size_t found = domain.find("://");
  if(found != std::string::npos)
  {
    std::string proto = domain.substr(0, found + 3);
    if(proto == "https://")
    {
      logger.trace("Set protocol to https");
      request.set_protocol("https");
    }
    domain = domain.substr(found + 3);
    request.set_server(domain);
    logger.trace("Converted to: " + domain);
  }
  
  std::ostream request_stream(&request.get_request_buf());
  
  if(request.get_request().size() > 0) // Request provided
    request_stream << request.get_request();
  else if (request.get_request_type() == RequestType::GET) // Get request
  {
    request_stream << "GET " << request.get_path() << " HTTP/1.0\r\n";
    request_stream << "User-Agent: SomeSortaBotIguess\r\n";
    request_stream << "Host: " << request.get_server() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
  } else if (request.get_request_type() == RequestType::HEAD || 
              request.get_request_type() == RequestType::CRAWL)
  {
    request_stream << "HEAD " << request.get_path() << " HTTP/1.0\r\n";
    request_stream << "User-Agent: SomeSortaBotIguess\r\n";
    request_stream << "Host: " << request.get_server() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
  }
  
  tcp::resolver::query query(request.get_server(), std::to_string(request.get_port()));

  resolver.async_resolve( query, strand.wrap(bind ( &http_client::handle_resolve, this,
    asio::placeholders::error, asio::placeholders::iterator, ref(request) ) ) );
}

void http_client::handle_resolve(
  const system::error_code &err, 
  tcp::resolver::iterator endpoint_it, 
  http_request &request)
{
  logger.trace("handle_resolve: " + request.get_server());
  if(stopped)
    return;
    
  if(!err)
  {
    if(request.get_protocol() == "https")
    {
      logger.trace("https resolve");
      sslctx.set_verify_mode(asio::ssl::verify_none);
      
      asio::async_connect( ssl_sock.lowest_layer(), endpoint_it,
      strand.wrap( bind( &http_client::handle_connect, this, 
        asio::placeholders::error, ref(request) ) ) );
    } else {
      asio::async_connect( socket, endpoint_it,
        strand.wrap( bind( &http_client::handle_connect, this, 
          asio::placeholders::error, ref(request) ) ) );
    }
  } else {
    logger.warn(err.message());
    request.add_error("Error: " + err.message());
    stop(request, "resolve");
  }
}

void http_client::handle_connect(
  const system::error_code &err, 
  http_request &request)
{
  logger.trace("handle_connect: " + request.get_server());
  if(stopped)
    return;
    
  if(!err)
  {
    if(request.get_protocol() == "https")
    {
      logger.trace("https connect");
      ssl_sock.async_handshake(asio::ssl::stream_base::client,
      strand.wrap( bind( &http_client::handle_handshake, this, 
        asio::placeholders::error, ref(request) ) ) );
    } else {
      asio::async_write( socket, request.get_request_buf(), 
        strand.wrap( bind ( &http_client::handle_write_request, this, 
          asio::placeholders::error, ref(request) ) ) );
    }
  } else {
    logger.warn(err.message());
    request.add_error ("Error: " + err.message());
    stop(request, "connect");
  }
}

void http_client::handle_handshake(
    const system::error_code &err, 
    http_request &request)
{
  if(stopped)
    return;
    
  if(!err)
  {
    logger.trace("https handshake: " + request.get_server());
    asio::async_write( ssl_sock, request.get_request_buf(), 
      strand.wrap( bind ( &http_client::handle_write_request, this, 
        asio::placeholders::error, ref(request) ) ) );
  } else {
    logger.warn(err.message());
    request.add_error ("Error: " + err.message());
    stop(request, "handshake");
  }
}

void http_client::handle_write_request(
  const system::error_code &err, 
  http_request &request)
{
  logger.trace("handle_write_request: " + request.get_server());
  if(stopped)
    return;
    
  if(!err)
  {
    if(request.get_protocol() == "https")
    {
      logger.trace("https write_request: " + request.get_server());
      asio::async_read_until( ssl_sock, request.get_response_buf(), "\r\n",
        strand.wrap ( bind ( &http_client::handle_read_status_line, this, asio::placeholders::error,
        ref(request) ) ) );
    } else {
      asio::async_read_until( socket, request.get_response_buf(), "\r\n",
        strand.wrap ( bind ( &http_client::handle_read_status_line, this, asio::placeholders::error,
        ref(request) ) ) );
    }
  } else {
    logger.warn(err.message());
    request.add_error ("Error: " + err.message());
    stop(request, "write_request");
  }
}

void http_client::handle_read_status_line(
  const system::error_code &err, 
  http_request &request)
{
  logger.trace("handle_read_status_line: " + request.get_server());
  if(stopped)
    return;  

  if(!err)
  {
    std::istream response_stream(&request.get_response_buf());
    std::string http_version;
    std::string status_message;
    size_t status_code;
    
    if(request.get_protocol() == "https")
    {
      logger.trace("https read_status");
      response_stream >> http_version;
      response_stream >> status_code;
      request.set_http_version(http_version);
      request.set_status_code(status_code);
      
      std::getline(response_stream, status_message);
      if(!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        logger.warn("SSL Invalid HTTP response");
        request.add_error( "Invalid HTTP response");
        return;
      }
      
      logger.trace("HTTP Version: " + http_version);
      logger.trace("Status code: " + std::to_string(status_code));
      logger.trace("Status message: " + status_message);
      
      asio::async_read_until( ssl_sock, request.get_response_buf(), "\r\n\r\n",
        strand.wrap ( bind ( &http_client::handle_read_headers, this, asio::placeholders::error,
        ref(request) ) ) );
    } else {      
      response_stream >> http_version;
      response_stream >> status_code;
      request.set_http_version(http_version);
      request.set_status_code(status_code);
      
      std::getline(response_stream, status_message);
      if(!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        logger.warn("Invalid HTTP response");
        request.add_error( "Invalid HTTP response");
        return;
      }
      
      logger.trace("HTTP Version: " + http_version);
      logger.trace("Status code: " + std::to_string(status_code));
      logger.trace("Status message: " + status_message);
      
      asio::async_read_until( socket, request.get_response_buf(), "\r\n\r\n",
        strand.wrap( bind( &http_client::handle_read_headers, this, asio::placeholders::error, 
          ref(request) ) ) );
    }
  } else {
    logger.warn(err.message());
    request.add_error ("Error: " + err.message());
    stop(request, "status_line");
  }
}

void http_client::handle_read_headers(
  const system::error_code &err, 
  http_request &request)
{
  logger.trace("handle_read_headers: " + request.get_server());
  if(stopped)
    return;
    
  if(!err)
  {
    std::istream response_stream(&request.get_response_buf());
    std::string header;
    
    while(std::getline(response_stream, header) && header != "\r")
    {
      logger.trace(header);
      std::size_t found = std::string::npos;
      request.add_header(header + "\n");
      found = header.find("Content-Type: text/html");
      if(found != std::string::npos && request.get_request_type() == RequestType::CRAWL)
      {
        request.set_request_type(RequestType::GET);
        request.reset_buffers();
        request.reset_errors();
        make_request(request);
      }
    }
    
    if(request.get_response_buf().size() > 0)
    {
      logger.trace(header);
      request.add_header(header);
    }
     
    if(request.get_status_code() == 403 || 
       request.get_status_code() == 404 ||
       request.get_status_code() == 503)
    {
      logger.warn(request.get_status_code() + ": " + request.get_server() + request.get_path());
      stop(request, "Bad code");
    }
    
    // If response code is 302 or 301 try request again
    if(request.get_status_code() == 302 || request.get_status_code() == 301)
    {
      auto headers = request.get_headers();
      for(auto &header : headers)
      {
        std::size_t found = header.find("Location: ");
        if(found != std::string::npos)
        {
          std::string location = header.substr(10, header.length());
          found = location.find("https://");
          if(found != std::string::npos)
            request.set_protocol("https");
            
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
                request.set_server(server);
                request.set_path(resource);
                request.reset_buffers();
                request.reset_errors();
                deadline.cancel();
                redirect_count++;
                logger.debug("301/302: (" + std::to_string(redirect_count) + ") Re-requesting: " + server + resource);
                if(redirect_count > 15)
                {
                  request.should_blacklist(true, "redirect loop");
                  stop(request, "Redirect");
                }
                make_request(request);
              }
            }
          }
        }
      }
      return;
    }
    if(request.get_protocol() == "https")
    {
      logger.trace("https read_header");
      asio::async_read( ssl_sock, request.get_response_buf(), 
        asio::transfer_at_least(1), strand.wrap( bind( &http_client::handle_read_content, 
          this, asio::placeholders::error, ref(request) ) ) );
    } else {
      asio::async_read( socket, request.get_response_buf(), 
        asio::transfer_at_least(1), strand.wrap( bind( &http_client::handle_read_content, 
          this, asio::placeholders::error, ref(request) ) ) );
    }
  } else {
    logger.warn("Error: " + err.message());
    request.add_error ("Error: " + err.message());
    stop(request, "read_headers");
  }
}

void http_client::handle_read_content(
  const system::error_code &err, 
  http_request &request)
{
  logger.trace("handle_read_content: " + request.get_server());
  if(stopped)
    return;
    
  if(!err)
  {    
    std::ostringstream ss;
    ss << &request.get_response_buf();
    request.get_data().append(ss.str());

    if(request.get_protocol() == "https")
    {
      //logger.trace("https read_content");
      asio::async_read(ssl_sock, request.get_response_buf(), asio::transfer_at_least(1),
        strand.wrap( bind(&http_client::handle_read_content, this,
          asio::placeholders::error, ref(request) ) ) );
    } else {
      asio::async_read(socket, request.get_response_buf(), asio::transfer_at_least(1),
        strand.wrap( bind(&http_client::handle_read_content, this,
          asio::placeholders::error, ref(request) ) ) );
    }
  } else if (err == asio::error::eof) {
    request.set_completed(true);
    logger.trace("Request completed: " + request.get_server());
  } else if (err != asio::error::eof) {
    request.set_completed(true);
    logger.debug("Content Error: " + err.message());
    request.add_error ("Error: " + err.message());
  }
}
