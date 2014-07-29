/*
 * WebCrawler: crawler.cxx
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

Crawler::Crawler()
{
}
 
Crawler::~Crawler()
{
}
 
void Crawler::start()
{
  thread_group threads;
  asio::io_service::work work(io_service);
  http_client client(io_service);
  sqlite_database db("test.db");
  asio::strand strand(io_service);
  
  for(std::size_t i = 0; i < num_threads; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
    
  //std::shared_ptr<http_request> test(new http_request("www.google.com", "/services/"));
  //test->set_request_type(RequestType::CRAWL);
  //request_queue.push_back(std::move(test));
  
  auto fill = db.fill_queue();
  for(auto &req : fill)
  {
    req->set_request_type(RequestType::CRAWL);
    request_queue.push_back(req);
  }
  
  std::shared_ptr<http_request> request;
  while(!request_queue.empty())
  {
    request = request_queue.front();
    if(!db.get_visited(request))
    {
      int count = 0;
      io_service.post(boost::bind(&http_client::make_request, &client, request));
      while(!request->get_completed())
      {
        count++;
        sleep(1);
        if(count == 30);
        {
          std::cout << "Timeout: " << request->get_server() << request->get_path();
          sleep(2);
          break;
        }
      }
      io_service.post(strand.wrap(boost::bind(&sqlite_database::set_visited, &db, request)));
      io_service.post(strand.wrap(boost::bind(&sqlite_database::add_links, &db, request)));
    }
    request_queue.pop_front();
  }
  
  std::cout << "Done?";
  sleep(3);
  io_service.stop();
  threads.join_all();
  
  return;
}
