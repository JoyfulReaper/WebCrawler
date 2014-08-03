/*
 * WebCrawler: main.cpp
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
 * @file main.cpp
 * @author Kyle Givler
 */

#include "crawler.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "http_client.hpp"
#include "http_request.hpp"


int main(int argc, char **argv)
{
  boost::asio::io_service io;
  Crawler crawler(io);
  
  if(argc == 3)
  {
    crawler.seed(argv[1], argv[2]);
  }
  
  //asio::io_service::work work(io);
  //boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
  io.post(boost::bind(&Crawler::start, &crawler));
  io.run();
  //io.stop();
  //t.join();
  
  return 0;
}
