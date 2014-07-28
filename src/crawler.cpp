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
#include "sqlite_database.hpp"
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
  sqlite_database db("test.db");
  thread_group threads;
  asio::io_service::work work(io_service);
  http_client client(io_service);
  
  for(std::size_t i = 0; i < num_threads; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  http_request r("www.reddit.com");
  io_service.post(boost::bind(&http_client::make_request, &client, boost::ref(r)));
  
  while(!r.get_completed())
  {
    sleep(1);
  }
  
  io_service.stop();
  threads.join_all();
  
  db.add_links(r);
  
  //auto links = r.get_links();
  //for(auto &link : links)
  //  std::cout << link << std::endl;
  
  return;
}
