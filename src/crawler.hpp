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
#include <memory>
#include <deque>
#include "http_request.hpp"

using namespace boost;

class crawler
{
public:
  crawler();

  virtual ~crawler();

  void start();

  http_request get_next_request();

  void set_database_name(std::string database) { this->databaseFile = database; }

  std::string get_database_name() { return this->databaseFile; }

private:
  asio::io_service io_service;
  std::size_t num_threads = 2;
  std::string databaseFile = "UNSET";
  std::deque<std::shared_ptr<http_request>> request_queue;
};

#endif
