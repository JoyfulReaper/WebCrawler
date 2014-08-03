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

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <deque>

#include "logger/logger.hpp"
#include "sqlite.hpp"
#include "http_client.hpp"
#include "request_reciver.hpp"

using namespace boost;

class Crawler : public request_reciver
{
public:
  Crawler(boost::asio::io_service &io_service);

  virtual ~Crawler();

  void receive_http_request(http_request *request);

  /**
   * Start the crawl
   */
  void start();
  
  /**
   * Check if the given resources retuns the Content: text/html header
   * @param A vector of headers to check
   * @return true if html/text or false if the header wasn't sent
   */
  bool check_if_header_text_html(
    std::vector<std::string> headers);
  
  /**
   * Make the request, download contents, check if it has an <html> tag
   * @param data The resource to check
   * @return true if html, false otherwise
   */
  bool check_if_html(std::string data);
  
  
  /**
   * Add the given URL to the database to be processed
   * @param domain The domain
   * @param path The resource (/, /index.html, etc)
   */
  void seed(std::string domain, std::string path);
  

private:
  http_client client;
  std::deque<std::tuple<std::string,std::string,std::string>> request_queue;
  asio::signal_set signals;
  asio::strand strand;
  asio::io_service &io_service;
  sqlite db;
  Logger logger;
  
  void do_request(http_request *request);
  
  void prepare_next_request(RequestType type);
  
  /**
   * Close the database and exit
   */
  void handle_stop();
  
  /**
   * Process a sites robots.txt
   * @param server The domain
   * @param path
   * @param protocol http or https
   * @param data
   */
  void process_robots(
    std::string server,
    std::string protocol,
    std::string data);
};

#endif
