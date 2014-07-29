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
  http_request r("www.google.com");
  http_client c(io_service, r);
  
  http_request r2("www.reddit.com");
  http_client c2(io_service, r2);
  
  http_request r3("www.slashdot.org");
  http_client c3(io_service, r3);
  
  http_request r4("www.weather.com");
  http_client c4(io_service, r4);
  
  io_service.run();
  
  return;
}
