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
#include <iostream>

Crawler::Crawler()
{
}
 
Crawler::~Crawler()
{
}
 
void Crawler::start()
{
  process_robots("www.wikipedia.org");
  //http_request r("http://www.google.com", "/robots.txt");
  //http_client c(io_service, r);
  
  //http_request r2("www.reddit.com");
  //http_client c2(io_service, r2);
  
  //http_request r3("www.slashdot.org");
  //http_client c3(io_service, r3);
  
  //http_request r4("www.facebook.com");
  //http_client c4(io_service, r4);
  
  //io_service.run();
  
  //std::cout << r.get_data() << std::endl;
  
  return;
}

void Crawler::process_robots(std::string domain)
{
  // Follow SOME robots.txt rules...
  // Not fully compliant
  
  http_request r(domain, "/robots.txt");
  http_client c(io_service, r);
  
  std::string line;
  bool foundUserAgent = false;
  std::size_t found;
  
  io_service.run(); // TESTING ONLY
  
  std::istringstream ss(r.get_data());
  while(!ss.eof())
  {
    getline(ss, line);
    if( (found = line.find("#")) != std::string::npos )
      line = line.substr(0, found);
      
    if( (found = line.find("User-agent: ")) != std::string::npos)
    {
      if( (found = line.find("User-agent: *")) != std::string::npos)
      {
        foundUserAgent = true;
      } else {
        foundUserAgent = false;
      }
    }
    
    if( foundUserAgent && (found = line.find("Disallow: ")) != std::string::npos)
    {
      std::string disallow = line.substr(10, line.length());
      std::cout << "Disallow: " << disallow << std::endl;
    }
  }
  
  sleep(3); // TESTING ONLY
}
