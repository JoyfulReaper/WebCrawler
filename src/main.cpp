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
#include "http_client.hpp"
#include "http_request.hpp"

int main(int argc, char **argv)
{
  
  if(argc == 3)
  {
    Crawler crawler;
    crawler.seed(argv[1], argv[2]);
    crawler.start();
  } else {
    Crawler crawler;
    crawler.start();
  }
  return 0;
}
