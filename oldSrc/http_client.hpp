/*
 * WebCrawler: http_client.hpp
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
 * 
 * Inspired by the Boost ASIO example:
 * async_client.cpp
 * Under the Boost License
 * A slightly modified copy can be found int the test/asio/async_http.cpp file
 * 
 */

/**
 * @file http_client.hpp
 * @author Kyle Givler
 */

#ifndef _WC_HTTP_CLIENT_H_
#define _WC_HTTP_CLIENT_H_

#include <boost/asio.hpp>
#include <map>
#include <memory>
#include "logger/logger.hpp"

class http_request;

using boost::asio::ip::tcp;
using namespace boost;

typedef std::shared_ptr<http_request> s_request;

class http_client
{
public:
  http_client(asio::io_service &io_service);
  
  virtual ~http_client();
  
  void make_request(s_request request);

private:
  asio::io_service &io_service;
  std::map<std::string, tcp::socket> sockets;
  asio::strand strand;
  tcp::resolver resolver;
  Logger logger;

  void handle_resolve(
    const system::error_code &err, 
    tcp::resolver::iterator endpoint_it, 
    s_request request);

  void handle_connect(
    const system::error_code &err, 
    s_request request);

  void handle_write_request(
    const system::error_code &err, 
    s_request request);

  void handle_read_status_line(
    const system::error_code &err, 
    s_request request);

  void handle_read_headers(
    const system::error_code &err, 
    s_request request);

  void handle_read_content(
    const system::error_code &err, 
    s_request request);
};

#endif
