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
 * @file crawler.cxx
 * @author Kyle Givler
 */
 
#include "crawler.hpp"
#include "http_client.hpp"
#include "http_request.hpp"
#include "sqlite.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>

Crawler::Crawler()
{
}
 
Crawler::~Crawler()
{
}
 
void Crawler::start()
{
  //FOR TESTING ONLY!!!
  //NOT READY FOR REAL USE!!!
  
  sqlite db("test.db");
  
  //asio::io_service::work work(io_service);
  auto links = db.get_links(5);
  for(auto &link : links)
  {
    std::unique_ptr<http_request> r(new http_request(
      std::get<0>(link), std::get<1>(link) ) );
      
    r->set_protocol( std::get<2>(link) );
    r->set_request_type(RequestType::CRAWL);
    
    request_queue.push_back(std::move(r));
  }
  
  while(!request_queue.empty())
  {
    http_request *request = request_queue.front().get();
    process_robots(request->get_server(), db);
    http_client c(io_service, *request);
    c.make_request(*request);
    io_service.run();
    db.add_links(request->get_links());
    db.set_visited(request->get_server(), request->get_path(), request->get_protocol());
    request_queue.pop_front();
    io_service.reset();
  }
  
  sleep(3);
  //io_service.stop();
  
  return;
}

void Crawler::process_robots(std::string domain, sqlite &db)
{
  // Follow SOME robots.txt rules...
  // Not fully compliant
  // This is very much a WIP
  // Probably doesn't really work at this point
  
  if(!db.should_process_robots(domain))
    return;
  
  http_request r(domain, "/robots.txt");
  http_client c(io_service, r);
  c.make_request(r);
  io_service.run();
  std::string line;
  bool foundUserAgent = false;
  std::size_t found;
  
  std::istringstream ss(r.get_data());
  io_service.reset();
  while(!ss.eof())
  {
    getline(ss, line);
    boost::to_lower(line);
    if( (found = line.find("#")) != std::string::npos )
      line = line.substr(0, found);
      
    if( (found = line.find("user-agent: ")) != std::string::npos)
    {
      if( (found = line.find("user-agent: *")) != std::string::npos)
      {
        foundUserAgent = true;
      } else {
        foundUserAgent = false;
      }
    }
    
    if( foundUserAgent && (found = line.find("disallow: ")) != std::string::npos)
    {
      std::string disallow = line.substr(10, line.length());
      //std::cout << disallow << std::endl;
      db.blacklist(r.get_server(), disallow, r.get_protocol());
    }
  }
  db.set_robot(domain);
}
