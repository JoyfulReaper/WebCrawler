/*
 * WebCrawler: crawler.cpp
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
 * @file crawler.cpp
 * @author Kyle Givler
 */
 
#include "crawler.hpp"
#include "http_client.hpp"
#include "http_request.hpp"
#include <iostream>
#include <boost/thread/thread.hpp>

 
crawler::crawler()
{
}
 
crawler::~crawler()
{
}
 
 
void crawler::start()
{
  thread_group threads;
  http_client client(io_service);
  
  http_request r("google.com");
  client.make_request(r);
  
  for(std::size_t i = 0; i < num_threads; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  io_service.run();
  threads.join_all();
  
  auto ers = r.get_errors();
  for(auto &er : ers)
    std::cout << er << std::endl;
  
  auto links = r.get_links();
  for(auto &link : links)
    std::cout << link << std::endl;
  
  return;
}
