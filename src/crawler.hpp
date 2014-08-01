/*
 * WebCrawler: crawler.hpp
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
 * @file crawler.hpp
 * @author Kyle Givler
 */

#ifndef _WC_CRAWLER_H_
#define _WC_CRAWLER_H_

#include "logger/logger.hpp"
#include "sqlite.hpp"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <deque>
#include <unordered_map>

using namespace boost;
class http_request;

class Crawler
{
public:
  Crawler();

  virtual ~Crawler();

  void start();
  
  bool check_if_header_text_html(http_request &request);
  
  bool check_if_html(http_request &request);
  
  void process_robots(std::string domain, std::string protocol, sqlite &db);
  
  void seed(std::string domain, std::string path);
  
  void handle_stop();

private:
  Logger logger;
  asio::io_service io_service;
  std::deque<std::unique_ptr<http_request>> request_queue;
  asio::signal_set signals;
  sqlite db;
};

#endif
